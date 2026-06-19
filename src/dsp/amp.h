#pragma once

#include "env.h"
#include "dsp/IIR_filter.h"
#include "cab_simulator.h"
#include "storage.h"
#include "NeuralModel.h"

struct AmpSettings
{
    std::string model = "Bogner XTC Red Channel";
    std::string subModel = "Bogner XTC Red Channel";
    float subBlend = 0.0f;
    std::string subMode = "PARALLEL";
    float inputGain = 0.0f;
    float masterVolumeGain = 0.0f;
    float bass = 0.0f;
    float middle = 0.0f;
    float treble = 0.0f;
    bool tonestackEnabled = true;
    bool ampEnabled = true;
    bool subAmpEnabled = false;

    float getBassValue() const
    {
        return (bass / 4.0f) + 5.0f;
    }
    
    float getMiddleValue() const
    {
        return (middle / 3.0f) + 5.0f;
    }

    float getTrebleValue() const
    {
        return (treble / 2.0f) + 5.0f;
    }

    void setBassValue(float value)
    {
        auto gainDB = 4.0f * (value - 5.0f);
        this->bass = gainDB;
    }

    void setMiddleValue(float value)
    {
        auto gainDB = 3.0f * (value - 5.0f);
        this->middle = gainDB;
    }
    
    void setTrebleValue(float value)
    {
        auto gainDB = 2.0f * (value - 5.0f);
        this->treble = gainDB;
    }
};

inline void to_json(nlohmann::json& j, const AmpSettings& a) {
    j = nlohmann::json{
        {"model", a.model},
        {"subModel", a.subModel},
        {"subMode", a.subMode},
        {"subBlend", a.subBlend},
        {"inputGain", a.inputGain},
        {"masterVolumeGain", a.masterVolumeGain},
        {"bass", a.bass},
        {"middle", a.middle},
        {"treble", a.treble},
        {"tonestackEnabled", a.tonestackEnabled},
        {"ampEnabled", a.ampEnabled},
        {"subAmpEnabled", a.subAmpEnabled}
    };
}

inline void from_json(const nlohmann::json& j, AmpSettings& a) {
    j.at("model").get_to(a.model);
    a.subModel = j.value("subModel", "Bogner XTC Red Channel");
    a.subBlend = j.value("subBlend", 0.0f);
    a.subMode = j.value("subMode", "PARALLEL");
    j.at("inputGain").get_to(a.inputGain);
    j.at("masterVolumeGain").get_to(a.masterVolumeGain);
    j.at("bass").get_to(a.bass);
    j.at("middle").get_to(a.middle);
    j.at("treble").get_to(a.treble);
    j.at("tonestackEnabled").get_to(a.tonestackEnabled);
    j.at("ampEnabled").get_to(a.ampEnabled);
    a.subAmpEnabled = j.value("subAmpEnabled", false);
}

class Amp
{
public:
    Amp(CabSimulator *cab = nullptr);
    ~Amp();
    void loadModel(const std::string &path);
    void loadSubModel(const std::string &path);
    void process(const float *input, float *output, const size_t numFrames);
    void reload();

    inline void enabled(bool on) 
    { 
        _enabled = on; 
        _settings.ampEnabled = on;
        Storage::get().persist<AmpSettings>(_settings, "amp");
    }

    inline void subEnabled(bool on) 
    { 
        _subEnabled = on;
        _settings.subAmpEnabled = on;
        Storage::get().persist<AmpSettings>(_settings, "amp");
    }

    inline void tonestackEnabled(bool on) 
    { 
        _settings.tonestackEnabled = on; 
        Storage::get().persist<AmpSettings>(_settings, "amp");
    }

    void setMVGain(float gainDb);
    void setInputGain(float gainDb);
    void setSubBlend(float blend);
    void setSubMode(std::string mode);
    void setBass(float value);
    void setMiddle(float value);
    void setTreble(float value);
    float getInputRMS() const { return _inputRMS; }
    float getOutputRMS() const { return _outputRMS; }

    inline const AmpSettings *ampSettings()
    {
        return &_settings;
    }

private:
    std::atomic<bool> _enabled{false};
    std::atomic<bool> _subEnabled{false};
    NeuralAudio::NeuralModel *_model = nullptr;
    NeuralAudio::NeuralModel *_sub_model = nullptr;
    CabSimulator *_cab = nullptr;
    
    float _inputCorrection;
    float _subInputCorrection;
    float _outputCorrection;
    float _subOutputCorrection;
    float _mvGainAmp;
    float _inputGainAmp;
    float _subBlend;
    std::string _subMode = "PARALLEL";

    AmpSettings _settings;

    float _inputRMS = 0;
    float _outputRMS = 0;

    // Tonestack
    BiQuadFilter _bass;
    BiQuadFilter _middle;
    BiQuadFilter _treble;
};