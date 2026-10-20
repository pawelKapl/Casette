#include "reverb.h"

Reverb::Reverb(int samplingFrequency)
  : _samplingFrequency(samplingFrequency)
{
  _roomReverb[0].right = true;
}

t_float *Reverb::init(t_float *memory)
{
  t_float *currentPtr = memory;

  currentPtr = _roomReverb[0].init(currentPtr, _samplingFrequency);
  currentPtr = _roomReverb[1].init(currentPtr, _samplingFrequency);

  return currentPtr;
}

void Reverb::process(const t_float* const* input, t_float** output, int frames, int inChannels, int outChannels)
{
  for (int c = 0; c < outChannels; c++)
  {
    for (int i = 0; i < frames; i++)
    {
      t_float drySample = input[c][i];
      t_float sendSample = drySample;

      _hiPassFilter[c].filter(sendSample, &sendSample);
      _lowPassFilter[c].filter(sendSample, &sendSample);

      float wet = _roomReverb[c].process(sendSample);

      float wetGain = std::sqrt(_reverbMix);
      float dryGain = std::sqrt(1.0f - _reverbMix);

      output[c][i] =
          drySample * dryGain +
          wet * wetGain;
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
