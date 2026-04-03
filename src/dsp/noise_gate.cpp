#include "noise_gate.h"

NoiseGate::NoiseGate()
{
    reload();
}

void NoiseGate::reload()
{
    auto persistedSettings = Storage::get().read<NoiseGateParams>("ng");
    if (persistedSettings.has_value())
    {
        _settings = *persistedSettings;
    }
    setParams(_settings);
}

void NoiseGate::setParams(const NoiseGateParams &settings)
{
    _settings = settings;

    _thresholdP = std::pow(10.0f, _settings.threshold / 10.0f);
    _attackCoeff = std::exp(-1.0f / (_settings.sampleRate * _settings.attack));
    _releaseCoeff = std::exp(-1.0f / (_settings.sampleRate * _settings.release));
}

void NoiseGate::setThresholdDB(float threshold)
{
    _settings.threshold = threshold;
    _thresholdP = std::pow(10.0f, _settings.threshold / 10.0f);
    Storage::get().persist<NoiseGateParams>(_settings, "ng");
}

void NoiseGate::process(const float *input, float *output, const size_t numFrames)
{
    if (_settings.enabled)
    {
        for (size_t i = 0; i < numFrames; ++i)
        {
            float in = input[i];
            float inPower = in * in;

            if (inPower > _envelope)
                _envelope = _attackCoeff * (_envelope - inPower) + inPower;
            else
                _envelope = _releaseCoeff * (_envelope - inPower) + inPower;


            if (_envelope > _thresholdP) // Check threshold
            {
                _gateOpen = true;
                _holdCounter = _settings.holdTime * _settings.sampleRate; // open wide now, above threshold, so resetting
            }
            else // went below threshold, lets hold for a bit if its not just a temp noise.
            {
                if (_holdCounter > 0)
                    _holdCounter -= 1;
                else
                    _gateOpen = false;
            }

            // up till now, only _gateOpen & current _envelope determination

            float targetGain = 1.0f; // gate opened
            if (!_gateOpen)
            {
                // Apply compression or full gate
                if (_settings.compRatio > 0.0f && _envelope < _thresholdP)
                {
                    float normalized = _envelope / _thresholdP; // 0..1
                    targetGain = std::pow(normalized, _settings.compRatio);
                }
                else
                {
                    targetGain = 0.0f;
                }
            }

            float smoothing = _gateOpen ? _attackCoeff : _releaseCoeff; // smoothing to not drop gain instantly (like stairs) - it would introduce pops, and unpleasant effects
            _gain = smoothing * _gain + (1.0f - smoothing) * targetGain;
            output[i] = in * _gain;
        }
    }
}