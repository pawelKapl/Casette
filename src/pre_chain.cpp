#include "pre_chain.h"

PreChain::PreChain()
{
    _octaver = std::make_unique<Octaver>(48000);
    reload();
    _octaver->settle();
}

// keeps the same octaver running and lets it glide to the new preset, nothing gets torn down under the audio thread
void PreChain::reload()
{
    auto persistedOctaverSettings = Storage::get().read<OctaverSettings>("octaver");
    _octaverSettings = persistedOctaverSettings.has_value() ? *persistedOctaverSettings : OctaverSettings{};
    applyOctaverSettings(*_octaver, _octaverSettings);
}

void PreChain::adjustOctaverParam(int param, int newValue)
{
    *_octaverSettings.getParam(param) = newValue;
    applyOctaverParam(*_octaver, _octaverSettings, param);
    Storage::get().persist<OctaverSettings>(_octaverSettings, "octaver");
}

void PreChain::enableOctaver(bool on)
{
    adjustOctaverParam(7, on);
}

void PreChain::process(const float *input, float *output, int frames)
{
    _octaver->process(input, output, frames);
}
