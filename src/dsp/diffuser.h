#pragma once

#include "dsp_math.h"
#include "lfo.h"

#define BUFFER_OVERLAP 15

class AllPass
{
public:
  AllPass() = default;
  AllPass(t_float aFactor, int delayLength, int samplingFrequency, t_float *bufferMemory)
    : _a(aFactor),
    _delay(delayLength),
    _buffer(delayLength + BUFFER_OVERLAP)
  {
    for (int i = 0; i < _delay + BUFFER_OVERLAP; i++)
      _buffer[i] = 0;
  }

  inline t_float process(t_float input, int offset)
  {
    int bufferSize = _delay + BUFFER_OVERLAP;
    int modDelay = _delay + offset;

    _index = (_index + 1) % bufferSize;
    int readIndex = (_index - modDelay + bufferSize) % bufferSize;

    t_float output = -_a * input + _buffer[readIndex];
    _buffer[_index] = input + _a * output;

    return output;
  }

  void setA(t_float a)
  {
    _a = a;
  }

private:
  int _index = 0;
  t_float _a;
  int _delay;
  std::vector<t_float> _buffer;
};

class Diffuser
{
public:
  t_float* init(t_float *memory, int samplingFrequency)
  {
    t_float *memoryPtr = memory;
    _filters[0] = std::make_unique<AllPass>(0.7, 179, samplingFrequency, memoryPtr);
    memoryPtr += (179 + BUFFER_OVERLAP);
    _filters[1] = std::make_unique<AllPass>(0.7, 269, samplingFrequency, memoryPtr);
    memoryPtr += (269 + BUFFER_OVERLAP);
    _filters[2] = std::make_unique<AllPass>(0.7, 419, samplingFrequency, memoryPtr);
    memoryPtr += (419 + BUFFER_OVERLAP);
    _filters[3] = std::make_unique<AllPass>(0.7, 631, samplingFrequency, memoryPtr);
    memoryPtr += (631 + BUFFER_OVERLAP);

    _lfo = std::make_unique<LFO>(0.1f, samplingFrequency, 4);

    return memoryPtr;
  }

  void setGain(t_float diffusion)
  {
    for (auto& filter : _filters)
      filter->setA(diffusion);
  }

  inline void process(t_float input, t_float& output)
  {
    _lfo->tick();
    int offset = _lfo->getOffset();
    t_float processed = input;
    for (int i = 0; i < 4; i++)
      processed = _filters[i]->process(processed, offset);

    output = processed;
  }

private:
  std::unique_ptr<AllPass> _filters[4];
  std::unique_ptr<LFO> _lfo;
};