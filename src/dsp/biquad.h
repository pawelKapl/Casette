#pragma once

#include "dsp_math.h"

class Biquad
{
public:
  void setLowPass(float freq, float q, int samplingFrequency)
  {
    float w = omega(freq, samplingFrequency);
    float cosw = cosf(w);
    float alpha = sinf(w) / (2.0f * q);
    setCoefficients((1.0f - cosw) * 0.5f, 1.0f - cosw, (1.0f - cosw) * 0.5f, 1.0f + alpha, -2.0f * cosw, 1.0f - alpha);
  }

  void setHighPass(float freq, float q, int samplingFrequency)
  {
    float w = omega(freq, samplingFrequency);
    float cosw = cosf(w);
    float alpha = sinf(w) / (2.0f * q);
    setCoefficients((1.0f + cosw) * 0.5f, -(1.0f + cosw), (1.0f + cosw) * 0.5f, 1.0f + alpha, -2.0f * cosw, 1.0f - alpha);
  }

  inline t_float process(t_float x)
  {
    t_float y = _b0 * x + _z1;
    _z1 = _b1 * x - _a1 * y + _z2;
    _z2 = _b2 * x - _a2 * y;
    return y;
  }

  void clear()
  {
    _z1 = 0.0f;
    _z2 = 0.0f;
  }

private:
  static float omega(float freq, int samplingFrequency)
  {
    return 2.0f * M_PI * std::clamp(freq, 10.0f, 0.49f * samplingFrequency) / samplingFrequency;
  }

  void setCoefficients(float b0, float b1, float b2, float a0, float a1, float a2)
  {
    _b0 = b0 / a0;
    _b1 = b1 / a0;
    _b2 = b2 / a0;
    _a1 = a1 / a0;
    _a2 = a2 / a0;
  }

  t_float _b0 = 1.0f;
  t_float _b1 = 0.0f;
  t_float _b2 = 0.0f;
  t_float _a1 = 0.0f;
  t_float _a2 = 0.0f;
  t_float _z1 = 0.0f;
  t_float _z2 = 0.0f;
};
