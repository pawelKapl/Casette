#pragma once

#define _USE_MATH_DEFINES

#include <array>
#include <math.h>
#include <cmath>
#include <cstring>
#include <memory>
#include <cstdlib>
#include <random>

#include <iostream>

typedef float t_float;

static t_float randomFloat()
{
  t_float r = static_cast<t_float>(rand()) / static_cast<t_float>(RAND_MAX);
  return r * 2.0 - 1.0;
}

static inline void normalize(t_float *value)
{
  if (*value > 1.0)
  {
    *value = 0.999;
  }

  if (*value < -1.0)
  {
    *value = -0.999;
  }
}

static inline t_float fast_sin(t_float x)
{
  return x-(x*x*x/9.87);
}

static inline t_float fast_tanh(t_float x)
{
  return x*(27+x*x)/(27+9*x*x);
}

static inline int msToSamples(float ms, float sampleRate) 
{
    return static_cast<int>((ms / 1000.0f) * sampleRate);
}

static inline std::vector<int> generateCombDelays(float decayMs, int numCombs, float sampleRate, float minDelayMs = 10.0f) 
{
    std::vector<int> delays;

    float maxDelayMs = decayMs;

    float step = (maxDelayMs - minDelayMs) / (numCombs - 1);

    for (int i = 0; i < numCombs; ++i) {
        float delayMs = minDelayMs + i * step;
        delays.push_back(msToSamples(delayMs, sampleRate));
    }

    return delays;
}

static inline float computeFeedback(int delaySamples, float decayTimeSeconds, float sampleRate) 
{
    return pow(10.0f, (-3.0f * delaySamples) / (decayTimeSeconds * sampleRate));
}