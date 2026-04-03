#include "dsp/cab_simulator.h"

CabSimulator::CabSimulator()
{
    reload();
}

void CabSimulator::reload()
{
    auto persistedSettings = Storage::get().read<CabSimulatorSettings>("cab");

    if (persistedSettings.has_value())
    {
        _settings = *persistedSettings;
    }

    HiPassFilterParams cabFilterParams;
    cabFilterParams.sampleRate = 48000;
    cabFilterParams.frequency = _settings.hiPass;

    _cabPostFilter.setParams(cabFilterParams);


    _enabled = _settings.enabled;
    loadIr(_settings.model);
}

void CabSimulator::process(const float* in, float* out, size_t frames)
{
    if (_enabled)
    {
        _conv->process(in, out, frames);
        for (int i = 0; i < frames; i++)
            out[i] = _cabPostFilter.processSample(out[i]);
    }
}

void CabSimulator::loadIr(const std::string &name)
{
    bool currentlyEnabled = _enabled;
    _enabled = false;
    
    const auto irPath { std::string { "../irs/" } + name + ".wav" };
    auto wav = Wav{irPath};
    _conv = std::make_unique<fftconvolver::FFTConvolver>();
    _conv->init(64, wav.data(), wav.size());

    _enabled = currentlyEnabled;
    _settings.model = name;
    Storage::get().persist<CabSimulatorSettings>(_settings, "cab");
    log_info << "IR loaded";
}

void CabSimulator::setHighPassFrequency(int frequency)
{
    _settings.hiPass = frequency;

    HiPassFilterParams cabFilterParams;
    cabFilterParams.sampleRate = 48000;
    cabFilterParams.frequency = _settings.hiPass;

    _cabPostFilter.setParams(cabFilterParams);
    Storage::get().persist<CabSimulatorSettings>(_settings, "cab");
}