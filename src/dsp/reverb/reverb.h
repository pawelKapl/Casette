#pragma once

#include "dsp/dsp_math.h"
#include "dsp/highPassIOrder.h"
#include "dsp/lowPassIOrder.h"
#include "dsp/diffuser.h"
#include "dsp/comb_filter.h"
#include "dsp/lfo.h"

#define PRE_DELAY_BUFFER_SIZE 12000

enum ReverbMode
{
  ROOM = 0,
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
    setRoomSize(0.6, samplingFrequency);

    return currentPtr;
  }

  void setRoomSize(float size, int samplingFrequency)
  {
    combFilters.clear();
    auto feedback = size * 0.29f + 0.7f;

    auto shift = right ? 23 : 0;

    combFilters.push_back({feedback, 1116 + shift, samplingFrequency});
    combFilters.push_back({feedback, 1188 + shift, samplingFrequency});
    combFilters.push_back({feedback, 1277 + shift, samplingFrequency});
    combFilters.push_back({feedback, 1356 + shift, samplingFrequency});
    combFilters.push_back({feedback, 1422 + shift, samplingFrequency});
    combFilters.push_back({feedback, 1491 + shift, samplingFrequency});
    combFilters.push_back({feedback, 1557 + shift, samplingFrequency});
    combFilters.push_back({feedback, 1617 + shift, samplingFrequency});
  }

  void setPreDelay(float delay, int samplingFrequency)
  {
    preDelayLength = delay * samplingFrequency;
  }

  void setDamping(float damp)
  {
    for (int i = 0; i < 8; i++)
      combFilters[i].setDamp(damp);
  }

  float process(float_t in)
  {
    lfo.tick();

    t_float readHead = (t_float)writeHead - preDelayLength;
    readHead = fmod(readHead + PRE_DELAY_BUFFER_SIZE, PRE_DELAY_BUFFER_SIZE);

    t_float delayedSample = buffer[(int)readHead];
    buffer[(int)readHead] = 0;

    buffer[writeHead] = in;
    writeHead = (writeHead + 1) % PRE_DELAY_BUFFER_SIZE;

    float combSum = 0.0f;
    for (int i = 0; i < 8; i++)
      combSum += combFilters[i].process(delayedSample, lfo.getOffset());

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

  inline void setDamping(t_float damping)
  {
    _roomReverb[0].setDamping(damping);
    _roomReverb[1].setDamping(damping);
  }

  inline void setMix(t_float mix)
  {
    _reverbMix = mix;
  }

  inline void setMode(ReverbMode mode)
  {
    _mode = mode;
  }

  inline void setPreDelay(float preDelay)
  {
    _roomReverb[0].setPreDelay(preDelay, _samplingFrequency);
    _roomReverb[1].setPreDelay(preDelay, _samplingFrequency);
  }

  inline void setRoomSize(float size)
  {
    _roomReverb[0].setRoomSize(size, _samplingFrequency);
    _roomReverb[1].setRoomSize(size, _samplingFrequency);
  }

private:
  int _samplingFrequency = 48000;

  ReverbMode _mode = ROOM;
  t_float _reverbMix = 0.36f;

  RoomReverb _roomReverb[2];

  HighPassIOrder _hiPassFilter[2];
  LowPassIOrder _lowPassFilter[2];
};
