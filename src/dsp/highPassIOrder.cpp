#include "highPassIOrder.h"

HighPassIOrder::HighPassIOrder()
{
  setCutOff(250, 48000);
}

void HighPassIOrder::filter(t_float* input, t_float* output, int frames)
{
  for (int i = 0; i < frames; ++i)
    filter(input[i], &output[i]);
}

void HighPassIOrder::filter(t_float input, t_float *output)
{
  t_float x = input;
  t_float y = _b0 * (x - _lastX) - _a1 * _lastY;
  _lastX = x;
  _lastY = y;
  *output = y;
}

void HighPassIOrder::setCutOff(int freq, int samplingFrequency)
{
  _cutOffFreq = freq;
  t_float K = onePoleK(_cutOffFreq, samplingFrequency);
  _b0 = 1.0 / (1.0 + K);
  _a1 = (K - 1.0) / (K + 1.0);
}
