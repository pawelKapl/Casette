#include "highPassIOrder.h"

HighPassIOrder::HighPassIOrder()
{
  setCutOff(250, 44100);
}

void HighPassIOrder::filter(t_float* input, t_float* output, int frames)
{
  static t_float lastX = 0;
  static t_float lastY = 0;
  for (int i = 0; i < frames; ++i)
  {
    t_float x = input[i];
    t_float y = _alpha * (lastY + x - lastX);
    lastX = x;
    lastY = y;
    output[i] = y;
  }
}

void HighPassIOrder::filter(t_float input, t_float *output)
{
  t_float x = input;
  t_float y = _alpha * (_lastY + x - _lastX);
  _lastX = x;
  _lastY = y;
  *output = y;
}

void HighPassIOrder::setCutOff(int freq, int samplingFrequency)
{
  _cutOffFreq = freq;
  t_float RC = 1.0 / (2.0 * M_PI * _cutOffFreq);
  t_float dt = 1.0 / samplingFrequency;
  _alpha = RC / (RC + dt);
}