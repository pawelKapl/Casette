#pragma once

struct ModulatorSettings
{
	int enabled = false;
	int mix = 50;
	int depth = 5;
	int rate = 500;
	int width = 0;
	int feedback = 0;
	int vibratoMode = 0;

	int* getParam(int index)
	{
		switch (index)
		{
			case 0:
				return &enabled;
			case 1:
				return &mix;
			case 2:
				return &depth;
			case 3:
				return &rate;
			case 4:
				return &width;
			case 5:
				return &feedback;
			case 6:
				return &vibratoMode;
			default:
				return nullptr;
		}
	}
};

inline void to_json(nlohmann::json& j, const ModulatorSettings& d) {
    j = nlohmann::json{
        {"mix", d.mix},
        {"enabled", d.enabled},
		{"depth", d.depth},
		{"rate", d.rate},
		{"width", d.width},
		{"feedback", d.feedback},
		{"vibrato", d.vibratoMode}
    };
}

inline void from_json(const nlohmann::json& j, ModulatorSettings& d) {
    j.at("mix").get_to(d.mix);
    j.at("enabled").get_to(d.enabled);
	j.at("depth").get_to(d.depth);
	j.at("rate").get_to(d.rate);
	j.at("width").get_to(d.width);
	j.at("feedback").get_to(d.feedback);
	j.at("vibrato").get_to(d.vibratoMode);
}