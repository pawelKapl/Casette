#pragma once

#include <atomic>
#include "dsp/dsp_math.h"
#include "dsp/biquad.h"
#include "dsp/mod_delay_line.h"
#include "dsp/random_lfo.h"
#include "dsp/smoothed_value.h"

#define SPRING_COUNT 3
#define SPRING_STAGES 100
// fs / (2 * SPRING_CEILING) at 48 kHz
#define SPRING_STRETCH 6
#define SPRING_CEILING 4000.0f
#define SPRING_BLOCK 64
#define SPRING_MAX_LOOP 0.065f
#define SPRING_MAX_PRE_DELAY 0.5f
#define SPRING_WANDER 4.0f
#define SPRING_LOW_CUT 120.0f
#define SPRING_GLIDE_TIME 0.05f
// level matched to room while playing
#define SPRING_OUTPUT_GAIN 0.54f

struct Spring
{
  // last SPRING_STRETCH values of every allpass stage, oldest first
  std::array<t_float, SPRING_STAGES * SPRING_STRETCH> chain;
  ModDelayLine loop;
  RandomLFO wander;
  Biquad damping;
  Biquad lowCut;
  SmoothedValue loopDelay;
  SmoothedValue dispersion;
  SmoothedValue feedback;
  t_float loopOut = 0.0f;

  void clear()
  {
    chain.fill(0.0f);
    loop.clear();
    damping.clear();
    lowCut.clear();
    loopOut = 0.0f;
  }

  // frames <= SPRING_BLOCK < loop delay, so the block's loop input is known before the chain runs
  void process(const t_float *in, t_float *out, int frames)
  {
    t_float x[SPRING_BLOCK];

    for (int n = 0; n < frames; n++)
    {
      // pickup sits halfway along the spring: first arrival after half a trip, echoes every full one
      t_float delay = loopDelay.next() + wander.tick() * SPRING_WANDER;
      out[n] = loop.read(delay * 0.5f - n);
      t_float back = loop.read(delay - n);

      x[n] = in[n] + loopOut * feedback.next();
      loopOut = lowCut.process(damping.process(back));
    }

    disperse(x, frames);

    for (int n = 0; n < frames; n++)
      loop.write(x[n]);
  }

  // stretched allpass cascade, lows come out later than highs - every pass smears into a falling chirp
  inline void disperse(t_float *x, int frames)
  {
    t_float a = dispersion.next();
    int tail = frames % SPRING_STRETCH;
    int whole = frames - tail;

    for (int stage = 0; stage < SPRING_STAGES; stage++)
    {
      t_float *state = &chain[stage * SPRING_STRETCH];
      t_float h[SPRING_STRETCH];
      for (int j = 0; j < SPRING_STRETCH; j++)
        h[j] = state[j];

      // sample n only depends on n - SPRING_STRETCH, so the lanes run side by side in registers
      for (int n = 0; n < whole; n += SPRING_STRETCH)
        for (int j = 0; j < SPRING_STRETCH; j++)
          step(x[n + j], h[j], a);

      for (int j = 0; j < SPRING_STRETCH; j++)
        if (j < tail)
          step(x[whole + j], h[j], a);

      // keep the state oldest first
      for (int r = 0; r < tail; r++)
      {
        t_float first = h[0];
        for (int j = 0; j < SPRING_STRETCH - 1; j++)
          h[j] = h[j + 1];
        h[SPRING_STRETCH - 1] = first;
      }

      for (int j = 0; j < SPRING_STRETCH; j++)
        state[j] = h[j];
    }
  }

  static inline void step(t_float &x, t_float &h, t_float a)
  {
    t_float v = x - a * h;
    x = a * v + h;
    h = v;
  }
};

struct SpringReverb
{
  Spring springs[SPRING_COUNT];
  ModDelayLine preDelay;
  SmoothedValue preDelayLength;
  Biquad ceiling[2];
  t_float drive = 1.0f;
  t_float makeup = 1.0f;
  t_float size = 0.5f;
  t_float decay = 0.4f;
  t_float tension = 0.5f;
  int samplingFrequency = 48000;

  std::atomic<int> requestedCount{2};
  int activeCount = 2;

  static constexpr t_float LENGTHS[SPRING_COUNT] = {1.0f, 1.23f, 0.86f};
  static constexpr t_float TENSIONS[SPRING_COUNT] = {0.0f, 0.03f, -0.04f};

  t_float *init(t_float *ptr, int fs)
  {
    samplingFrequency = fs;
    ptr = preDelay.init(ptr, int(SPRING_MAX_PRE_DELAY * fs));
    preDelayLength.setTime(SPRING_GLIDE_TIME, fs);

    for (int i = 0; i < SPRING_COUNT; i++)
    {
      Spring &spring = springs[i];
      ptr = spring.loop.init(ptr, int(SPRING_MAX_LOOP * fs));
      spring.wander = {0.6f + 0.25f * i, fs, 17u + 31u * i};
      spring.lowCut.setHighPass(SPRING_LOW_CUT, 0.707f, fs);
      spring.loopDelay.setTime(SPRING_GLIDE_TIME, fs);
      spring.feedback.setTime(SPRING_GLIDE_TIME, fs);
      // stepped once per block
      spring.dispersion.setTime(SPRING_GLIDE_TIME, fs / SPRING_BLOCK);
    }

    // 4th order butterworth, the stretched allpasses mirror everything above the ceiling
    ceiling[0].setLowPass(SPRING_CEILING, 0.541f, fs);
    ceiling[1].setLowPass(SPRING_CEILING, 1.307f, fs);

    setDamping(0.1f);
    setDwell(0.4f);
    setPreDelay(0.0f);
    update();
    settle();
    clear();
    return ptr;
  }

  void clear()
  {
    for (auto &spring : springs)
      spring.clear();
    preDelay.clear();
    ceiling[0].clear();
    ceiling[1].clear();
  }

  void settle()
  {
    activeCount = requestedCount.load();
    preDelayLength.snap();
    for (auto &spring : springs)
    {
      spring.loopDelay.snap();
      spring.dispersion.snap();
      spring.feedback.snap();
    }
  }

  void setSizeDecay(float newSize, float newDecay)
  {
    size = newSize;
    decay = newDecay;
    update();
  }

  void setTension(float newTension)
  {
    tension = newTension;
    update();
  }

  void setDamping(float damp)
  {
    for (auto &spring : springs)
      spring.damping.setLowPass(SPRING_CEILING * (1.0f - 0.8f * damp), 0.707f, samplingFrequency);
  }

  void setDwell(float dwell)
  {
    drive = 0.5f * powf(12.0f, dwell);
    makeup = 1.0f / std::sqrt(drive);
  }

  void setPreDelay(float seconds)
  {
    preDelayLength.setTarget(std::clamp(seconds * samplingFrequency, 1.0f, (float)preDelay.maxDelay()));
  }

  void setCount(int count)
  {
    requestedCount.store(std::clamp(count, 1, SPRING_COUNT), std::memory_order_release);
  }

  inline bool countChangePending() const
  {
    return requestedCount.load(std::memory_order_relaxed) != activeCount;
  }

  // audio thread, while the wet is faded out
  void applyCount()
  {
    int count = requestedCount.load(std::memory_order_acquire);
    for (int i = activeCount; i < count; i++)
      springs[i].clear();
    activeCount = count;
  }

  void update()
  {
    t_float rt60 = 0.5f * powf(16.0f, decay);

    for (int i = 0; i < SPRING_COUNT; i++)
    {
      Spring &spring = springs[i];
      t_float a = -std::clamp(0.75f - 0.4f * tension + TENSIONS[i], 0.2f, 0.85f);
      t_float loop = (0.01f + 0.04f * size) * LENGTHS[i] * samplingFrequency;
      loop = std::clamp(loop, 2.0f * (SPRING_BLOCK + SPRING_WANDER + 1.0f), (t_float)spring.loop.maxDelay() - SPRING_WANDER - 1.0f);

      // lows spend K(1-a)/(1+a) per stage in the chain, highs K(1+a)/(1-a), decay is set from the middle
      t_float chainDelay = SPRING_STAGES * SPRING_STRETCH * 0.5f * ((1.0f - a) / (1.0f + a) + (1.0f + a) / (1.0f - a));

      spring.dispersion.setTarget(a);
      spring.loopDelay.setTarget(loop);
      spring.feedback.setTarget(powf(10.0f, -3.0f * (loop + chainDelay) / samplingFrequency / rt60));
    }
  }

  void process(const t_float *inL, const t_float *inR, t_float *outL, t_float *outR, int frames)
  {
    for (int start = 0; start < frames; start += SPRING_BLOCK)
    {
      int count = std::min(SPRING_BLOCK, frames - start);
      t_float x[SPRING_BLOCK];
      t_float s0[SPRING_BLOCK];
      t_float s1[SPRING_BLOCK];
      t_float s2[SPRING_BLOCK];

      for (int n = 0; n < count; n++)
      {
        // dwell: how hard the driver pushes the tank, pushed hard it splashes
        t_float driven = std::clamp((inL[start + n] + inR[start + n]) * 0.5f * drive, -3.0f, 3.0f);
        driven = fast_tanh(driven) * makeup;

        t_float delayed = preDelay.read(preDelayLength.next());
        preDelay.write(driven);
        x[n] = ceiling[1].process(ceiling[0].process(delayed));
      }

      springs[0].process(x, s0, count);
      if (activeCount > 1)
        springs[1].process(x, s1, count);
      if (activeCount > 2)
        springs[2].process(x, s2, count);

      for (int n = 0; n < count; n++)
      {
        t_float l = s0[n];
        t_float r = s0[n];
        if (activeCount == 2)
          r = s1[n];
        else if (activeCount > 2)
        {
          l = (s0[n] + 0.5f * s2[n]) * 0.894f;
          r = (s1[n] + 0.5f * s2[n]) * 0.894f;
        }
        outL[start + n] = l * SPRING_OUTPUT_GAIN;
        outR[start + n] = r * SPRING_OUTPUT_GAIN;
      }
    }
  }
};
