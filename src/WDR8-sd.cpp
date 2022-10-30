#include "plugin.hpp"

#include "WDF_structs.hpp"

// very short current peak with exponential decay, constants selected manually by fitting curves with LTspice
float I1(float x) {
    if (x <= 0.0) return 0.0;
    return -0.045 * ( 1 / std::exp(x/0.000177));
}

struct WDR8sd : Module {

	chowdsp::ButterworthFilter< 2, chowdsp::ButterworthFilterType::Highpass, float> fi;

	EnvelopeGenerator env;
	DiodeClipper vca;
	SnareResonatorLow resoLow;
	SnareResonatorHigh resoHigh;

	enum ParamId {
		RESOHIGH_FREQ_PARAM,
		RESOHIGH_VOL_PARAM,
		RESOLOW_FREQ_PARAM,
		RESOLOW_VOL_PARAM,
		RESO_TUNE_PARAM,
		ENVCAP_PARAM,
		ENVRES_PARAM,
		ENVATTEN_PARAM,
		NOISEFREQ_PARAM,
		NOISERESO_PARAM,
		TONE_PARAM,
		SNAPPY_PARAM,
		TONECV_PARAM,
		SNAPPYCV_PARAM,
		ENVRESCV_PARAM,
		TUNINGCV_PARAM,
		ACCENT_PARAM,
		VOLUME_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRIGGER_INPUT,
		ACCENT_INPUT,
		TONE_INPUT,
		SNAPPY_INPUT,
		ENVRES_INPUT,
		TUNING_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		RESOLOW_OUTPUT,
		RESOHIGH_OUTPUT,
		ENV_OUTPUT,
		CLIP_OUTPUT,
		FULL_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	bool prepared = false;
	float I1time = 1000.0; // offset I1 time (envelope current pulse) so that it doesn't trigger when the module initializes, I1time is reset to 0.0 on every trigger

	dsp::SchmittTrigger trigger;
	dsp::PulseGenerator pulse;
	dsp::ClockDivider parametersDivider;

	WDR8sd() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RESOHIGH_FREQ_PARAM, -1.0f, 1.0f, 0.0f, "High resonator frequency");
		configParam(RESOHIGH_VOL_PARAM, 0.0f, 2.0f, 1.0f, "High resonator volume");
		configParam(RESOLOW_FREQ_PARAM, -1.0f, 1.0f, 0.0f, "Low resonator frequency");
		configParam(RESOLOW_VOL_PARAM, 0.0f, 4.0f, 1.0f, "Low resonator volume");
		configParam(RESO_TUNE_PARAM, -1.0f, 1.0f, 0.0f, "Resonators tuning");
		configParam(ENVCAP_PARAM, -1.0f, 1.0f, 0.0f, "Envelope capacitor");
		configParam(ENVRES_PARAM, 1.0f, -1.0f, 0.0f, "Envelope resistor");
		configParam(ENVATTEN_PARAM, 0.0f, 2.0f, 1.0f, "Envelope attenuator");
		configParam(NOISERESO_PARAM, 0.01f, 3.0f, 1.0f, "Noise high-pass resonance");
		configParam(TONE_PARAM, 0.0f, 1.0f, 0.5f, "Tone");
		configParam(SNAPPY_PARAM, 0.1f, 2.0f, 0.75f, "Snappy");
		configParam(TONECV_PARAM, 0.0f, 1.0f, 0.0f, "Tone modulation");
		configParam(SNAPPYCV_PARAM, 0.0f, 1.0f, 0.0f, "Snappy modulation");
		configParam(ENVRESCV_PARAM, 0.0f, 1.0f, 0.0f, "Env. decay modulation");
		configParam(TUNINGCV_PARAM, 0.0f, 1.0f, 0.0f, "Main tuning modulation");
		configParam(ACCENT_PARAM, 0.0f, 1.0f, 0.5f, "Accent amount");
		configParam(VOLUME_PARAM, 0.0f, 2.0f, 1.0f, "Output volume");

		configParam(NOISEFREQ_PARAM, 150.f, 10000.f, 2841.f, "Noise high-pass frequency", " Hz");

		configInput(TRIGGER_INPUT, "Trigger");
		configInput(ACCENT_INPUT, "Accent");
		configInput(TONE_INPUT, "Tone CV");
		configInput(SNAPPY_INPUT, "Snappy CV");
		configInput(ENVRES_INPUT, "Envelope resistor/decay CV");
		configInput(TUNING_INPUT, "Main tuning CV");

		configOutput(RESOLOW_OUTPUT, "Low resonator");
		configOutput(RESOHIGH_OUTPUT, "High resonator");
		configOutput(ENV_OUTPUT, "Envelope");
		configOutput(CLIP_OUTPUT, "Clipper VCA");
		configOutput(FULL_OUTPUT, "Full");

		parametersDivider.setDivision(16);
	}

	int overSampling = 4;
	void prepare(float sr) {
		env.prepare(sr);
		env.reset();

		vca.prepare(sr * overSampling);
		vca.reset();

		resoLow.prepare(sr);
		resoLow.reset();

		resoHigh.prepare(sr);
		resoHigh.reset();

		fi.prepare(1);
		fi.calcCoefs(2841, 1.0, sr);

		prepared = true;
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override {
		prepare(e.sampleRate);
	}

	const float noiseGain = 5.f / std::sqrt(2.f);
	float lastWhite = 0.f;
	float noise() {
		float white = random::normal();
		float violet = (white - lastWhite) / 1.41f;
		lastWhite = white;
		return violet * noiseGain;
	}

	float lastNoiseRes_Param;
	float lastNoiseFreq_Param;
	float lastEnvCap_Param;
	float lastEnvRes_Param;
	float filteredPulse = 0.0;
	bool accent;
	void process(const ProcessArgs& args) override {
		I1time += args.sampleTime;

		if (trigger.process(inputs[TRIGGER_INPUT].getVoltage())) {
			I1time = 0.0;
			pulse.trigger(1e-3); // Gate time is 1ms
			accent = inputs[ACCENT_INPUT].getVoltage() > 0.1; // Latch accent to trigger
		}

		if (!prepared) prepare(args.sampleRate);

		if (parametersDivider.process()) {
			if (params[NOISERESO_PARAM].getValue() != lastNoiseRes_Param ||
				params[NOISEFREQ_PARAM].getValue() != lastNoiseFreq_Param) {
				lastNoiseFreq_Param = params[NOISEFREQ_PARAM].getValue();
				lastNoiseRes_Param = params[NOISERESO_PARAM].getValue();
				fi.calcCoefs(lastNoiseFreq_Param, lastNoiseRes_Param, args.sampleRate);
			}
			if (!inputs[TUNING_INPUT].isConnected()) {
				resoHigh.setRackParameters((params[RESOHIGH_FREQ_PARAM].getValue() + params[RESO_TUNE_PARAM].getValue()) * 0.5);
				resoLow.setRackParameters((params[RESOLOW_FREQ_PARAM].getValue() + params[RESO_TUNE_PARAM].getValue()) * 0.5);
			}
			if (((params[ENVCAP_PARAM].getValue() != lastEnvCap_Param ||
				params[ENVRES_PARAM].getValue() != lastEnvRes_Param)) &&
				(!inputs[ENVRES_INPUT].isConnected())) {
				lastEnvRes_Param = params[ENVRES_PARAM].getValue();
				lastEnvCap_Param = params[ENVCAP_PARAM].getValue();
				env.setRackParameters(params[ENVCAP_PARAM].getValue(), params[ENVRES_PARAM].getValue());
			}
		}
		if (inputs[ENVRES_INPUT].isConnected()) {
			lastEnvRes_Param = params[ENVRES_PARAM].getValue();
			lastEnvCap_Param = params[ENVCAP_PARAM].getValue();
			float envres_param = math::clamp(params[ENVRES_PARAM].getValue() + (params[ENVRESCV_PARAM].getValue() * 
								 (inputs[ENVRES_INPUT].getVoltage() / 10.f)) * -1.f, -1.f, 1.f);
			env.setRackParameters(params[ENVCAP_PARAM].getValue(), envres_param);
		}
		if (inputs[TUNING_INPUT].isConnected()) {
			float reso_tune_param = math::clamp(params[RESO_TUNE_PARAM].getValue() + (params[TUNINGCV_PARAM].getValue() *
									(inputs[TUNING_INPUT].getVoltage() / 10.f)), -1.f, 1.f);
			resoHigh.setRackParameters((params[RESOHIGH_FREQ_PARAM].getValue() + reso_tune_param) * 0.5);
			resoLow.setRackParameters((params[RESOLOW_FREQ_PARAM].getValue() + reso_tune_param) * 0.5);
		}

		float overSampled = 0.0;
        for (int i = 0; i < overSampling; ++i)
        {
            overSampled += vca.processSample(noise() * 0.5);
        }
        overSampled /= overSampling;

        // All the constants and multipliers below were selected either by fitting curves with simulation or by subjective listening tests
        float accent_boost = (accent ? params[ACCENT_PARAM].getValue() * 10.f : 0.f);
		bool pulse_high = pulse.process(args.sampleTime);
		filteredPulse += ((pulse_high ? 4.f + accent_boost : 0.0) - filteredPulse) * 0.344;
		float resoLow_out = 0.8255 * resoLow.processSample(filteredPulse);
		float resoHigh_out = -0.63085 * resoHigh.processSample((pulse_high ? 26.126e-3 * (4.f + accent_boost) : 0.0));
		resoLow_out *= params[RESOLOW_VOL_PARAM].getValue();
		resoHigh_out *= params[RESOHIGH_VOL_PARAM].getValue();

		float tone_param = math::clamp(params[TONE_PARAM].getValue() + 
						   (params[TONECV_PARAM].getValue() * (inputs[TONE_INPUT].getVoltage() / 10.f)), 0.0, 1.0);
		float resonators = resoHigh_out * 0.5 * tone_param + resoLow_out * 0.25 * (1 - tone_param);

		float env_out = 0.00926 * (env.processSample(I1(I1time))) * (4.f + accent_boost) * params[ENVATTEN_PARAM].getValue();
		float vca_out = 2 * (overSampled + 0.425) + 0.66;

		float snappy_param = math::clamp(params[SNAPPY_PARAM].getValue() + math::rescale(
						     params[SNAPPYCV_PARAM].getValue() * (inputs[SNAPPY_INPUT].getVoltage() / 10.f), 0.f, 1.f, 0.1f, 2.f),
						     0.1f, 2.f);
		float out = params[VOLUME_PARAM].getValue() * 4.0 * (fi.processSample(vca_out * env_out) * snappy_param + resonators);
		outputs[RESOLOW_OUTPUT].setVoltage(resoLow_out);
		outputs[RESOHIGH_OUTPUT].setVoltage(resoHigh_out);
		outputs[ENV_OUTPUT].setVoltage(env_out);
		outputs[CLIP_OUTPUT].setVoltage(vca_out);
		outputs[FULL_OUTPUT].setVoltage(out);
	}
};

struct WDR8OrangeKnob : RoundSmallBlackKnob {
    WDR8OrangeKnob() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/WDR-8_SmallKnob.svg")));
        bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/WDR-8_SmallKnob_Orange_bg.svg")));
    }
};

struct WDR8WhiteKnob : RoundSmallBlackKnob {
    WDR8WhiteKnob() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/WDR-8_SmallKnob.svg")));
        bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/WDR-8_SmallKnob_White_bg.svg")));
    }
};

struct WDR8YellowKnob : RoundSmallBlackKnob {
    WDR8YellowKnob() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/WDR-8_SmallKnob.svg")));
        bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/WDR-8_SmallKnob_Yellow_bg.svg")));
    }
};

struct WDR8Trimpot : Trimpot {
    WDR8Trimpot() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/WDR-8_Trimpot.svg")));
        bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/WDR-8_Trimpot_bg.svg")));
    }
};

struct WDR8TinyTrimpot : Trimpot {
    WDR8TinyTrimpot() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/WDR-8_TinyTrimpot.svg")));
        bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/WDR-8_TinyTrimpot_bg.svg")));
    }
};

struct WDR8sdWidget : ModuleWidget {
	WDR8sdWidget(WDR8sd* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/wdr-8-sd_vector.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// guides X and Y values, see design SVG file
		float xGuides[] = {9.68, 22.74, 37.71};
		float yGuides[] = {25.74, 38.74, 45.75, 61.34, 72.2, 71.02, 85.41, 93.67, 101.27};
		addParam(createParamCentered<WDR8OrangeKnob>(mm2px(Vec(xGuides[0], yGuides[0])), module, WDR8sd::RESOLOW_FREQ_PARAM));
		addParam(createParamCentered<WDR8OrangeKnob>(mm2px(Vec(xGuides[1], yGuides[0])), module, WDR8sd::RESOHIGH_FREQ_PARAM));
		addParam(createParamCentered<WDR8OrangeKnob>(mm2px(Vec(xGuides[2], yGuides[0])), module, WDR8sd::ENVCAP_PARAM));
		
		addParam(createParamCentered<WDR8WhiteKnob>(mm2px(Vec(xGuides[0], yGuides[1])), module, WDR8sd::RESOLOW_VOL_PARAM));
		addParam(createParamCentered<WDR8WhiteKnob>(mm2px(Vec(xGuides[1], yGuides[1])), module, WDR8sd::RESOHIGH_VOL_PARAM));
		addParam(createParamCentered<WDR8WhiteKnob>(mm2px(Vec(xGuides[2], yGuides[1])), module, WDR8sd::ENVRES_PARAM));

		addParam(createParamCentered<WDR8WhiteKnob>(mm2px(Vec(xGuides[2], yGuides[3])), module, WDR8sd::NOISEFREQ_PARAM));
		addParam(createParamCentered<WDR8Trimpot>(mm2px(Vec(xGuides[2], yGuides[4])), module, WDR8sd::NOISERESO_PARAM));

		addParam(createParamCentered<WDR8YellowKnob>(mm2px(Vec(16.14, 56.57)), module, WDR8sd::RESO_TUNE_PARAM));

		addParam(createParamCentered<WDR8Trimpot>(mm2px(Vec(xGuides[2], 50.25)), module, WDR8sd::ENVATTEN_PARAM));

		addParam(createParamCentered<WDR8OrangeKnob>(mm2px(Vec(xGuides[0], yGuides[5])), module, WDR8sd::TONE_PARAM));
		addParam(createParamCentered<WDR8OrangeKnob>(mm2px(Vec(xGuides[1], yGuides[5])), module, WDR8sd::SNAPPY_PARAM));
		addParam(createParamCentered<WDR8YellowKnob>(mm2px(Vec(xGuides[0], yGuides[6])), module, WDR8sd::ACCENT_PARAM));
		addParam(createParamCentered<WDR8YellowKnob>(mm2px(Vec(xGuides[1], yGuides[6])), module, WDR8sd::VOLUME_PARAM));

		addParam(createParamCentered<WDR8TinyTrimpot>(mm2px(Vec(xGuides[0], yGuides[7])), module, WDR8sd::TONECV_PARAM));
		addParam(createParamCentered<WDR8TinyTrimpot>(mm2px(Vec(xGuides[1], yGuides[7])), module, WDR8sd::SNAPPYCV_PARAM));
		addParam(createParamCentered<WDR8TinyTrimpot>(mm2px(Vec(xGuides[2], yGuides[7])), module, WDR8sd::ENVRESCV_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xGuides[0], 113.81)), module, WDR8sd::TRIGGER_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xGuides[1], 113.81)), module, WDR8sd::ACCENT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xGuides[0], yGuides[8])), module, WDR8sd::TONE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xGuides[1], yGuides[8])), module, WDR8sd::SNAPPY_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xGuides[2], yGuides[8])), module, WDR8sd::ENVRES_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xGuides[2], yGuides[6]-2)), module, WDR8sd::TUNING_INPUT));
		addParam(createParamCentered<WDR8TinyTrimpot>(mm2px(Vec(42.5, 78.5)), module, WDR8sd::TUNINGCV_PARAM));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xGuides[2], 113.81)), module, WDR8sd::FULL_OUTPUT));
	}
};

Model* modelWDR8sd = createModel<WDR8sd, WDR8sdWidget>("WDR8sd");