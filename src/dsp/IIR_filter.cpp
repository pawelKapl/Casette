#include "IIR_filter.h"

void BaseIIRFilter::process(float *inputs, float *outputs, const size_t numFrames)
{
    for (size_t n = 0; n < numFrames; ++n)
    {
        float input = inputs[n];
        float output = 0.0f;

        // Input part: b0*x[n] + b1*x[n-1] + ... + bM*x[n-M]
        output += _inputCoefficients[0] * input;
        for (int i = 1; i < _inputCoefficientsSize; ++i)
        {
            output += _inputCoefficients[i] * _inputHistory[i - 1];
        }

        // Output part: -a1*y[n-1] - a2*y[n-2] - ... - aN*y[n-N]
        for (int j = 1; j < _outputCoefficientsSize; ++j)
        {
            output -= _outputCoefficients[j] * _outputHistory[j - 1];
        }

        outputs[n] = output;

        // Advance histories
        if (_inputCoefficientsSize > 1)
        {
            for (int i = _inputCoefficientsSize - 2; i > 0; --i)
                _inputHistory[i] = _inputHistory[i - 1];
            _inputHistory[0] = input;
        }

        if (_outputCoefficientsSize > 1)
        {
            for (int j = _outputCoefficientsSize - 2; j > 0; --j)
                _outputHistory[j] = _outputHistory[j - 1];
            _outputHistory[0] = output;
        }
    }
}

float BaseIIRFilter::processSample(float in)
{
    float input = in;
    float output = 0.0f;

    output += _inputCoefficients[0] * input;
    for (int i = 1; i < _inputCoefficientsSize; ++i)
    {
        output += _inputCoefficients[i] * _inputHistory[i - 1];
    }

    // Output part: -a1*y[n-1] - a2*y[n-2] - ... - aN*y[n-N]
    for (int j = 1; j < _outputCoefficientsSize; ++j)
    {
        output -= _outputCoefficients[j] * _outputHistory[j - 1];
    }

    // Advance histories
    if (_inputCoefficientsSize > 1)
    {
        for (int i = _inputCoefficientsSize - 1; i > 0; --i)
            _inputHistory[i] = _inputHistory[i - 1];
        _inputHistory[0] = input;
    }

    if (_outputCoefficientsSize > 1)
    {
        for (int j = _outputCoefficientsSize - 1; j > 0; --j)
            _outputHistory[j] = _outputHistory[j - 1];
        _outputHistory[0] = output;
    }

    return output;
}

void BiQuadFilter::setParams(const BiQuadFilterParams &params)
{
    _params = params;

    float A = std::pow(10.0f, params.gainDB / 40.0f);
    float omega = 2.0f * M_PI * params.frequency / params.sampleRate;
    float sn = std::sin(omega);
    float cs = std::cos(omega);
    float alpha = sn / (2.0f * params.quality);
    float beta = std::sqrt(A) / params.quality;

    float b0, b1, b2, a0, a1, a2;

    switch (params.type)
    {
    case LOW_SHELF:
    {
        float sqrtA = std::sqrt(A);
        b0 = A * ((A + 1) - (A - 1) * cs + 2 * sqrtA * alpha);
        b1 = 2 * A * ((A - 1) - (A + 1) * cs);
        b2 = A * ((A + 1) - (A - 1) * cs - 2 * sqrtA * alpha);
        a0 = (A + 1) + (A - 1) * cs + 2 * sqrtA * alpha;
        a1 = -2 * ((A - 1) + (A + 1) * cs);
        a2 = (A + 1) + (A - 1) * cs - 2 * sqrtA * alpha;
        break;
    }
    case PEAKING:
    {
        b0 = 1 + alpha * A;
        b1 = -2 * cs;
        b2 = 1 - alpha * A;
        a0 = 1 + alpha / A;
        a1 = -2 * cs;
        a2 = 1 - alpha / A;
        break;
    }
    case HIGH_SHELF:
    {
        float sqrtA = std::sqrt(A);
        b0 = A * ((A + 1) + (A - 1) * cs + 2 * sqrtA * alpha);
        b1 = -2 * A * ((A - 1) + (A + 1) * cs);
        b2 = A * ((A + 1) + (A - 1) * cs - 2 * sqrtA * alpha);
        a0 = (A + 1) - (A - 1) * cs + 2 * sqrtA * alpha;
        a1 = 2 * ((A - 1) - (A + 1) * cs);
        a2 = (A + 1) - (A - 1) * cs - 2 * sqrtA * alpha;
        break;
    }
    }

    _inputCoefficients[0] = b0 / a0;
    _inputCoefficients[1] = b1 / a0;
    _inputCoefficients[2] = b2 / a0;

    _outputCoefficients[0] = 1.0f; // a0 is normalized out
    _outputCoefficients[1] = a1 / a0;
    _outputCoefficients[2] = a2 / a0;
}

void BiQuadFilter::setGain(float gainDB)
{
    _params.gainDB = gainDB;
    setParams(_params);
}

void HiPassFilter::setParams(const HiPassFilterParams &params)
{
    _params = params;
    
    const float c = 2.0f * M_PI * params.frequency / params.sampleRate;
    const float alpha = 1.0f / (c + 1.0f);

    _inputCoefficients[0] = alpha;
    _inputCoefficients[1] = -alpha;

    _outputCoefficients[0] = 1.0f;
    _outputCoefficients[1] = -alpha;
}

void LoPassFilter::setParams(const LoPassFilterParams &params)
{
    _params = params;

    const float c = 2.0f * M_PI * params.frequency / params.sampleRate;
    const float alpha = 1.0f / (c + 1.0f);

    _inputCoefficients[0] = 1.0f - alpha;
    _inputCoefficients[1] = 0.0f;

    _outputCoefficients[0] = 1.0f;
    _outputCoefficients[1] = -alpha;
}