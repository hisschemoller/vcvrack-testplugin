#include "plugin.hpp"

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

	// Constructs a Module with no params, inputs, outputs, and lights.
	WavPlay() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PITCH_PARAM, 0.f, 1.f, 0.f, "");
	}

	// Advances the module by one audio sample.
	void process(const ProcessArgs& args) override {
	}
};

/**
 * Manages an engine::Module in the rack.
 * rack::app::ModuleWidget
 * @see https://vcvrack.com/docs/structrack_1_1app_1_1ModuleWidget.html
 */
struct WavPlayWidget : ModuleWidget {

  /**
   * Constructor
   * @param {WavPlay*} module Pointer to the Module.
   */
	WavPlayWidget(WavPlay* module) {
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
	}
};

/**
 * Creates a headless Module. (whatever that means)
 * rack::plugin::Model
 * @param {String} Module name.
 * @returns {engine::Module}
 * @see https://vcvrack.com/docs/structrack_1_1plugin_1_1Model.html
 */
Model* modelWavPlay = createModel<WavPlay, WavPlayWidget>("WavPlay");
