#pragma once

#include "dsp_math.h"

class SmoothedValue
{
public:
  SmoothedValue() = default;
  SmoothedValue(t_float value, float seconds, int samplingFrequency)
      : _current(value),
        _target(value)
  {
    setTime(seconds, samplingFrequency);
  }

  // time to cover ~63% of a jump
  void setTime(float seconds, int samplingFrequency)
  {
    _coeff = 1.0f - expf(-1.0f / (std::max(seconds, 0.0001f) * samplingFrequency));
  }

  inline void setTarget(t_float target)
  {
    _target = target;
  }

  inline void reset(t_float value)
  {
    _current = value;
    _target = value;
  }

  inline void snap()
  {
    _current = _target;
  }

  inline t_float next()
  {
    t_float diff = _target - _current;
    t_float stepped = _current + _coeff * diff;

    // snap when close or when rounding stalls, otherwise it never lands
    _current = (std::abs(diff) < 1e-6f || stepped == _current) ? _target : stepped;

    return _current;
  }

  inline t_float current() const
  {
    return _current;
  }

private:
  t_float _current = 0.0f;
  t_float _target = 0.0f;
  t_float _coeff = 1.0f;
};
