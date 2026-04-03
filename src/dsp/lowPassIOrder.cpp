#include "lowPassIOrder.h"

LowPassIOrder::LowPassIOrder()
{
  setCutOff(250, 44100);
}

void LowPassIOrder::filter(t_float* input, t_float* output, int frames)
{
  static t_float lastY = 0;
  for (int i = 0; i < frames; ++i)
  {
    t_float x = input[i];
    t_float y = _alpha * x + (1.0 - _alpha) * lastY;
    lastY = y;
    output[i] = y;
  }
}

void LowPassIOrder::filter(t_float input, t_float *output)
{
  t_float x = input;
  t_float y = _alpha * x + (1.0 - _alpha) * _lastY;
  _lastY = y;
  *output = y;
}

void LowPassIOrder::setCutOff(int cutOff, int samplingFrequency)
{
  _cutOffFreq = cutOff;
  t_float RC = 1.0 / (2.0 * M_PI * _cutOffFreq);
  t_float dt = 1.0 / samplingFrequency;
  _alpha = dt / (RC + dt);
}