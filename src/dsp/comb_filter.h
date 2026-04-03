#pragma once

#include "dsp_math.h"

#define BUFFER_OVERLAP 15

class Comb
{
public:
  Comb() = default;

  Comb(t_float feedback, int delayLength, int samplingFrequency)
    : _feedback(feedback),
      _delay(delayLength),
      _buffer(delayLength + BUFFER_OVERLAP)
  {
    for (int i = 0; i < _delay + BUFFER_OVERLAP; ++i)
      _buffer[i] = 0;
  }

  inline t_float process(t_float input, int offset)
  {
    int bufferSize = _delay + BUFFER_OVERLAP;
    int modDelay = _delay + offset;

    _index = (_index + 1) % bufferSize;
    int readIndex = (_index - modDelay + bufferSize) % bufferSize;

    t_float delayed = _buffer[readIndex];

    _damped = (1.0f - _damp) * delayed + _damp * _damped;

    t_float output = _damped;

    _buffer[_index] = input + _damped * _feedback;

    return output;
  }

  void setFeedback(t_float fb)
  {
    _feedback = fb;
  }

  void setDamp(t_float damp)
  {
    _damp = damp;
  }

private:
  int _index = 0;
  t_float _feedback;
  t_float _damp = 0.0f;
  t_float _damped = 0.0f;
  int _delay;
  std::vector<t_float> _buffer;
};

