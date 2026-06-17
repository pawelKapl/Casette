#pragma once

#include "dsp_math.h"
#include "lfo.h"
#include "algorithm"

#define BUFFER_OVERLAP 15

class AllPass
{
public:
  AllPass() = default;
  AllPass(t_float aFactor, int delayLength)
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
    int modDelay = std::clamp(_delay + offset, 1, _delay + BUFFER_OVERLAP - 1);

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
  void init(int samplingFrequency)
  {
    _filters[0] = std::make_unique<AllPass>(0.75, 179);
    _filters[1] = std::make_unique<AllPass>(0.72, 269);
    _filters[2] = std::make_unique<AllPass>(0.68, 419);
    _filters[3] = std::make_unique<AllPass>(0.64, 631);

    _lfo = std::make_unique<LFO>(0.1f, samplingFrequency, 4);
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
    processed = _filters[0]->process(processed, offset);
    processed = _filters[1]->process(processed, -offset);
    processed = _filters[2]->process(processed, offset / 2);
    processed = _filters[3]->process(processed, -offset / 2);

    output = processed;
  }

private:
  std::unique_ptr<AllPass> _filters[4];
  std::unique_ptr<LFO> _lfo;
};