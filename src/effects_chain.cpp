#include "effects_chain.h"

FxChain::FxChain()
{
	reload();
}

void FxChain::reload()
{

	auto persistedDelaySettings = Storage::get().read<DelaySettings>("delay");
    if (persistedDelaySettings.has_value())
    {
        _delaySettings = *persistedDelaySettings;
    }

	auto persistedReverbSettings = Storage::get().read<ReverbSettings>("reverb");
    if (persistedReverbSettings.has_value())
    {
        _reverbSettings = *persistedReverbSettings;
    }

	auto persistedModSettings = Storage::get().read<ModulatorSettings>("mod");
    if (persistedModSettings.has_value())
    {
        _modulatorSettings = *persistedModSettings;
    }

	float_t *memPtr = DELAY_MEMORY;

	bool enabledCurrently = _enabled;
    _enabled = false;

	_mod = std::make_unique<Modulator>(48000);
	_mod->setMix(float(_modulatorSettings.mix) / 100.);
	_mod->setDepth(float(_modulatorSettings.depth) / 10.);
	_mod->setRate(float(_modulatorSettings.rate) / 1000.);
	_mod->setWidth(float(_modulatorSettings.width) / 100.);
	_mod->setFeedback(float(_modulatorSettings.feedback) / 100.);
	_mod->setVibratoMode(_modulatorSettings.vibratoMode);

    _delay = std::make_unique<class Delay>(48000);
	memPtr = _delay->init(DELAY_MEMORY);
	_delay->setMix(float(_delaySettings.mix) / 100.);
	_delay->setFeedback(float(_delaySettings.feedback) / 100.);
	_delay->setTime(float(_delaySettings.time) / 1000.);
	_delay->setTime2(float(_delaySettings.time2) / 1000.);
	_delay->setMode(DelayMode(_delaySettings.delayMode));
	_delay->setHighPass(_delaySettings.hiPass);
	_delay->setLowPass(_delaySettings.loPass);
	_delay->setMod(_delaySettings.diffusion ? 1 : 0);
	_delay->setDiffusion(float(_delaySettings.diffusion) / 100.);   

	_reverb = std::make_unique<Reverb>(48000);
	_reverb->init(memPtr);
	_reverb->setMix(float(_reverbSettings.mix) / 100.);
	_reverb->setRoomSize(float(_reverbSettings.roomSize) / 100.);
	_reverb->setHighPass(_reverbSettings.hiPass);
	_reverb->setDamping(float(_reverbSettings.damping) / 100.);
	_reverb->setLowPass(_reverbSettings.loPass);

	_enabled = enabledCurrently;
}

void FxChain::adjustDelayParam(int param, int newValue)
{
	*_delaySettings.getParam(param) = newValue;

	switch (param)
	{	
		case 0:
			_delay->setMode((DelayMode)_delaySettings.delayMode);
		case 1:
			_delay->setMix(float(_delaySettings.mix) / 100.);
			break;
		case 2:
			_delay->setFeedback(float(_delaySettings.feedback) / 100.);
			break;
		case 3:
			_delay->setTime(float(_delaySettings.time) / 1000.);
			break;
		case 4:
			_delay->setTime2(float(_delaySettings.time2) / 1000.);
			break;
		case 5:
			_delay->setHighPass(_delaySettings.hiPass);
			break;
		case 6:
			_delay->setLowPass(_delaySettings.loPass);
			break;
		case 7:
			_delay->setDiffusion(float(_delaySettings.diffusion) / 100.);
			break;
		case 8:
			_delay->setMod(_delaySettings.modRate);
			break;
	}

	Storage::get().persist<DelaySettings>(_delaySettings, "delay");
}

void FxChain::adjustReverbParam(int param, int newValue)
{
	*_reverbSettings.getParam(param) = newValue;

	switch (param)
	{	
		case 0:
			// mode
			break;
		case 1:
			_reverb->setMix(float(_reverbSettings.mix) / 100.);
			break;
		case 2:
			_reverb->setRoomSize(float(_reverbSettings.roomSize) / 100.);
			break;
		case 3:
			_reverb->setDamping(float(_reverbSettings.damping) / 100.);
			break;
		case 4:
			_reverb->setPreDelay(float(_reverbSettings.preDelay) / 1000.);
			break;
		case 5:
			_reverb->setHighPass(_reverbSettings.hiPass);
			break;
		case 6:
			_reverb->setLowPass(_reverbSettings.loPass);
			break;
	}

	Storage::get().persist<ReverbSettings>(_reverbSettings, "reverb");
}

void FxChain::adjustModParam(int param, int newValue)
{
	*_modulatorSettings.getParam(param) = newValue;

	switch (param)
	{	
		case 1:
			_mod->setMix(float(_modulatorSettings.mix) / 100.);
			break;
		case 2:
			_mod->setDepth(float(_modulatorSettings.depth) / 10.);
			break;
		case 3:
			_mod->setRate(float(_modulatorSettings.rate) / 1000.);
			break;
		case 4:
			_mod->setWidth(float(_modulatorSettings.width) / 100.);
			break;
		case 5:
			_mod->setFeedback(float(_modulatorSettings.feedback) / 100.);
			break;
		case 6:
			_mod->setVibratoMode(_modulatorSettings.vibratoMode);
			break;
	}

	Storage::get().persist<ModulatorSettings>(_modulatorSettings, "mod");
}

void FxChain::process(const float* const* input, float** output, int frames, int inChannels, int outChannels)
{
	if (_enabled)
	{
		if (_modulatorSettings.enabled)
		{
			_mod->process(input, output, frames, inChannels, outChannels);
		}

		if (_delaySettings.enabled)
		{
			auto inPtr = _modulatorSettings.enabled ? output : input;
			_delay->process(inPtr, output, frames, inChannels, outChannels);
		}
	
		if (_reverbSettings.enabled)
		{
			auto inPtr = (_delaySettings.enabled || _modulatorSettings.enabled) ? output : input;
			_reverb->process(inPtr, output, frames, inChannels, outChannels);
		}
	}
}