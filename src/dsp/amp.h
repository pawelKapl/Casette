#pragma once

#include "env.h"
#include "wavenet/wave_net.h"
#include "dsp/IIR_filter.h"
#include "cab_simulator.h"
#include "storage.h"

using IStdDilations = NeuralAudio::Dilations<1, 2, 4, 8, 16, 32, 64, 128, 256, 512>;
using WaveNetNAMModel = typename NeuralAudio::WaveNetModelT<
    NeuralAudio::WaveNetLayerArrayT<1, 1, 8, 16, 3, IStdDilations, false>,
    NeuralAudio::WaveNetLayerArrayT<16, 1, 1, 8, 3, IStdDilations, true>>;

struct AmpSettings
{
    std::string model = "Bogner XTC Red Channel";
    float inputGain = 0.0f;
    float masterVolumeGain = 0.0f;
    float bass = 0.0f;
    float middle = 0.0f;
    float treble = 0.0f;
    bool tonestackEnabled = true;
    bool ampEnabled = true;

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
        {"inputGain", a.inputGain},
        {"masterVolumeGain", a.masterVolumeGain},
        {"bass", a.bass},
        {"middle", a.middle},
        {"treble", a.treble},
        {"tonestackEnabled", a.tonestackEnabled},
        {"ampEnabled", a.ampEnabled}
    };
}

inline void from_json(const nlohmann::json& j, AmpSettings& a) {
    j.at("model").get_to(a.model);
    j.at("inputGain").get_to(a.inputGain);
    j.at("masterVolumeGain").get_to(a.masterVolumeGain);
    j.at("bass").get_to(a.bass);
    j.at("middle").get_to(a.middle);
    j.at("treble").get_to(a.treble);
    j.at("tonestackEnabled").get_to(a.tonestackEnabled);
    j.at("ampEnabled").get_to(a.ampEnabled);
}

class Amp
{
public:
    Amp(CabSimulator *cab = nullptr);
    ~Amp();
    void loadModel(const std::string &path);
    void process(const float *input, float *output, const size_t numFrames);
    void reload();
    inline void enabled(bool on) 
    { 
        _enabled = on; 
        _settings.ampEnabled = on;
        Storage::get().persist<AmpSettings>(_settings, "amp");
    }
    inline void tonestackEnabled(bool on) 
    { 
        _settings.tonestackEnabled = on; 
        Storage::get().persist<AmpSettings>(_settings, "amp");
    }
    void setMVGain(float gainDb);
    void setInputGain(float gainDb);
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
    std::atomic<bool> _enabled{true};
    float _modelLoudnessDb = -18.0f;
    WaveNetNAMModel *_model = nullptr;
    CabSimulator *_cab = nullptr;

    float _modelLoudnessAmp;
    float _mvGainAmp;
    float _inputGainAmp;

    AmpSettings _settings;

    float _inputRMS = 0;
    float _outputRMS = 0;

    // Tonestack
    BiQuadFilter _bass;
    BiQuadFilter _middle;
    BiQuadFilter _treble;
};