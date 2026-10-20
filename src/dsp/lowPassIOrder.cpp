#include "lowPassIOrder.h"

LowPassIOrder::LowPassIOrder()
{
  setCutOff(250, 48000);
}

void LowPassIOrder::filter(t_float* input, t_float* output, int frames)
{
  for (int i = 0; i < frames; ++i)
    filter(input[i], &output[i]);
}

void LowPassIOrder::filter(t_float input, t_float *output)
{
  t_float x = input;
  t_float y = _b0 * (x + _lastX) - _a1 * _lastY;
  _lastX = x;
  _lastY = y;
  *output = y;
}

void LowPassIOrder::setCutOff(int cutOff, int samplingFrequency)
{
  _cutOffFreq = cutOff;
  t_float K = onePoleK(_cutOffFreq, samplingFrequency);
  _b0 = K / (1.0 + K);
  _a1 = (K - 1.0) / (K + 1.0);
}
