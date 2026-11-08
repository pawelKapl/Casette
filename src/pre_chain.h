#pragma once

#include "env.h"
#include "storage.h"
#include "dsp/octaver/octaver.h"
#include "dsp/octaver/octaver_settings.h"

// effects in front of the amp
class PreChain
{
public:
    PreChain();

    void process(const float *input, float *output, int frames);
    void reload();

    inline bool isActive() const
    {
        return _octaver->running();
    }

    inline const OctaverSettings *octaverSettings()
    {
        return &_octaverSettings;
    }

    void adjustOctaverParam(int param, int newValue);
    void enableOctaver(bool on);

private:
    std::unique_ptr<Octaver> _octaver;
    OctaverSettings _octaverSettings;
};
