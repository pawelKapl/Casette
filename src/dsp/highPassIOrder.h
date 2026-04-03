#pragma once

#include "dsp_math.h"

class HighPassIOrder
{
public:
  HighPassIOrder();
  void filter(t_float *input, t_float *output, int frames);
  void filter(t_float input, t_float *output);
  void setCutOff(int cutOff, int samplingFrequency);

private:
  int _cutOffFreq = 100;
  t_float _alpha = 0;
  t_float _lastX = 0;
  t_float _lastY = 0;
};