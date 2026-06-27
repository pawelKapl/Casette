#pragma once

#include "dsp_math.h"

#define BUFFER_OVERLAP 15
#define MAX_BUFFER_SIZE 5000

class Comb
{
public:
  Comb()
  {
    for (int i = 0; i < MAX_BUFFER_SIZE; ++i)
      _buffer[i] = 0;
  }

  Comb(t_float feedback, int delayLength, int samplingFrequency)
      : _feedback(feedback),
        _delay(delayLength),
        _maxDelay(delayLength + BUFFER_OVERLAP)
  {
    for (int i = 0; i < MAX_BUFFER_SIZE; ++i)
      _buffer[i] = 0;
  }

  inline t_float process(t_float input, int offset)
  {
    int modDelay =
        std::clamp(
            _delay + offset,
            1,
            _maxDelay - 1);

    _index = (_index + 1) % _maxDelay;
    int readIndex = (_index - modDelay + _maxDelay) % _maxDelay;

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
    _damp = std::clamp(
        damp,
        0.0f,
        0.99f);
  }

  void setDelay(int delay)
  {
    _delay = std::clamp(delay, 1, MAX_BUFFER_SIZE);
    _maxDelay = _delay + BUFFER_OVERLAP;
  }

private:
  int _index = 0;
  t_float _feedback;
  t_float _damp = 0.0f;
  t_float _damped = 0.0f;
  int _delay;
  int _maxDelay;
  std::array<t_float, MAX_BUFFER_SIZE> _buffer;
};
