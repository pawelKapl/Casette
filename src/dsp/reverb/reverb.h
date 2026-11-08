#pragma once

#include <atomic>
#include "dsp/dsp_math.h"
#include "dsp/highPassIOrder.h"
#include "dsp/lowPassIOrder.h"
#include "dsp/diffuser.h"
#include "dsp/comb_filter.h"
#include "dsp/lfo.h"
#include "dsp/smoothed_value.h"
#include "dsp/reverb/spring_reverb.h"
#include "dsp/reverb/hall_reverb.h"

#define PRE_DELAY_BUFFER_SIZE 24000
#define MODE_FADE_TIME 0.01f
#define MIX_SMOOTHING_TIME 0.02f
#define COMB_GLIDE_TIME 0.05f
#define REVERB_BLOCK 64

// stored in presets as an int - only append
enum ReverbMode : int
{
  ROOM = 0,
  SPRING = 1,
  HALL = 2,
  REVERB_MODE_COUNT
};

struct RoomReverb
{
  Diffuser diffuser;
  LFO lfo;
  std::vector<Comb> combFilters;
  bool right = false;
  t_float *buffer;
  int preDelayLength = 0;
  int writeHead = 0;

  t_float *init(t_float *ptr, int samplingFrequency)
  {
    t_float *currentPtr = ptr;
    buffer = currentPtr;
    currentPtr += PRE_DELAY_BUFFER_SIZE;
    diffuser.init(samplingFrequency);

    diffuser.setGain(0.8f);
    lfo = {0.5, samplingFrequency, 1, 0.1};

    setPreDelay(0.02f, samplingFrequency);

    for (int i = 0; i < 8; i++)
    {
      combFilters.push_back({});
      combFilters.back().setDelayGlide(COMB_GLIDE_TIME, samplingFrequency);
    }

    setCombFilters(0.5f, 0.5f, samplingFrequency);
    settle();
    clear();
    return currentPtr;
  }

  void settle()
  {
    for (auto &comb : combFilters)
      comb.settle();
  }

  void clear()
  {
    std::fill(buffer, buffer + PRE_DELAY_BUFFER_SIZE, 0.0f);
    for (auto &comb : combFilters)
      comb.clear();
    diffuser.clear();
  }

  void setCombFilters(float roomSize, float decay, int samplingFrequency)
  {
    float sizeModifier = 0.5f + roomSize * 2;

    // RT60 od około 0.3 s do 12 s
    float rt60 = 0.3f + decay * 11.7f;

    int shift = right ? 23 : 0;

    const int baseLengths[8] =
        {
            1116,
            1188,
            1277,
            1356,
            1422,
            1491,
            1557,
            1617};

    for (int i = 0; i < 8; i++)
    {
      int delayLength =
          static_cast<int>(baseLengths[i] * sizeModifier) + shift;

      float delaySeconds =
          static_cast<float>(delayLength) /
          static_cast<float>(samplingFrequency);

      float feedback =
          powf(10.0f, -3.0f * delaySeconds / rt60);
      combFilters[i].setFeedback(feedback);
      combFilters[i].setDelay(delayLength);
    }
  }

  void setPreDelay(float delay, int samplingFrequency)
  {
    preDelayLength = std::clamp(int(delay * samplingFrequency), 0, PRE_DELAY_BUFFER_SIZE - 1);
  }

  void setDamping(float damp)
  {
    for (int i = 0; i < 8; i++)
      combFilters[i].setDamp(damp);
  }

  float process(float_t in)
  {
    lfo.tick();

    buffer[writeHead] = in;

    t_float readHead = (t_float)writeHead - preDelayLength;
    readHead = fmod(readHead + PRE_DELAY_BUFFER_SIZE, PRE_DELAY_BUFFER_SIZE);

    t_float delayedSample = buffer[(int)readHead];
    buffer[(int)readHead] = 0;

    writeHead = (writeHead + 1) % PRE_DELAY_BUFFER_SIZE;

    float combSum = 0.0f;
    for (int i = 0; i < 8; i++)
      combSum += combFilters[i].process(delayedSample, lfo.getOffset());
    combSum *=  0.125f;

    float out = 0;
    diffuser.process(combSum, out);

    return out;
  }
};

class Reverb
{
public:
  Reverb(int samplingFrequency);
  t_float *init(t_float *memory);
  void process(const t_float *const *input, t_float **output, int frames, int inChannels, int outChannels);
  void setHighPass(int freq);
  void setLowPass(int freq);
  void setMode(ReverbMode mode);
  void settle();

  inline void setDamping(t_float damping)
  {
    _roomReverb[0].setDamping(damping);
    _roomReverb[1].setDamping(damping);
    _spring.setDamping(damping);
    _hall.setDamping(damping);
  }

  inline void setMix(t_float mix)
  {
    _wetGain.setTarget(std::sqrt(mix));
    _dryGain.setTarget(std::sqrt(1.0f - mix));
  }

  inline void setPreDelay(float preDelay)
  {
    _roomReverb[0].setPreDelay(preDelay, _samplingFrequency);
    _roomReverb[1].setPreDelay(preDelay, _samplingFrequency);
    _spring.setPreDelay(preDelay);
    _hall.setPreDelay(preDelay);
  }

  inline void setSizeDecay(float size, float decay)
  {
    _roomReverb[0].setCombFilters(size, decay, _samplingFrequency);
    _roomReverb[1].setCombFilters(size, decay, _samplingFrequency);
    _spring.setSizeDecay(size, decay);
    _hall.setSizeDecay(size, decay);
  }

  inline void setDwell(float dwell)
  {
    _spring.setDwell(dwell);
  }

  inline void setTension(float tension)
  {
    _spring.setTension(tension);
  }

  inline void setSprings(int count)
  {
    _spring.setCount(count);
  }

  inline void setModDepth(float depth)
  {
    _hall.setModDepth(depth);
  }

  inline void setModRate(float rate)
  {
    _hall.setModRate(rate);
  }

  inline void setFreeze(bool on)
  {
    _hall.setFreeze(on);
  }

private:
  template <typename Tank>
  void processBlock(const t_float *const *input, t_float **output, int frames, int inChannels, int outChannels, t_float fadeTarget, Tank &&tank);
  void clearMode(ReverbMode mode);

  int _samplingFrequency = 48000;

  // UI requests a mode, audio thread fades out, switches, fades in
  std::atomic<ReverbMode> _requestedMode{ROOM};
  std::atomic<ReverbMode> _activeMode{ROOM};
  t_float _modeFade = 1.0f;
  t_float _modeFadeStep = 1.0f;

  SmoothedValue _wetGain;
  SmoothedValue _dryGain;

  RoomReverb _roomReverb[2];
  SpringReverb _spring;
  HallReverb _hall;

  HighPassIOrder _hiPassFilter[2];
  LowPassIOrder _lowPassFilter[2];
};

template <typename Tank>
void Reverb::processBlock(const t_float *const *input, t_float **output, int frames, int inChannels, int outChannels, t_float fadeTarget, Tank &&tank)
{
  const t_float *inL = input[0];
  const t_float *inR = inChannels > 1 ? input[1] : input[0];

  for (int start = 0; start < frames; start += REVERB_BLOCK)
  {
    int count = std::min(REVERB_BLOCK, frames - start);
    t_float sendL[REVERB_BLOCK];
    t_float sendR[REVERB_BLOCK];
    t_float wetL[REVERB_BLOCK];
    t_float wetR[REVERB_BLOCK];

    for (int i = 0; i < count; i++)
    {
      sendL[i] = inL[start + i];
      sendR[i] = inR[start + i];
      _hiPassFilter[0].filter(sendL[i], &sendL[i]);
      _lowPassFilter[0].filter(sendL[i], &sendL[i]);
      _hiPassFilter[1].filter(sendR[i], &sendR[i]);
      _lowPassFilter[1].filter(sendR[i], &sendR[i]);
    }

    tank(sendL, sendR, wetL, wetR, count);

    for (int i = 0; i < count; i++)
    {
      t_float dryL = inL[start + i];
      t_float dryR = inR[start + i];

      _modeFade += std::clamp(fadeTarget - _modeFade, -_modeFadeStep, _modeFadeStep);
      t_float wetGain = _wetGain.next() * _modeFade;
      t_float dryGain = _dryGain.next();

      output[0][start + i] = dryL * dryGain + wetL[i] * wetGain;
      if (outChannels > 1)
        output[1][start + i] = dryR * dryGain + wetR[i] * wetGain;
    }
  }
}
