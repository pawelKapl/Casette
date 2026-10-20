#include "dsp/mod/modulator.h"

Modulator::Modulator(int samplingFrequency, float maxDelayMs)
    : _samplingFrequency(samplingFrequency),
      _lfoL(0.3f, samplingFrequency, 330),
      _lfoR(0.3f, samplingFrequency, 335)
{
    int maxDelaySize = (maxDelayMs / 1000.0f) * samplingFrequency;
    _maxDelay = maxDelaySize;
    _delayBufferL.resize(maxDelaySize, 0.0f);
    _delayBufferR.resize(maxDelaySize, 0.0f);
    updateBaseDelay(330);
}

void Modulator::updateBaseDelay(float depthSamples)
{
    float minBase = 2.0f * _samplingFrequency / 1000.0f;
    float maxBase = (_maxDelay - 2) / 2.0f;
    _baseDelay = std::clamp(depthSamples * LFO_PEAK + 1.0f, minBase, maxBase);
}

void Modulator::process(const t_float *const *input, t_float **output, int frames, int inChannels, int outChannels)
{
    for (int i = 0; i < frames; i++)
    {
        // input and output may alias, so read the dry sample before writing any output
        const float in = input[0][i];

        _lfoL.tick();
        _lfoR.tick();

        auto lOffset = _lfoL.getOffset() + _baseDelay;
        auto rOffset = _lfoR.getOffset() + _baseDelay;

        float outL = readDelay(_delayBufferL, lOffset);
        float outR = readDelay(_delayBufferR, rOffset);

        _delayBufferL[_writeIndex] = in + _feedbackSampleL * _feedbackGain;
        _delayBufferR[_writeIndex] = in + _feedbackSampleR * _feedbackGain;
        _writeIndex = (_writeIndex + 1) % _maxDelay;

        if (_vibratoMode)
        {
            output[0][i] = outL;
            output[1][i] = outR;
        }
        else
        {
            float dryGain = std::sqrt(1.0f - _modMix);
            float wetGain = std::sqrt(_modMix);

            output[0][i] = dryGain * in + wetGain * outL;
            output[1][i] = dryGain * in + wetGain * outR;
        }

        _feedbackSampleL = outL;
        _feedbackSampleR = outR;
    }
}

float Modulator::readDelay(const std::vector<t_float> &buffer, t_float delaySamples)
{
    float readIndex = static_cast<float>(_writeIndex) - delaySamples;
    if (readIndex < 0)
        readIndex += _maxDelay;

    int indexA = static_cast<int>(readIndex);
    int indexB = (indexA + 1) % _maxDelay;
    float frac = readIndex - indexA;

    float a = buffer[indexA % _maxDelay];
    float b = buffer[indexB % _maxDelay];
    return a + frac * (b - a);
}