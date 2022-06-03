#include "plugin.hpp"


struct DummyModule : Module {
	enum ParamId {
		K1_PARAM,
		K2_PARAM,
		// FMCV_PARAM,
		B1_PARAM,
		B2_PARAM,
		// PWMCV_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		// FM_INPUT,
		// PITCH_INPUT,
		// SYNC_INPUT,
		// PWM_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		// SIN_OUTPUT,
		// TRI_OUTPUT,
		// SAW_OUTPUT,
		// SQR_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		// FREQ_LIGHT,
		LIGHTS_LEN
	};

	DummyModule() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(K1_PARAM, 0.f, 1.f, 0.f, "K1_PARAM");
		configParam(K2_PARAM, 0.f, 1.f, 0.f, "K2_PARAM");
		// configParam(FMCV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(B1_PARAM, 0.f, 1.f, 0.f, "B1_PARAM");
		configParam(B2_PARAM, 0.f, 1.f, 0.f, "B2_PARAM");
		// configParam(PWMCV_PARAM, 0.f, 1.f, 0.f, "");
		// configInput(FM_INPUT, "");
		// configInput(PITCH_INPUT, "");
		// configInput(SYNC_INPUT, "");
		// configInput(PWM_INPUT, "");
		// configOutput(SIN_OUTPUT, "");
		// configOutput(TRI_OUTPUT, "");
		// configOutput(SAW_OUTPUT, "");
		// configOutput(SQR_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct DummyModuleWidget : ModuleWidget {
	DummyModuleWidget(DummyModule* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/dummy-panel.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(22.905, 29.808)), module, DummyModule::K1_PARAM));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(22.862, 56.388)), module, DummyModule::K2_PARAM));
		// addParam(createParamCentered<Trimpot>(mm2px(Vec(6.607, 80.603)), module, DummyModule::FMCV_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(15.444, 80.603)), module, DummyModule::B1_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(30.282, 80.603)), module, DummyModule::B2_PARAM));
		// addParam(createParamCentered<Trimpot>(mm2px(Vec(39.118, 80.603)), module, DummyModule::PWMCV_PARAM));

		// addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.607, 96.859)), module, DummyModule::FM_INPUT));
		// addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.444, 96.859)), module, DummyModule::PITCH_INPUT));
		// addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28.282, 96.859)), module, DummyModule::SYNC_INPUT));
		// addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.15, 96.859)), module, DummyModule::PWM_INPUT));

		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.607, 113.115)), module, DummyModule::SIN_OUTPUT));
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(17.444, 113.115)), module, DummyModule::TRI_OUTPUT));
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(28.282, 113.115)), module, DummyModule::SAW_OUTPUT));
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.119, 113.115)), module, DummyModule::SQR_OUTPUT));

		// addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(31.089, 16.428)), module, DummyModule::FREQ_LIGHT));
	}
};


Model* modelDummyModule = createModel<DummyModule, DummyModuleWidget>("DummyModule");