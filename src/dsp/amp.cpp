#include "amp.h"
#include "json.hpp"
#include <list>

Amp::Amp(CabSimulator *cab) : _cab(cab) {}

Amp::~Amp()
{
    delete _model;
}

void Amp::reload()
{
    _enabled = false;

    if (_model == nullptr)
        _model = new NeuralAudio::NeuralModel;

    auto persistedSettings = Storage::get().read<AmpSettings>("amp");
    if (persistedSettings.has_value())
    {
        _settings = *persistedSettings;
    }

    loadModel(_settings.model);

    // Tonestack
    BiQuadFilterParams bassParams{};
    bassParams.type = LOW_SHELF;
    bassParams.frequency = 150;
    bassParams.sampleRate = 48000;
    bassParams.quality = 0.707;
    bassParams.gainDB = _settings.bass;

    _bass.setParams(bassParams);

    BiQuadFilterParams midParams{};
    midParams.type = PEAKING;
    midParams.frequency = 425;
    midParams.sampleRate = 48000;
    midParams.quality = 0.707;
    midParams.gainDB = _settings.middle;

    _middle.setParams(midParams);

    BiQuadFilterParams trebleParams{};
    trebleParams.type = HIGH_SHELF;
    trebleParams.frequency = 1800;
    trebleParams.sampleRate = 48000;
    trebleParams.quality = 0.707;
    trebleParams.gainDB = _settings.treble;

    _treble.setParams(trebleParams);

    _mvGainAmp = pow(10.0f, _settings.masterVolumeGain / 20.0f);
    _inputGainAmp = pow(10.0f, _settings.inputGain / 20.0f);

    _enabled = _settings.ampEnabled;
}

void Amp::loadModel(const std::string &name)
{
    bool currentlyEnabled = _enabled;
    _enabled = false;

    if (_model == nullptr)
        _model = new NeuralAudio::NeuralModel;

    const auto modelPath{std::string{"../models/"} + name + ".nam"};
    log_info << "Loading amp model from: " + modelPath; 

    NeuralAudio::NeuralModelLoader loader;
    _model = loader.CreateFromFile(modelPath);

    log_info << "Amp loaded with model: " << _model->GetLoadMode();

    _inputCorrection = pow(10.0f, _model->GetRecommendedInputDBAdjustment() / 20.0f);
    _outputCorrection = pow(10.0f, _model->GetRecommendedOutputDBAdjustment() / 20.0f);

    log_info << "Input Correction: " << _model->GetRecommendedInputDBAdjustment() << "db, factor: " << _inputCorrection;
    log_info << "Output Correction: " << _model->GetRecommendedOutputDBAdjustment() << "db, factor: " << _outputCorrection;

    _settings.model = name;

    _enabled = currentlyEnabled;

    Storage::get().persist<AmpSettings>(_settings, "amp");
    log_info << "Amp loaded";
}

void Amp::process(const float *input, float *output, const size_t numFrames)
{
    float outMax = 0;
    float inMax = 0;
    
    // apply input gain
    for (int i = 0; i < numFrames; i++)
    {
        output[i] = input[i] * _inputGainAmp * _inputCorrection;
        if (std::abs(output[i]) > inMax)
            inMax = std::abs(output[i]);
    }

    _inputRMS = inMax;

    // preamp
    if (_enabled)
    {
        _model->Process(output, output, numFrames);

        if (_settings.tonestackEnabled)
        {
            for (int i = 0; i < numFrames; i++)
            {
                float out = _bass.processSample(output[i]);
                out = _middle.processSample(out);
                out = _treble.processSample(out);
                output[i] = out;
            }
        }
    }

    // cab ir
    _cab->process(output, output, numFrames);

    // apply output gain
    for (int i = 0; i < numFrames; i++)
    {
        output[i] = output[i] * _outputCorrection * _mvGainAmp;
        if (std::abs(output[i]) > outMax)
            outMax = std::abs(output[i]);
    }
        
    _outputRMS = outMax;
}

void Amp::setMVGain(float gainDB)
{
    _settings.masterVolumeGain = gainDB;
    _mvGainAmp = pow(10.0f, _settings.masterVolumeGain / 20.0f);
    Storage::get().persist<AmpSettings>(_settings, "amp");
}

void Amp::setInputGain(float gainDB)
{
    _settings.inputGain = gainDB;
    _inputGainAmp = pow(10.0f, _settings.inputGain / 20.0f);
    Storage::get().persist<AmpSettings>(_settings, "amp");
}

void Amp::setBass(float value)
{
    _settings.setBassValue(value);
    _bass.setGain(_settings.bass);
    Storage::get().persist<AmpSettings>(_settings, "amp");
}

void Amp::setMiddle(float value)
{
    _settings.setMiddleValue(value);
    _middle.setGain(_settings.middle);
    Storage::get().persist<AmpSettings>(_settings, "amp");
}

void Amp::setTreble(float value)
{
    _settings.setTrebleValue(value);
    _treble.setGain(_settings.treble);
    Storage::get().persist<AmpSettings>(_settings, "amp");
}