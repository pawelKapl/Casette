#pragma once

#include "dsp/wav.h"
#include "FFTConvolver.h"
#include "storage.h"
#include "dsp/IIR_filter.h"

struct CabSimulatorSettings
{
    bool enabled = true;
    std::string model = "Mesa 4x12 sm57";
    int hiPass = 5;
    int loPass = 16000;
};

inline void to_json(nlohmann::json& j, const CabSimulatorSettings& a) {
    j = nlohmann::json{
        {"model", a.model},
        {"enabled", a.enabled},
        {"hiPass", a.hiPass},
        {"loPass", a.loPass}
    };
}

inline void from_json(const nlohmann::json& j, CabSimulatorSettings& a) {
    a.model = j.value("model", a.model);
    a.enabled = j.value("enabled", a.enabled);
    a.hiPass = j.value("hiPass", a.hiPass);
    a.loPass = j.value("loPass", a.loPass);
}

class CabSimulator
{
public:
    CabSimulator();

    void process(const float* in, float* out, size_t frames);
    void loadIr(const std::string &irPath = "");
    void setHighPassFrequency(int frequency);
    void setLowPassFrequency(int frequency);
    void reload();
    inline void enabled(bool on) 
    { 
        _enabled = on; 
        _settings.enabled = on;
        Storage::get().persist<CabSimulatorSettings>(_settings, "cab");
    }

    inline const CabSimulatorSettings *cabSettings()
    {
        return &_settings;
    }

private:
    CabSimulatorSettings _settings;
    std::atomic<bool> _enabled{true};
    std::unique_ptr<fftconvolver::FFTConvolver> _conv;
    HiPassFilter _cabPostHiPassFilter;
    LoPassFilter _cabPostLoPassFilter;
};