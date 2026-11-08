#pragma once

#include <atomic>
#include "dsp/dsp_math.h"
#include "dsp/biquad.h"
#include "dsp/smoothed_value.h"

#define OCTAVER_BLOCK 64
#define OCTAVER_FADE_TIME 0.005f
#define OCTAVER_LEVEL_TIME 0.02f
#define OCTAVER_TONE_TIME 0.03f
// -2, -1, +1, +2
#define OCTAVER_VOICES 4

#define OCTAVER_BANDS 40
// sub voices only come from the bands below ~1.6 kHz
#define OCTAVER_SUB_BANDS 32
#define OCTAVER_LOWEST_BAND 40.0f
#define OCTAVER_BANDS_PER_OCTAVE 6.0f
// per one-pole stage, as a fraction of the band frequency
#define OCTAVER_BANDWIDTH 0.19f
#define OCTAVER_MIN_BANDWIDTH 20.0f

// stored in presets as an int - only append
enum OctaverMode : int
{
  POLYPHONIC = 0,
  ANALOG = 1,
  OCTAVER_MODE_COUNT
};

// one partial per band as a rotating phasor: squaring doubles its frequency, a continuous square root halves it
struct PolyOctaver
{
  alignas(16) t_float poleRe[OCTAVER_BANDS];
  alignas(16) t_float poleIm[OCTAVER_BANDS];
  alignas(16) t_float gain[OCTAVER_BANDS];
  alignas(16) t_float aRe[OCTAVER_BANDS];
  alignas(16) t_float aIm[OCTAVER_BANDS];
  alignas(16) t_float bRe[OCTAVER_BANDS];
  alignas(16) t_float bIm[OCTAVER_BANDS];
  alignas(16) t_float halfRe[OCTAVER_BANDS];
  alignas(16) t_float halfIm[OCTAVER_BANDS];
  alignas(16) t_float quarterRe[OCTAVER_BANDS];
  alignas(16) t_float quarterIm[OCTAVER_BANDS];
  alignas(16) t_float peak[OCTAVER_BANDS];
  t_float peakCoeff = 0.0f;

  void init(int fs);
  void clear();
  void process(const t_float *in, t_float *const *voices, const bool *active, int frames);
};

// OC-2 style: flip-flops switch the low passed input's polarity at its zero crossings, octave up is rectified
struct AnalogOctaver
{
  Biquad tracking[2];
  Biquad upBand[2];
  Biquad upDc[2];
  t_float envelope = 0.0f;
  t_float inputEnvelope = 0.0f;
  t_float attack = 0.0f;
  t_float release = 0.0f;
  t_float previous = 0.0f;
  bool armed = false;
  t_float flip1 = 1.0f;
  t_float flip2 = 1.0f;

  void init(int fs);
  void clear();
  void process(const t_float *in, t_float *const *voices, const bool *active, int frames);
};

class Octaver
{
public:
  Octaver(int samplingFrequency);
  void process(const t_float *input, t_float *output, int frames);
  void setMode(OctaverMode mode);
  void setOn(bool on);
  void settle();
  void clear();

  // true until a switch-off has faded out
  inline bool running() const
  {
    return _on.load(std::memory_order_relaxed) || _running.load(std::memory_order_acquire);
  }

  inline void setDry(t_float level)
  {
    _dry.setTarget(level);
  }

  inline void setVoiceLevel(int voice, t_float level)
  {
    _levels[voice].setTarget(level);
  }

  void setTone(t_float tone);

private:
  void updateTone();

  void clearMode(OctaverMode mode);

  int _samplingFrequency = 48000;

  // UI requests a mode, audio thread fades the voices out, switches, fades in
  std::atomic<OctaverMode> _requestedMode{POLYPHONIC};
  std::atomic<OctaverMode> _activeMode{POLYPHONIC};
  t_float _fade = 1.0f;
  t_float _fadeStep = 1.0f;

  // crossfade between the plain input and the octaver on switch on/off
  std::atomic<bool> _on{false};
  std::atomic<bool> _running{false};
  t_float _onGain = 0.0f;

  SmoothedValue _dry;
  SmoothedValue _levels[OCTAVER_VOICES];
  Biquad _tone;
  // glides in octaves and the coefficients follow per sample on the audio thread, a jump in them clicks
  SmoothedValue _toneGlide;
  t_float _appliedTone = -1.0f;

  PolyOctaver _poly;
  AnalogOctaver _analog;
};
