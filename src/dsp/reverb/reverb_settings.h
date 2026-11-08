#pragma once

#include "dsp/reverb/reverb.h"

struct ReverbSettings
{
	int reverbMode = 0;
	int mix = 20;
	int roomSize = 50;
	int damping = 10;
	int preDelay = 31;
	int hiPass = 300;
	int loPass = 3500;
	int enabled = false;
	int decay = 40;
	int dwell = 40;
	int tension = 50;
	int springs = 2;
	int modDepth = 7;
	int modRate = 400;
	int freeze = false;

	static constexpr int paramCount = 15;

	int* getParam(int index)
	{
		switch (index)
		{
			case 0:
				return &reverbMode;
			case 1:
				return &mix;
			case 2:
				return &roomSize;
			case 3:
				return &damping;
			case 4:
				return &preDelay;
			case 5:
				return &hiPass;
			case 6:
				return &loPass;
			case 7:
				return &enabled;
			case 8:
				return &decay;
			case 9:
				return &dwell;
			case 10:
				return &tension;
			case 11:
				return &springs;
			case 12:
				return &modDepth;
			case 13:
				return &modRate;
			case 14:
				return &freeze;
			default:
				return nullptr;
		}
	}
};

inline void to_json(nlohmann::json& j, const ReverbSettings& d) {
    j = nlohmann::json{
        {"reverbMode", d.reverbMode},
        {"mix", d.mix},
        {"damping", d.damping},
        {"preDelay", d.preDelay},
        {"roomSize", d.roomSize},
        {"hiPass", d.hiPass},
        {"loPass", d.loPass},
        {"enabled", d.enabled},
		{"decay", d.decay},
		{"dwell", d.dwell},
		{"tension", d.tension},
		{"springs", d.springs},
		{"modDepth", d.modDepth},
		{"modRate", d.modRate},
		{"freeze", d.freeze}
    };
}

inline void from_json(const nlohmann::json& j, ReverbSettings& d) {
    d.reverbMode = j.value("reverbMode", d.reverbMode);
    d.mix = j.value("mix", d.mix);
    d.damping = j.value("damping", d.damping);
    d.preDelay = j.value("preDelay", d.preDelay);
    d.roomSize = j.value("roomSize", d.roomSize);
    d.hiPass = j.value("hiPass", d.hiPass);
    d.loPass = j.value("loPass", d.loPass);
    d.enabled = j.value("enabled", d.enabled);
	d.decay = j.value("decay", d.decay);
	d.dwell = j.value("dwell", d.dwell);
	d.tension = j.value("tension", d.tension);
	d.springs = j.value("springs", d.springs);
	d.modDepth = j.value("modDepth", d.modDepth);
	d.modRate = j.value("modRate", d.modRate);
	d.freeze = j.value("freeze", d.freeze);
}

// shared by the fx chain and tools/reverb_render
inline void applyReverbParam(Reverb &reverb, const ReverbSettings &s, int param)
{
	switch (param)
	{
		case 0:
			reverb.setMode(ReverbMode(s.reverbMode));
			break;
		case 1:
			reverb.setMix(float(s.mix) / 100.);
			break;
		case 2:
		case 8:
			reverb.setSizeDecay(float(s.roomSize) / 100., float(s.decay) / 100.);
			break;
		case 3:
			reverb.setDamping(float(s.damping) / 100.);
			break;
		case 4:
			reverb.setPreDelay(float(s.preDelay) / 1000.);
			break;
		case 5:
			reverb.setHighPass(s.hiPass);
			break;
		case 6:
			reverb.setLowPass(s.loPass);
			break;
		case 9:
			reverb.setDwell(float(s.dwell) / 100.);
			break;
		case 10:
			reverb.setTension(float(s.tension) / 100.);
			break;
		case 11:
			reverb.setSprings(s.springs);
			break;
		case 12:
			reverb.setModDepth(float(s.modDepth) / 20.);
			break;
		case 13:
			reverb.setModRate(float(s.modRate) / 1000.);
			break;
		case 14:
			reverb.setFreeze(s.freeze);
			break;
	}
}

// for a freshly created reverb
inline void applyReverbSettings(Reverb &reverb, const ReverbSettings &s)
{
	for (int param = 0; param < ReverbSettings::paramCount; param++)
		applyReverbParam(reverb, s, param);
	reverb.settle();
}