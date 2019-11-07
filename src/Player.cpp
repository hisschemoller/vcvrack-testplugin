#include "plugin.hpp"

/**
 * DSP processor
 * rack::engine::Module
 * @see https://vcvrack.com/docs/structrack_1_1engine_1_1Module.html
 */
struct Player : Module {

  // Constructs a Module with no params, inputs, outputs, and lights.
  Player() {

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
struct PlayerWidget : ModuleWidget {

  /**
   * Constructor
   * @param {Player*} player Pointer to the Module.
   */
  PlayerWidget(Player* player) {

  }
};

/**
 * Creates a headless Module. (whatever that means)
 * rack::plugin::Model
 * @param {String} Module name.
 * @returns {engine::Module}
 * @see https://vcvrack.com/docs/structrack_1_1plugin_1_1Model.html
 */
Model* modelPlayer = createModel<Player, PlayerWidget>("Player");
