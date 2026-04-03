#include "reverb.h"

Reverb::Reverb(int samplingFrequency)
  : _samplingFrequency(samplingFrequency)
{
  _roomReverbR.right = true;
}

t_float *Reverb::init(t_float *memory)
{
  t_float *currentPtr = memory;

  currentPtr = _roomReverbL.init(memory, _samplingFrequency);
  currentPtr = _roomReverbR.init(memory, _samplingFrequency);

  return currentPtr;
}

void Reverb::process(const t_float* const* input, t_float** output, int frames, int inChannels, int outChannels)
{
  for (int i = 0; i < frames; i++)
  {
    t_float inputSample = 0;
    if (inChannels == 2)
      inputSample = (input[0][i] * 0.5f) + (input[1][i] * 0.5f);
    else
      inputSample = input[0][i];

    _hiPassFilter.filter(inputSample, &inputSample);
    _lowPassFilter.filter(inputSample, &inputSample);

    float outL = _roomReverbL.process(inputSample);
    float outR = _roomReverbR.process(inputSample);
  
    output[0][i] += outL * _reverbMix;
    output[1][i] += outR * _reverbMix;

    normalize(&output[0][i]);
    normalize(&output[1][i]);
  }
}

void Reverb::setHighPass(int freq)
{
  _hiPassFilter.setCutOff(freq, _samplingFrequency);
}

void Reverb::setLowPass(int freq)
{
  _lowPassFilter.setCutOff(freq, _samplingFrequency);
}
