#include "plugin.hpp"
#include "cmath"
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
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIOOUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ISPLAYING_LIGHT,
		NUM_LIGHTS
	};

	bool isLoading = false;
	bool isFileLoaded = true;
	bool isPlaying = false;
	unsigned int channels;
	unsigned int sampleRate;
	drwav_uint64 totalSampleCount;
	std::string lastPath = "";
	std::string fileDesc = "";
	std::vector<std::vector<float>> playBuffer;

	// Constructs a Module with no params, inputs, outputs, and lights.
	WavPlay() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PITCH_PARAM, 0.f, 1.f, 0.f, "");
	}

	// Advances the module by one audio sample.
	void process(const ProcessArgs& args) override {
		lights[ISPLAYING_LIGHT].setBrightness(isPlaying ? 1.f : 0.f);
	}

	void loadWavFile(std::string path);
};

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
		playBuffer[0].clear();
		for (unsigned int i = 0; i < _totalSampleCount; i++) {
			playBuffer[0].push_back(pSampleData[i]);
		}
		totalSampleCount = playBuffer[0].size();
		drwav_free(pSampleData);

		isLoading = false;
		isFileLoaded = true;
		fileDesc = rack::string::filename(path);
		DEBUG("loadWavFile");
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

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 46.063)), module, WavPlay::PITCH_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 77.478)), module, WavPlay::PITCH_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 108.713)), module, WavPlay::AUDIOOUT_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(15.24, 25.81)), module, WavPlay::ISPLAYING_LIGHT));
	};

	void appendContextMenu(Menu *menu) override {
	
		// empty spacer
		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		struct LoadWavMenuItem : MenuItem {
			WavPlay *wavPlay;
			void onAction(const event::Action& e) override {
				std::string dir = wavPlay->lastPath.empty() ? asset::user("") : rack::string::directory(wavPlay->lastPath);
				char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, NULL);
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
 * @param {String} Module name.
 * @returns {engine::Module}
 * @see https://vcvrack.com/docs/structrack_1_1plugin_1_1Model.html
 */
Model* modelWavPlay = createModel<WavPlay, WavPlayWidget>("WavPlay");
