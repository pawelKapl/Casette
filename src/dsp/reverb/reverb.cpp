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
      t_float inputSample = input[c][i];

      _hiPassFilter[c].filter(inputSample, &inputSample);
      _lowPassFilter[c].filter(inputSample, &inputSample);

      float wet = _roomReverb[c].process(inputSample);

      float wetGain = std::sqrt(_reverbMix);
      float dryGain = std::sqrt(1.0f - _reverbMix);

      output[c][i] =
          inputSample * dryGain +
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
