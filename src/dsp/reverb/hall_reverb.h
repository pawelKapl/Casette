#pragma once

#include "dsp/dsp_math.h"
#include "dsp/diffuser.h"
#include "dsp/mod_delay_line.h"
#include "dsp/random_lfo.h"
#include "dsp/smoothed_value.h"

#define HALL_LINES 8
#define HALL_MAX_SCALE 2.4f
#define HALL_MAX_PRE_DELAY 0.5f
#define HALL_MAX_MOD 0.002f
#define HALL_GLIDE_TIME 0.1f
#define HALL_FREEZE_TIME 0.05f
// headroom in the tank, long decays build up a lot
#define HALL_INPUT_GAIN 0.25f
// level matched to room while playing
#define HALL_OUTPUT_GAIN 0.6f

struct HallReverb
{
  ModDelayLine lines[HALL_LINES];
  RandomLFO wander[HALL_LINES];
  SmoothedValue lengths[HALL_LINES];
  SmoothedValue gains[HALL_LINES];
  t_float damped[HALL_LINES] = {};
  ModDelayLine preDelay[2];
  SmoothedValue preDelayLength;
  AllPass diffusers[2][4];
  SmoothedValue freeze;
  SmoothedValue damping;
  SmoothedValue modDepth;
  t_float size = 0.5f;
  t_float decay = 0.4f;
  int samplingFrequency = 48000;

  // at 48 kHz, scaled by size
  static constexpr int BASE_LENGTHS[HALL_LINES] = {1327, 1487, 1663, 1847, 2053, 2273, 2503, 2741};
  static constexpr t_float RATE_SPREAD[HALL_LINES] = {0.81f, 1.13f, 0.94f, 1.27f, 0.88f, 1.06f, 1.19f, 0.97f};
  static constexpr int DIFFUSER_LENGTHS[2][4] = {{229, 173, 611, 447}, {241, 163, 593, 467}};
  static constexpr t_float DIFFUSER_GAINS[4] = {0.75f, 0.75f, 0.625f, 0.625f};

  t_float *init(t_float *ptr, int fs)
  {
    samplingFrequency = fs;

    for (int c = 0; c < 2; c++)
    {
      ptr = preDelay[c].init(ptr, int(HALL_MAX_PRE_DELAY * fs));
      for (int k = 0; k < 4; k++)
        diffusers[c][k] = AllPass(DIFFUSER_GAINS[k], DIFFUSER_LENGTHS[c][k] * fs / 48000);
    }
    preDelayLength.setTime(HALL_GLIDE_TIME, fs);
    freeze.setTime(HALL_FREEZE_TIME, fs);
    damping.setTime(HALL_GLIDE_TIME, fs);
    modDepth.setTime(HALL_GLIDE_TIME, fs);

    for (int i = 0; i < HALL_LINES; i++)
    {
      ptr = lines[i].init(ptr, int((BASE_LENGTHS[i] * HALL_MAX_SCALE / 48000.0f + HALL_MAX_MOD) * fs) + 2);
      wander[i] = {RATE_SPREAD[i], fs, 101u + 7919u * i};
      lengths[i].setTime(HALL_GLIDE_TIME, fs);
      gains[i].setTime(HALL_GLIDE_TIME, fs);
    }

    setDamping(0.1f);
    setModDepth(0.35f);
    setModRate(0.4f);
    setPreDelay(0.0f);
    update();
    settle();
    clear();
    return ptr;
  }

  void clear()
  {
    for (int i = 0; i < HALL_LINES; i++)
    {
      lines[i].clear();
      damped[i] = 0.0f;
    }
    for (int c = 0; c < 2; c++)
    {
      preDelay[c].clear();
      for (auto &diffuser : diffusers[c])
        diffuser.clear();
    }
  }

  void settle()
  {
    preDelayLength.snap();
    freeze.snap();
    damping.snap();
    modDepth.snap();
    for (int i = 0; i < HALL_LINES; i++)
    {
      lengths[i].snap();
      gains[i].snap();
    }
  }

  void setSizeDecay(float newSize, float newDecay)
  {
    size = newSize;
    decay = newDecay;
    update();
  }

  void setDamping(float damp)
  {
    damping.setTarget(0.85f * damp);
  }

  // 0..1 of HALL_MAX_MOD
  void setModDepth(float depth)
  {
    modDepth.setTarget(depth * HALL_MAX_MOD * samplingFrequency);
  }

  void setModRate(float rate)
  {
    for (int i = 0; i < HALL_LINES; i++)
      wander[i].setRate(rate * RATE_SPREAD[i]);
  }

  void setPreDelay(float seconds)
  {
    preDelayLength.setTarget(std::clamp(seconds * samplingFrequency, 1.0f, (float)preDelay[0].maxDelay()));
  }

  void setFreeze(bool on)
  {
    freeze.setTarget(on ? 1.0f : 0.0f);
  }

  void update()
  {
    t_float rt60 = 0.5f * powf(120.0f, decay);
    t_float scale = (0.4f + 2.0f * size) * samplingFrequency / 48000.0f;

    for (int i = 0; i < HALL_LINES; i++)
    {
      t_float length = BASE_LENGTHS[i] * scale;
      lengths[i].setTarget(length);
      gains[i].setTarget(powf(10.0f, -3.0f * length / samplingFrequency / rt60));
    }
  }

  // linear up to 1, eases into 2 - only extreme decays with loud sustained input get there
  static inline t_float softClip(t_float x)
  {
    t_float a = std::abs(x);
    if (a <= 1.0f)
      return x;
    t_float over = std::min(a - 1.0f, 2.0f);
    return std::copysign(1.0f + over - 0.25f * over * over, x);
  }

  static inline void hadamard(t_float *x)
  {
    for (int h = 1; h < HALL_LINES; h *= 2)
      for (int i = 0; i < HALL_LINES; i += 2 * h)
        for (int j = i; j < i + h; j++)
        {
          t_float a = x[j];
          t_float b = x[j + h];
          x[j] = a + b;
          x[j + h] = a - b;
        }
  }

  void process(const t_float *inL, const t_float *inR, t_float *outL, t_float *outR, int frames)
  {
    const t_float norm = 0.35355339f; // 1 / sqrt(HALL_LINES) keeps the hadamard lossless

    for (int n = 0; n < frames; n++)
    {
      // frozen: lossless loop, no damping, input faded out
      t_float frozen = freeze.next();
      t_float inputGain = (1.0f - frozen) * HALL_INPUT_GAIN;
      t_float damp = damping.next() * (1.0f - frozen);
      t_float depth = modDepth.next();

      t_float delay = preDelayLength.next();
      t_float x[2] = {preDelay[0].read(delay), preDelay[1].read(delay)};
      preDelay[0].write(inL[n]);
      preDelay[1].write(inR[n]);
      for (int c = 0; c < 2; c++)
        for (auto &diffuser : diffusers[c])
          x[c] = diffuser.process(x[c], 0);

      t_float y[HALL_LINES];
      t_float l = 0.0f;
      t_float r = 0.0f;
      for (int i = 0; i < HALL_LINES; i++)
      {
        y[i] = lines[i].read(lengths[i].next() + wander[i].tick() * depth);

        // two orthogonal hadamard rows, so left and right come out decorrelated
        l += (i & 1) ? -y[i] : y[i];
        r += (i & 2) ? -y[i] : y[i];

        damped[i] = (1.0f - damp) * y[i] + damp * damped[i];
        t_float gain = gains[i].next();
        y[i] = damped[i] * (gain + (1.0f - gain) * frozen) * norm;
      }

      hadamard(y);

      for (int i = 0; i < HALL_LINES; i++)
        lines[i].write(softClip(y[i] + x[i & 1] * inputGain));

      outL[n] = l * HALL_OUTPUT_GAIN;
      outR[n] = r * HALL_OUTPUT_GAIN;
    }
  }
};
