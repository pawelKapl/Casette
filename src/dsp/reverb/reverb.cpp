#include "reverb.h"

Reverb::Reverb(int samplingFrequency)
  : _samplingFrequency(samplingFrequency)
{
  _roomReverb[0].right = true;
}

t_float *Reverb::init(t_float *memory)
{
  t_float *currentPtr = memory;

  currentPtr = _roomReverb[0].init(memory, _samplingFrequency);
  currentPtr = _roomReverb[1].init(memory, _samplingFrequency);

  return currentPtr;
}

void Reverb::process(const t_float* const* input, t_float** output, int frames, int inChannels, int outChannels)
{
  for (int c = 0; c < inChannels; c++)
  {
    for (int i = 0; i < frames; i++)
    {
      t_float inputSample = input[c][i];

      _hiPassFilter[c].filter(inputSample, &inputSample);
      _lowPassFilter[c].filter(inputSample, &inputSample);

      float out = _roomReverb[c].process(inputSample);
    
      output[c][i] += out * _reverbMix;

      normalize(&output[c][i]);
    }  
  }
}

void Reverb::setHighPass(int freq)
{
  _hiPassFilter[0].setCutOff(freq, _samplingFrequency);
  _hiPassFilter[1].setCutOff(freq, _samplingFrequency);
}

void Reverb::setLowPass(int freq)
{
  _lowPassFilter[0].setCutOff(freq, _samplingFrequency);
  _lowPassFilter[1].setCutOff(freq, _samplingFrequency);
}
