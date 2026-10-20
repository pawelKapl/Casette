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
    d.mix = j.value("mix", d.mix);
    d.enabled = j.value("enabled", d.enabled);
	d.depth = j.value("depth", d.depth);
	d.rate = j.value("rate", d.rate);
	d.width = j.value("width", d.width);
	d.feedback = j.value("feedback", d.feedback);
	d.vibratoMode = j.value("vibrato", d.vibratoMode);
}