#pragma once

#include "dsp_math.h"

class ModDelayLine
{
public:
  // returns the pointer just past the memory taken
  t_float *init(t_float *memory, int maxDelaySamples)
  {
    _size = 1;
    while (_size < maxDelaySamples + 2)
      _size <<= 1;
    _mask = _size - 1;
    _buffer = memory;
    clear();

    return memory + _size;
  }

  void clear()
  {
    std::fill(_buffer, _buffer + _size, 0.0f);
    _writeIndex = 0;
  }

  inline void write(t_float sample)
  {
    _buffer[_writeIndex] = sample;
    _writeIndex = (_writeIndex + 1) & _mask;
  }

  // linear interpolation, delay in samples
  inline t_float read(t_float delay) const
  {
    delay = std::clamp(delay, (t_float)1.0, (t_float)(_size - 2));

    // + _size keeps it positive, so the int cast floors
    t_float readPos = (t_float)(_writeIndex + _size) - delay;
    int index = (int)readPos;
    t_float frac = readPos - index;

    t_float a = _buffer[index & _mask];
    t_float b = _buffer[(index + 1) & _mask];

    return a + frac * (b - a);
  }

  inline t_float read(int delay) const
  {
    delay = std::clamp(delay, 1, _size - 1);
    return _buffer[(_writeIndex - delay) & _mask];
  }

  inline int maxDelay() const
  {
    return _size - 2;
  }

private:
  t_float *_buffer = nullptr;
  int _size = 0;
  int _mask = 0;
  int _writeIndex = 0;
};
