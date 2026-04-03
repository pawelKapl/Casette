#pragma once

#include "env.h"
#include "storage.h"

struct NoiseGateParams
{
    float enabled = true;
    float sampleRate = 48000;
    float threshold = -60.0f;
    float compRatio = 0.5f;
    float attack = 0.004f;
    float release = 0.01f;
    float holdTime = 0.02f;
};

inline void to_json(nlohmann::json& j, const NoiseGateParams& p) 
{
    j = nlohmann::json{
        {"enabled", p.enabled},
        {"sampleRate", p.sampleRate},
        {"threshold", p.threshold},
        {"compRatio", p.compRatio},
        {"attack", p.attack},
        {"release", p.release},
        {"holdTime", p.holdTime}
    };
}

inline void from_json(const nlohmann::json& j, NoiseGateParams& p) 
{
    j.at("enabled").get_to(p.enabled);
    j.at("sampleRate").get_to(p.sampleRate);
    j.at("threshold").get_to(p.threshold);
    j.at("compRatio").get_to(p.compRatio);
    j.at("attack").get_to(p.attack);
    j.at("release").get_to(p.release);
    j.at("holdTime").get_to(p.holdTime);
}

class NoiseGate
{
public:
    NoiseGate();

    void process(const float *input, float *output, const size_t numFrames);
    void setParams(const NoiseGateParams &params);
    void setThresholdDB(float threshold);
    void reload();

    inline const NoiseGateParams *gateSettings()
    {
        return &_settings;
    }

    inline void enabled(bool on) 
    { 
        _settings.enabled = on; 
        Storage::get().persist<NoiseGateParams>(_settings, "ng");
    
    }
    inline bool isEnabled() { return _settings.enabled; }

private:
    NoiseGateParams _settings;

    float _envelope = 0.0f;
    float _gain = 1.0f;
    float _holdCounter = 0.0f;
    bool _gateOpen = false;

    float _attackCoeff = 0.0f;
    float _releaseCoeff = 0.0f;

    float _thresholdP = 0.0f;

};