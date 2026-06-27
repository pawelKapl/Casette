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

    for (int i = 0; i < 8; i++)
      combFilters.push_back({});

    setCombFilters(0.5f, 0.5f, samplingFrequency);
    return currentPtr;
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

  inline void setCombFilters(float roomSize, float decay)
  {
    _roomReverb[0].setCombFilters(roomSize, decay, _samplingFrequency);
    _roomReverb[1].setCombFilters(roomSize, decay, _samplingFrequency);
  }

private:
  int _samplingFrequency = 48000;

  ReverbMode _mode = ROOM;
  t_float _reverbMix = 0.36f;

  RoomReverb _roomReverb[2];

  HighPassIOrder _hiPassFilter[2];
  LowPassIOrder _lowPassFilter[2];
};
