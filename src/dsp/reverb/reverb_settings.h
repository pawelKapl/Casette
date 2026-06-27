#pragma once

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
		{"decay", d.decay}
    };
}

inline void from_json(const nlohmann::json& j, ReverbSettings& d) {
    j.at("reverbMode").get_to(d.reverbMode);
    j.at("mix").get_to(d.mix);
    j.at("damping").get_to(d.damping);
    j.at("preDelay").get_to(d.preDelay);
    j.at("roomSize").get_to(d.roomSize);
    j.at("hiPass").get_to(d.hiPass);
    j.at("loPass").get_to(d.loPass);
    j.at("enabled").get_to(d.enabled);
	d.decay = j.value("decay", 40);
}