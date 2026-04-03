#pragma once

#include "env.h"
#include "dsp/dsp_math.h"
#include "dsp/lfo.h"

class Modulator
{
public:
  Modulator(int samplingFrequency, float maxDelayMs = 30.0f);
  void process(const t_float *const *input, t_float **output, int frames, int inChannels, int outChannels);

  inline void setMix(t_float mix)
  {
    _modMix = mix;
  }

  inline void setDepth(t_float depth)
  {
    _lfoL.setRangeMs(depth);
    _lfoR.setRangeMs(depth);
  }

  inline void setRate(t_float rate)
  {
    _lfoL.setRate(rate);
    _lfoR.setRate(rate);
  }

  inline void setWidth(float offset) {
    _lfoL.setPhaseOffset(0);
    _lfoR.setPhaseOffset(offset);
  }

  inline void setFeedback(float fb) 
  {
    _feedbackGain = std::clamp(fb, -0.95f, 0.95f);
  }

  inline void setVibratoMode(bool enabled)
  {
    _vibratoMode = enabled;
  }

private:
  int _samplingFrequency = 48000;

  t_float _modMix = 0.4f;

  std::vector<t_float> _delayBufferL;
  std::vector<t_float> _delayBufferR;
  int _writeIndex = 0;
  int _maxDelay = 0;

  float _feedbackGain = 0.0f;
  float _feedbackSampleL = 0.0f;
  float _feedbackSampleR = 0.0f;

  bool _vibratoMode = false;

  LFO _lfoL;
  LFO _lfoR;

  float readDelay(const std::vector<t_float> &buffer, t_float delaySamples);

};
