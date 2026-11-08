#pragma once

#include "dsp/octaver/octaver.h"

struct OctaverSettings
{
	int mode = 0;
	int dry = 100;
	int sub2 = 0;
	int sub1 = 70;
	int up1 = 0;
	int up2 = 0;
	int tone = 60;
	int enabled = false;

	static constexpr int paramCount = 8;

	int* getParam(int index)
	{
		switch (index)
		{
			case 0:
				return &mode;
			case 1:
				return &dry;
			case 2:
				return &sub2;
			case 3:
				return &sub1;
			case 4:
				return &up1;
			case 5:
				return &up2;
			case 6:
				return &tone;
			case 7:
				return &enabled;
			default:
				return nullptr;
		}
	}
};

inline void to_json(nlohmann::json& j, const OctaverSettings& d)
{
    j = nlohmann::json{
        {"mode", d.mode},
        {"dry", d.dry},
        {"sub2", d.sub2},
        {"sub1", d.sub1},
        {"up1", d.up1},
        {"up2", d.up2},
        {"tone", d.tone},
        {"enabled", d.enabled}
    };
}

inline void from_json(const nlohmann::json& j, OctaverSettings& d)
{
    d.mode = j.value("mode", d.mode);
    d.dry = j.value("dry", d.dry);
    d.sub2 = j.value("sub2", d.sub2);
    d.sub1 = j.value("sub1", d.sub1);
    d.up1 = j.value("up1", d.up1);
    d.up2 = j.value("up2", d.up2);
    d.tone = j.value("tone", d.tone);
    d.enabled = j.value("enabled", d.enabled);
}

// shared by the pre chain and tools/octaver_render
inline void applyOctaverParam(Octaver &octaver, const OctaverSettings &s, int param)
{
	switch (param)
	{
		case 0:
			octaver.setMode(OctaverMode(s.mode));
			break;
		case 1:
			octaver.setDry(float(s.dry) / 100.);
			break;
		case 2:
			octaver.setVoiceLevel(0, float(s.sub2) / 100.);
			break;
		case 3:
			octaver.setVoiceLevel(1, float(s.sub1) / 100.);
			break;
		case 4:
			octaver.setVoiceLevel(2, float(s.up1) / 100.);
			break;
		case 5:
			octaver.setVoiceLevel(3, float(s.up2) / 100.);
			break;
		case 6:
			octaver.setTone(float(s.tone) / 100.);
			break;
		case 7:
			octaver.setOn(s.enabled);
			break;
	}
}

inline void applyOctaverSettings(Octaver &octaver, const OctaverSettings &s)
{
	for (int param = 0; param < OctaverSettings::paramCount; param++)
		applyOctaverParam(octaver, s, param);
}
