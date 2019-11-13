#include "plugin.hpp"
#include "cmath"
#include <dirent.h>
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include "osdialog.h"

/**
 * DSP processor
 * rack::engine::Module
 * @see https://vcvrack.com/docs/structrack_1_1engine_1_1Module.html
 */
struct WavPlay : Module {
	enum ParamIds {
		PITCH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		TRIGGER_INPUT,
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ISPLAYING_LIGHT,
		NUM_LIGHTS
	};

	bool isLoading = false;
	bool isFileLoaded = true;
	bool isPlaying = false;
	bool isReloading = false;
	int sampnumber = 0;
	float samplePos = 0;
	unsigned int channels;
	unsigned int sampleRate;
	drwav_uint64 totalSampleCount;
	std::string lastPath = "";
	std::string fileDesc = "";
	std::vector<std::vector<float>> playBuffer;
	std::vector<std::string> fileNames;

	// SchmittTrigger: Turns HIGH when value reaches 1.f, turns LOW when value reaches 0.f.
	dsp::SchmittTrigger loadsampleTrigger;
	dsp::SchmittTrigger playTrigger;
	dsp::SchmittTrigger nextTrigger;
	dsp::SchmittTrigger prevTrigger;

	// Constructs a Module with no params, inputs, outputs, and lights.
	WavPlay() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PITCH_PARAM, 0.f, 1.f, 0.f, "");
		playBuffer.resize(1);
		playBuffer[0].resize(0);
	}

	// advances the module by one audio sample
	void process(const ProcessArgs& args) override;

	// load a wav file
	void loadWavFile(std::string path);

	// persist module data
	json_t *dataToJson() override;
	void dataFromJson(json_t* root) override;
};

/**
 * Store extra internal data in the 'data' property of the module's JSON object.
 * @returns 
 */
json_t *WavPlay::dataToJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "lastPath", json_string(lastPath.c_str()));	
	return rootJ;
}

/**
 * Store extra internal data from the 'data' property of the module's JSON object.
 * @param rootJ
 */
void WavPlay::dataFromJson(json_t* rootJ) {
	json_t *lastPathJ = json_object_get(rootJ, "lastPath");
	if (lastPathJ) {
		lastPath = json_string_value(lastPathJ);
		isReloading = true ;
		loadWavFile(lastPath);
	}
}

/**
 * Advances the module by one audio sample.
 * @param args ProcessArgs struct
 * @param args.sampleRate Sample rate, for example 44100.
 * @param args.sampleTime Time since app started??? measured in seconds???
 */
void WavPlay::process(const ProcessArgs& args) {
	if (inputs[TRIGGER_INPUT].active) {

		// if the input value triggers the schmittrigger to flip HIGH
		if (playTrigger.process(inputs[TRIGGER_INPUT].value)) {
			isPlaying = true;
			samplePos = 0;
		}
	}

	// play and advance sample
	if ((!isLoading) && (isPlaying) && ((std::abs(floor(samplePos)) < totalSampleCount))) {
		if (samplePos >= 0) {
			outputs[AUDIO_OUTPUT].value = 5 * playBuffer[0][floor(samplePos)];
		} else {
			outputs[AUDIO_OUTPUT].value = 5 * playBuffer[0][floor(totalSampleCount - 1 + samplePos)];
		}
		samplePos = samplePos + 1;
	} else {
		isPlaying = false;
		outputs[AUDIO_OUTPUT].value = 0;
	}

	// light on while sample plays
	lights[ISPLAYING_LIGHT].setBrightness(isPlaying ? 1.f : 0.f);
}

/**
 * Load a Wav audio file.
 * @param path File path.
 */
void WavPlay::loadWavFile(std::string path) {
	isLoading = true;
	unsigned int _channels;
	unsigned int _sampleRate;
	drwav_uint64 _totalSampleCount;
	float* pSampleData;

	pSampleData = drwav_open_and_read_file_f32(path.c_str(), &_channels, &_sampleRate, &_totalSampleCount);

	if (pSampleData != NULL) {
		channels = _channels;
		sampleRate = _sampleRate;

		// clear playBuffer and fill with the loaded samples
		playBuffer[0].clear();
		for (unsigned int i = 0; i < _totalSampleCount; i++) {
			playBuffer[0].push_back(pSampleData[i]);
		}
		totalSampleCount = playBuffer[0].size();
		drwav_free(pSampleData);

		isLoading = false;
		isFileLoaded = true;
		fileDesc = rack::string::filename(path);

		if (isReloading) {
			DIR* directory = NULL;
			struct dirent* directoryEntry = NULL;
			std::string directoryName = path.empty() ? asset::user("") : rack::string::directory(path);
			directory = opendir(directoryName.c_str());
			int i = 0;
			fileNames.clear();

			// store all the directory's wav file names in vector wavFiles
			while ((directoryEntry = readdir(directory)) != NULL) {
				std::string fileName = directoryEntry->d_name;
				std::size_t found = fileName.find(".wav", fileName.length() - 5);
				if (found == std::string::npos) {
					found = fileName.find(".WAV", fileName.length() - 5);

  				if (found != std::string::npos) {
						fileNames.push_back(fileName);
						if ((directoryName + "/" + fileName) == path) {
							sampnumber = i;
						}
						i = i + 1;
					}
				}
			}

			// Linux needs this to get files in the right order
			sort(fileNames.begin(), fileNames.end());
			for (int fileIndex = 0; fileIndex < int(fileNames.size() - 1); fileIndex++) {
				if ((directoryName + "/" + fileNames[fileIndex]) == path) {
					sampnumber = fileIndex;
				}
			}

			closedir(directory);
			isReloading = false;
		}

		lastPath = path;
	} else {

		// no sampleData loaded
		isFileLoaded = false;
	}
}

/**
 * Manages an engine::Module in the rack.
 * rack::app::ModuleWidget
 * @see https://vcvrack.com/docs/structrack_1_1app_1_1ModuleWidget.html
 */
struct WavPlayWidget : ModuleWidget {
	WavPlay* wavPlay;
	
	WavPlayWidget(WavPlay* module) {
		wavPlay = module;

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WavPlay.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 43.947)), module, WavPlay::PITCH_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 65.535)), module, WavPlay::TRIGGER_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 87.124)), module, WavPlay::PITCH_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 108.713)), module, WavPlay::AUDIO_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(15.24, 25.81)), module, WavPlay::ISPLAYING_LIGHT));
	};

	void appendContextMenu(Menu *menu) override {
	
		// empty spacer
		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		struct LoadWavMenuItem : MenuItem {
			WavPlay *wavPlay;
			void onAction(const event::Action& e) override {
				std::string directoryName = wavPlay->lastPath.empty() ? asset::user("") : rack::string::directory(wavPlay->lastPath);
				char *path = osdialog_file(OSDIALOG_OPEN, directoryName.c_str(), NULL, NULL);
				if (path) {
					wavPlay->loadWavFile(path);
					wavPlay->lastPath = path;
					free(path);
				}
			};
		};

		LoadWavMenuItem *loadWavMenuItem = new LoadWavMenuItem();
		loadWavMenuItem->text = "Load WAV file";
		loadWavMenuItem->wavPlay = wavPlay;
		menu->addChild(loadWavMenuItem);
	};
};

/**
 * Creates a headless Module. (whatever that means)
 * rack::plugin::Model
 * @param {String} Module fileName.
 * @returns {engine::Module}
 * @see https://vcvrack.com/docs/structrack_1_1plugin_1_1Model.html
 */
Model* modelWavPlay = createModel<WavPlay, WavPlayWidget>("WavPlay");
