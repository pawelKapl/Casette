#include "amp.h"
#include "json.hpp"
#include <list>

Amp::Amp(CpuMonitor* cpuMonitor, CabSimulator *cab) : _cab(cab), _cpuMonitor(cpuMonitor) {}

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
    loadSubModel(_settings.subModel);

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
    _subEnabled = _settings.subAmpEnabled;
    _subBlend = _settings.subBlend;
    _subMode = _settings.subMode;
}

void Amp::loadModel(const std::string &name)
{
    bool currentlyEnabled = _enabled;
    _enabled = false;

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
}

void Amp::loadSubModel(const std::string &name)
{
    bool currentlyEnabled = _subEnabled;
    _subEnabled = false;

    const auto modelPath{std::string{"../models/"} + name + ".nam"};
    log_info << "Loading sub amp model from: " + modelPath;

    NeuralAudio::NeuralModelLoader loader;
    _sub_model = loader.CreateFromFile(modelPath);
    _subQualityDowngraded = false;

    log_info << "Sub amp loaded with model: " << _sub_model->GetLoadMode();

    _subInputCorrection = pow(10.0f, _sub_model->GetRecommendedInputDBAdjustment() / 20.0f);
    _subOutputCorrection = pow(10.0f, _sub_model->GetRecommendedOutputDBAdjustment() / 20.0f);

    log_info << "Sub Input Correction: " << _sub_model->GetRecommendedInputDBAdjustment() << "db, factor: " << _subInputCorrection;
    log_info << "Sub Output Correction: " << _sub_model->GetRecommendedOutputDBAdjustment() << "db, factor: " << _subOutputCorrection;

    _settings.subModel = name;

    _subEnabled = currentlyEnabled;

    Storage::get().persist<AmpSettings>(_settings, "amp");
}

void Amp::process(const float *input, float *output, const size_t numFrames)
{
    performanceScaling();

    float outMax = 0;
    float inMax = 0;
    float subOutput[numFrames];

    // apply input gain
    for (int i = 0; i < numFrames; i++)
    {
        subOutput[i] = input[i] * _inputGainAmp * _subInputCorrection;
        output[i] = input[i] * _inputGainAmp * _inputCorrection;
        if (std::abs(output[i]) > inMax)
            inMax = std::abs(output[i]);
    }

    _inputRMS = inMax;

    // preamp
    if (_enabled)
    {
        if (_subEnabled)
        {
            if (_subMode == "PARALLEL")
            {
                _model->Process(output, output, numFrames);
                _sub_model->Process(subOutput, subOutput, numFrames);
    
                for (int i = 0; i < numFrames; i++)
                    output[i] = (output[i] * _outputCorrection * (1.0f - _subBlend)) + (subOutput[i] * _subOutputCorrection * _subBlend);
            } 
            else if (_subMode == "SERIES")
            {
                _sub_model->Process(output, output, numFrames);
                for (int i = 0; i < numFrames; i++)
                    output[i] = output[i] * _subOutputCorrection;
                _model->Process(output, output, numFrames);
                for (int i = 0; i < numFrames; i++)
                    output[i] = output[i] * _outputCorrection;
            }
        }
        else
        {
            _model->Process(output, output, numFrames);
            for (int i = 0; i < numFrames; i++)
                output[i] = output[i] * _outputCorrection;
        }

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
        output[i] = output[i] * _mvGainAmp;
        if (std::abs(output[i]) > outMax)
            outMax = std::abs(output[i]);
    }

    _outputRMS = outMax;
}

void Amp::performanceScaling()
{
    static int counter = 0;
    if (!_subQualityDowngraded && _subEnabled && _sub_model->HasQualityScaling() && _sub_model->IsQualityChangeRealtimeSafe(0.5f))
    {
        float cpuUsage = _cpuMonitor->getCpuState()[0];
        if (int(cpuUsage) > 95)
        {
            counter++;
        }
        else
        {
            counter = 0;
        }

        if (counter > 500)
        {
            counter = 0;
            _subQualityDowngraded = true;
            _sub_model->SetQualityScaleFactor(0.5f);
            log_info << "Reducing Quality for Sub Model due to high CPU usage.";
        }
    }
    else 
    {
        counter = 0;
    }
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

void Amp::setSubBlend(float blend)
{
    _settings.subBlend = blend;
    _subBlend = blend;
    Storage::get().persist<AmpSettings>(_settings, "amp");
}

void Amp::setSubMode(std::string mode)
{
    _settings.subMode = mode;
    _subMode = mode;
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