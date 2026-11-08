#pragma once

#include "dsp_math.h"
#include "smoothed_value.h"

// power of two, so the ring buffer wraps with a mask
#define MAX_BUFFER_SIZE 8192
#define BUFFER_MASK (MAX_BUFFER_SIZE - 1)

class Comb
{
public:
  Comb()
  {
    clear();
  }

  Comb(t_float feedback, int delayLength, int samplingFrequency)
      : _feedback(feedback)
  {
    setDelay(delayLength);
    settle();
    clear();
  }

  inline t_float process(t_float input, int offset)
  {
    // delay changes glide instead of jumping the read head, which clicks
    t_float modDelay =
        std::clamp(
            _delay.next() + offset,
            (t_float)1.0,
            (t_float)(MAX_BUFFER_SIZE - 2));

    _index = (_index + 1) & BUFFER_MASK;
    t_float readPos = (t_float)(_index + MAX_BUFFER_SIZE) - modDelay;
    int readIndex = (int)readPos;
    t_float frac = readPos - readIndex;

    t_float a = _buffer[readIndex & BUFFER_MASK];
    t_float b = _buffer[(readIndex + 1) & BUFFER_MASK];
    t_float delayed = a + frac * (b - a);

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
    _delay.setTarget(std::clamp(delay, 1, MAX_BUFFER_SIZE - 2));
  }

  void setDelayGlide(float seconds, int samplingFrequency)
  {
    _delay.setTime(seconds, samplingFrequency);
  }

  void settle()
  {
    _delay.snap();
  }

  void clear()
  {
    _buffer.fill(0);
    _damped = 0.0f;
  }

private:
  int _index = 0;
  t_float _feedback = 0.0f;
  t_float _damp = 0.0f;
  t_float _damped = 0.0f;
  SmoothedValue _delay;
  std::array<t_float, MAX_BUFFER_SIZE> _buffer;
};
