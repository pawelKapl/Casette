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
    d.reverbMode = j.value("reverbMode", d.reverbMode);
    d.mix = j.value("mix", d.mix);
    d.damping = j.value("damping", d.damping);
    d.preDelay = j.value("preDelay", d.preDelay);
    d.roomSize = j.value("roomSize", d.roomSize);
    d.hiPass = j.value("hiPass", d.hiPass);
    d.loPass = j.value("loPass", d.loPass);
    d.enabled = j.value("enabled", d.enabled);
	d.decay = j.value("decay", d.decay);
}