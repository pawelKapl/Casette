#pragma once

#include "dsp/highPassIOrder.h"
#include "dsp/lowPassIOrder.h"
#include "dsp/diffuser.h"
#include "dsp/dsp_math.h"

#define BUFFER_SIZE 144000

enum DelayMode
{
  MONO = 0,
  TAPE = 1,
  DUAL_TAP = 2,
  PING_PONG = 3,
  CIRCULAR = 4,
  REVERSE = 5
};

struct ReverseGrain
{
  int start = 0;
  int pos = 0;
  int length = 0;
  bool active = false;
};

struct SingleDelay
{
  HighPassIOrder hiPassFilter;
  LowPassIOrder lowPassFilter;
  LFO lfo;
  t_float *buffer;
  int writeHead = 0;
  int bufferLength = 0;
  t_float feedbackLevel = 0;
};

class Delay
{
public:
  Delay(int samplingFrequency);
  t_float *init(t_float *memory);
  void clear();
  void process(const t_float* const* input, t_float** output, int frames, int inChannels, int outChannels);
  void setHighPass(int freq);
  void setLowPass(int freq);

  inline void setFeedback(t_float feedback)
  {
    _feedbackLvl = feedback;
    recalculate();
  }

  inline void setMix(t_float mix)
  {
    _delayMix = mix;
  }

  inline void setMod(t_float depth)
  {
    _modDepth = (depth / 1000.) * _samplingFrequency;
    _circularDelays[0].lfo = {0.5, _samplingFrequency, _modDepth, 0.1};
    _circularDelays[1].lfo = {0.6, _samplingFrequency, _modDepth, 0.1};
    _circularDelays[2].lfo = {0.7, _samplingFrequency, _modDepth, 0.1};
    _dualTapDelays[0].lfo = {0.5, _samplingFrequency, _modDepth, 0.1};
    _dualTapDelays[1].lfo = {0.6, _samplingFrequency, _modDepth, 0.1};
    _generalDelay[0].lfo = {0.5, _samplingFrequency, _modDepth, 0.1};
  }

  inline void setReverseWetOnly(bool wetOnly)
  {
    _reverseWetOnly = wetOnly;
  }

  inline void setMode(DelayMode mode)
  {
    _mode = mode;
    clear();
  }

  inline void setTime(t_float time)
  {
    _baseTime = time;
    recalculate();
  }

  inline void setTime2(t_float time)
  {
    _baseTime2 = time;
    recalculate();
  }

  inline void setDiffusion(t_float diffusion)
  {
    _diffusion = diffusion;
    _diffuserL.setGain(diffusion);
    _diffuserR.setGain(diffusion);
    _diffuserC1.setGain(diffusion);
    _diffuserC2.setGain(diffusion);
  }

private:
  void singleDelay(const t_float* const* input, t_float** output, DelayMode mode, int frames, int delayIndex, int panning, int inChannels, int outChannels);
  void reverseDelay(const t_float* const* input, t_float** output, int frames, int inChannels, int outChannels);
  void startGrain(ReverseGrain &grain);
  void recalculate();
  t_float* initSingleDelay(SingleDelay *_delay, t_float * ptr);
  void clearSingleDelay(SingleDelay *_delay);

  int _samplingFrequency = 48000;

  DelayMode _mode = MONO;
  t_float _delayMix = 0.36f;
  t_float _feedbackLvl = 0.1f;
  t_float _modDepth = 0;
  t_float _baseTime = 0.292;
  t_float _baseTime2 = 0.592;
  bool _diffusion = false;
  bool _reverseWetOnly = false;

  SingleDelay _circularDelays[3];
  SingleDelay _dualTapDelays[2];
  SingleDelay _generalDelay[1];
  SingleDelay _reverseDelay;

  // two grains offset by half a length, Hann windows sum to unity
  ReverseGrain _grains[2];
  int _reversePhase = 0;
  int _reverseLength = 2;

  Diffuser _diffuserL;
  Diffuser _diffuserR;
  Diffuser _diffuserC1;
  Diffuser _diffuserC2;

  LFO _lfoWow;
  LFO _lfoFlutter;
};
