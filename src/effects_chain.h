#pragma once

#include "env.h"
#include "storage.h"
#include "dsp/delay/delay.h"
#include "dsp/delay/delay_settings.h"
#include "dsp/reverb/reverb.h"
#include "dsp/reverb/reverb_settings.h"
#include "dsp/mod/modulator.h"
#include "dsp/mod/modulator_settins.h"

static float DELAY_MEMORY[4194304]; // 16mb

class FxChain
{
public:
    FxChain();

    void process(const float *const *input, float **output, int frames, int inChannels, int outChannels);
    void reload();

    inline const DelaySettings *delaySettings()
    {
        return &_delaySettings;
    }

    inline const ReverbSettings *reverbSettings()
    {
        return &_reverbSettings;
    }

    inline const ModulatorSettings *modulatorSettings()
    {
        return &_modulatorSettings;
    }

    void adjustDelayParam(int param, int newValue);
    void adjustReverbParam(int param, int newValue);
    void adjustModParam(int param, int newValue);

    inline void enableDelay(bool on)
    {
        _delaySettings.enabled = on;
        _delay->clear();
        Storage::get().persist<DelaySettings>(_delaySettings, "delay");
    }

    inline void enableReverb(bool on)
    {
        _reverbSettings.enabled = on;
        Storage::get().persist<ReverbSettings>(_reverbSettings, "reverb");
    }

    inline void enableMod(bool on)
    {
        _modulatorSettings.enabled = on;
        Storage::get().persist<ModulatorSettings>(_modulatorSettings, "mod");
    }

private:
    std::unique_ptr<Delay> _delay;
    std::unique_ptr<Reverb> _reverb;
    std::unique_ptr<Modulator> _mod;
    DelaySettings _delaySettings;
    ReverbSettings _reverbSettings;
    ModulatorSettings _modulatorSettings;

    std::atomic<bool> _enabled{true};

}; // FxChain