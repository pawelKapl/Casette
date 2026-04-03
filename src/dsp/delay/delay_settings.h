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
        {"enabled", d.enabled}
    };
}

inline void from_json(const nlohmann::json& j, DelaySettings& d) 
{
    j.at("delayMode").get_to(d.delayMode);
    j.at("mix").get_to(d.mix);
    j.at("feedback").get_to(d.feedback);
    j.at("time").get_to(d.time);
    j.at("time2").get_to(d.time2);
    j.at("hiPass").get_to(d.hiPass);
    j.at("loPass").get_to(d.loPass);
    j.at("diffusion").get_to(d.diffusion);
    j.at("modRate").get_to(d.modRate);
    j.at("enabled").get_to(d.enabled);
}