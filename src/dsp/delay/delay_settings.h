#pragma once

struct DelaySettings
{
	int delayMode = 0;
	int mix = 30;
	int feedback = 30;
	int time = 292;
	int time2 = 584;
	int hiPass = 100;
	int loPass = 4000;
	int diffusion = 65;
	int modRate = 1;
	int enabled = false;
	int reverseWetOnly = false;

	int* getParam(int index)
	{
		switch (index)
		{
			case 0:
				return &delayMode;
			case 1:
				return &mix;
			case 2:
				return &feedback;
			case 3:
				return &time;
			case 4:
				return &time2;
			case 5:
				return &hiPass;
			case 6:
				return &loPass;
			case 7:
				return &diffusion;
			case 8:
				return &modRate;
			case 9:
				return &enabled;
			case 10:
				return &reverseWetOnly;
			default:
				return nullptr;
		}
	}
};

inline void to_json(nlohmann::json& j, const DelaySettings& d) 
{
    j = nlohmann::json{
        {"delayMode", d.delayMode},
        {"mix", d.mix},
        {"feedback", d.feedback},
        {"time", d.time},
        {"time2", d.time2},
        {"hiPass", d.hiPass},
        {"loPass", d.loPass},
        {"diffusion", d.diffusion},
        {"modRate", d.modRate},
        {"enabled", d.enabled},
        {"reverseWetOnly", d.reverseWetOnly}
    };
}

inline void from_json(const nlohmann::json& j, DelaySettings& d) 
{
    d.delayMode = j.value("delayMode", d.delayMode);
    d.mix = j.value("mix", d.mix);
    d.feedback = j.value("feedback", d.feedback);
    d.time = j.value("time", d.time);
    d.time2 = j.value("time2", d.time2);
    d.hiPass = j.value("hiPass", d.hiPass);
    d.loPass = j.value("loPass", d.loPass);
    d.diffusion = j.value("diffusion", d.diffusion);
    d.modRate = j.value("modRate", d.modRate);
    d.enabled = j.value("enabled", d.enabled);
    d.reverseWetOnly = j.value("reverseWetOnly", d.reverseWetOnly);
}