#pragma once

#include "dsp_math.h"

class LFO
{
public:
  LFO() = default;
  LFO(t_float freq, int samplingFrequency, t_float range)
      : _phase(0.0),
        _freq(freq),
        _range(range),
        _finalRange(range),
        _randomness(0),
        _samplingFrequency(samplingFrequency)
  {
    _increment = 2.0 * M_PI * freq / samplingFrequency;
  }

  LFO(t_float freq, int samplingFrequency, t_float range, t_float randomness)
      : _phase(0.0),
        _freq(freq),
        _range(range),
        _finalRange(range),
        _randomness(randomness),
        _samplingFrequency(samplingFrequency)
  {
    _increment = 2.0 * M_PI * freq / samplingFrequency;
  }

  inline void tick()
  {
    _phase += _increment;
    if (_phase >= M_PI)
    {
      _phase -= 2.0 * M_PI;
      if (_randomness)
      {
        _finalRange = (1.0 - _randomness) * _range + _randomness * _range * std::abs(randomFloat());
      }
    }
  }

  inline void setRangeMs(float rangeMs)
  {
    _range = _samplingFrequency * rangeMs / 1000.0f;
    _finalRange = _range;
  }

  inline void setRate(float rate)
  {
    _freq = rate;
    _increment = 2.0 * M_PI * _freq / _samplingFrequency;
  }

  inline void setPhaseOffset(float offset) 
  {
    _phase = offset * 2.0f * M_PI;
  }

  inline t_float getOffset() const
  {
    return fast_sin(_phase) * _finalRange;
  }

private:
  t_float _phase = 0;
  t_float _freq = 0;
  t_float _range = 0;
  t_float _finalRange = 0;
  t_float _randomness = 0;
  t_float _samplingFrequency = 48000;
  t_float _increment = 0;
};