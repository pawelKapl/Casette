#pragma once

#include <cstdint>
#include "dsp_math.h"

class RandomLFO
{
public:
  RandomLFO() = default;
  RandomLFO(t_float rate, int samplingFrequency, uint32_t seed)
      : _samplingFrequency(samplingFrequency),
        _state(seed ? seed : 1)
  {
    setRate(rate);
    _from = nextRandom();
    _to = nextRandom();
  }

  inline void setRate(t_float rate)
  {
    _increment = rate / _samplingFrequency;
  }

  inline t_float tick()
  {
    _phase += _increment;
    if (_phase >= 1.0f)
    {
      _phase -= 1.0f;
      _from = _to;
      _to = nextRandom();
    }

    t_float shape = _phase * _phase * (3.0f - 2.0f * _phase);
    return _from + (_to - _from) * shape;
  }

private:
  // xorshift32
  inline t_float nextRandom()
  {
    _state ^= _state << 13;
    _state ^= _state >> 17;
    _state ^= _state << 5;
    return (t_float)_state / (t_float)UINT32_MAX * 2.0f - 1.0f;
  }

  t_float _samplingFrequency = 48000;
  t_float _increment = 0;
  t_float _phase = 0;
  t_float _from = 0;
  t_float _to = 0;
  uint32_t _state = 1;
};
