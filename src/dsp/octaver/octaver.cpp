#include "octaver.h"

#define POLY_PEAK_TIME 0.003f
// voices matched to the input level
#define POLY_UP1_GAIN 1.35f
#define POLY_UP2_GAIN 0.8f
#define ANALOG_UP2_GAIN 1.9f
#define ANALOG_TRACKING 300.0f
#define ANALOG_UP_FILTER 1000.0f
#define ANALOG_MAX_GAIN 8.0f
#define ANALOG_UP_GAIN 2.4f

// lanes spelled out so the sum vectorizes without fast-math, count is a multiple of 8
static inline t_float sumBands(const t_float *v, int count)
{
  t_float acc[8] = {};
  for (int k = 0; k < count; k += 8)
    for (int j = 0; j < 8; j++)
      acc[j] += v[k + j];
  return ((acc[0] + acc[4]) + (acc[1] + acc[5])) + ((acc[2] + acc[6]) + (acc[3] + acc[7]));
}

void PolyOctaver::init(int fs)
{
  for (int k = 0; k < OCTAVER_BANDS; k++)
  {
    t_float freq = OCTAVER_LOWEST_BAND * powf(2.0f, k / OCTAVER_BANDS_PER_OCTAVE);
    t_float bandwidth = std::max(OCTAVER_BANDWIDTH * freq, OCTAVER_MIN_BANDWIDTH);
    t_float r = expf(-M_PI * bandwidth / fs);
    t_float w = 2.0f * M_PI * freq / fs;
    poleRe[k] = r * cosf(w);
    poleIm[k] = r * sinf(w);
    gain[k] = 1.0f - r;
  }
  peakCoeff = 1.0f - expf(-1.0f / (POLY_PEAK_TIME * fs));
  clear();
}

void PolyOctaver::clear()
{
  for (int k = 0; k < OCTAVER_BANDS; k++)
  {
    aRe[k] = aIm[k] = bRe[k] = bIm[k] = 0.0f;
    halfRe[k] = quarterRe[k] = 1.0f;
    halfIm[k] = quarterIm[k] = 0.0f;
    peak[k] = 0.0f;
  }
}

void PolyOctaver::process(const t_float *in, t_float *const *voices, const bool *active, int frames)
{
  bool up = active[2] || active[3];
  bool sub = active[0] || active[1];

  for (int n = 0; n < frames; n++)
  {
    t_float x = in[n];
    alignas(16) t_float mag[OCTAVER_BANDS];
    alignas(16) t_float c[OCTAVER_BANDS];
    alignas(16) t_float s[OCTAVER_BANDS];
    alignas(16) t_float out[OCTAVER_VOICES][OCTAVER_BANDS];

    for (int k = 0; k < OCTAVER_BANDS; k++)
    {
      // two cascaded complex one-poles, unity gain at the band centre
      t_float ar = poleRe[k] * aRe[k] - poleIm[k] * aIm[k] + gain[k] * x;
      t_float ai = poleRe[k] * aIm[k] + poleIm[k] * aRe[k];
      t_float br = poleRe[k] * bRe[k] - poleIm[k] * bIm[k] + gain[k] * ar;
      t_float bi = poleRe[k] * bIm[k] + poleIm[k] * bRe[k] + gain[k] * ai;
      aRe[k] = ar;
      aIm[k] = ai;
      bRe[k] = br;
      bIm[k] = bi;

      // a real partial shows up as half its amplitude on the positive frequency side
      mag[k] = 2.0f * std::sqrt(br * br + bi * bi);
      t_float inv = 2.0f / (mag[k] + 1e-20f);
      c[k] = br * inv;
      s[k] = bi * inv;
    }

    if (up)
    {
      for (int k = 0; k < OCTAVER_BANDS; k++)
      {
        t_float c2 = c[k] * c[k] - s[k] * s[k];
        t_float s2 = 2.0f * c[k] * s[k];
        out[2][k] = mag[k] * c2;
        out[3][k] = mag[k] * (c2 * c2 - s2 * s2);
      }
      voices[2][n] = sumBands(out[2], OCTAVER_BANDS) * POLY_UP1_GAIN;
      voices[3][n] = sumBands(out[3], OCTAVER_BANDS) * POLY_UP2_GAIN;
    }

    if (sub)
    {
      // a partial shows up in two bands with independent sign flips, only the stronger one contributes
      for (int k = 0; k < OCTAVER_SUB_BANDS; k++)
      {
        t_float below = k > 0 ? mag[k - 1] : 0.0f;
        t_float isPeak = (mag[k] >= below && mag[k] >= mag[k + 1]) ? 1.0f : 0.0f;
        peak[k] += (isPeak - peak[k]) * peakCoeff;
      }

      for (int k = 0; k < OCTAVER_SUB_BANDS; k++)
      {
        // of the two square roots take the one closest to the last sample, the phase keeps rotating smoothly
        t_float hr = std::sqrt(std::max(0.5f * (1.0f + c[k]), 0.0f));
        t_float hi = std::copysign(std::sqrt(std::max(0.5f * (1.0f - c[k]), 0.0f)), s[k]);
        t_float flip = (hr * halfRe[k] + hi * halfIm[k] < 0.0f) ? -1.0f : 1.0f;
        halfRe[k] = hr * flip;
        halfIm[k] = hi * flip;

        t_float qr = std::sqrt(std::max(0.5f * (1.0f + halfRe[k]), 0.0f));
        t_float qi = std::copysign(std::sqrt(std::max(0.5f * (1.0f - halfRe[k]), 0.0f)), halfIm[k]);
        t_float flip2 = (qr * quarterRe[k] + qi * quarterIm[k] < 0.0f) ? -1.0f : 1.0f;
        quarterRe[k] = qr * flip2;
        quarterIm[k] = qi * flip2;

        out[1][k] = mag[k] * peak[k] * halfRe[k];
        out[0][k] = mag[k] * peak[k] * quarterRe[k];
      }
      voices[1][n] = sumBands(out[1], OCTAVER_SUB_BANDS);
      voices[0][n] = sumBands(out[0], OCTAVER_SUB_BANDS);
    }
  }
}

void AnalogOctaver::init(int fs)
{
  tracking[0].setLowPass(ANALOG_TRACKING, 0.541f, fs);
  tracking[1].setLowPass(ANALOG_TRACKING, 1.307f, fs);
  upBand[0].setHighPass(80.0f, 0.707f, fs);
  upBand[1].setLowPass(ANALOG_UP_FILTER, 0.707f, fs);
  upDc[0].setHighPass(40.0f, 0.707f, fs);
  upDc[1].setHighPass(40.0f, 0.707f, fs);
  attack = expf(-1.0f / (0.001f * fs));
  release = expf(-1.0f / (0.05f * fs));
  clear();
}

void AnalogOctaver::clear()
{
  for (int i = 0; i < 2; i++)
  {
    tracking[i].clear();
    upBand[i].clear();
    upDc[i].clear();
  }
  envelope = 0.0f;
  inputEnvelope = 0.0f;
  previous = 0.0f;
  armed = false;
  flip1 = 1.0f;
  flip2 = 1.0f;
}

void AnalogOctaver::process(const t_float *in, t_float *const *voices, const bool *active, int frames)
{
  bool up = active[2] || active[3];

  for (int n = 0; n < frames; n++)
  {
    t_float t = tracking[1].process(tracking[0].process(in[n]));

    t_float level = std::abs(t);
    t_float coeff = level > envelope ? attack : release;
    envelope = coeff * (envelope - level) + level;

    // tracking filter dulls high notes, the subs follow the input's envelope instead
    t_float inputLevel = std::abs(in[n]);
    coeff = inputLevel > inputEnvelope ? attack : release;
    inputEnvelope = coeff * (inputEnvelope - inputLevel) + inputLevel;
    t_float makeup = std::min(inputEnvelope / (envelope + 1e-9f), ANALOG_MAX_GAIN);

    // arm below a fraction of the envelope so noise can't toggle, flip right at the zero crossing so the product stays continuous
    if (t < -0.2f * envelope)
      armed = true;
    if (armed && previous < 0.0f && t >= 0.0f)
    {
      armed = false;
      flip1 = -flip1;
      if (flip1 > 0.0f)
        flip2 = -flip2;
    }
    previous = t;

    // -1: polarity flipped every cycle; -2: rectified and flipped every second cycle, four lobes up then four down
    voices[1][n] = t * flip1 * makeup;
    voices[0][n] = std::abs(t) * flip2 * makeup;

    // rectifying the near-sine tracking signal keeps the octave clean, a full band signal brings its even harmonics along
    if (up)
    {
      t_float octave = upDc[0].process(std::abs(t)) * makeup * ANALOG_UP_GAIN;
      voices[2][n] = octave;
      voices[3][n] = upDc[1].process(std::abs(octave)) * ANALOG_UP2_GAIN;
    }
  }
}

Octaver::Octaver(int samplingFrequency)
    : _samplingFrequency(samplingFrequency)
{
  _fadeStep = 1.0f / (OCTAVER_FADE_TIME * samplingFrequency);
  _dry.setTime(OCTAVER_LEVEL_TIME, samplingFrequency);
  for (auto &level : _levels)
    level.setTime(OCTAVER_LEVEL_TIME, samplingFrequency);
  _toneGlide.setTime(OCTAVER_TONE_TIME, samplingFrequency);

  _poly.init(samplingFrequency);
  _analog.init(samplingFrequency);
  setDry(1.0f);
  setTone(0.6f);
  settle();
}

void Octaver::setTone(t_float tone)
{
  _toneGlide.setTarget(tone);
}

void Octaver::updateTone()
{
  t_float tone = _toneGlide.next();
  if (tone != _appliedTone)
  {
    _tone.setLowPass(300.0f * powf(8000.0f / 300.0f, tone), 0.707f, _samplingFrequency);
    _appliedTone = tone;
  }
}

void Octaver::setOn(bool on)
{
  // audio thread has stopped touching it, so the tail of whatever played before can be wiped
  if (on && !running())
    clear();
  _on.store(on, std::memory_order_release);
}

void Octaver::setMode(OctaverMode mode)
{
  if (mode < 0 || mode >= OCTAVER_MODE_COUNT)
    mode = POLYPHONIC;

  if (mode == _requestedMode.load(std::memory_order_relaxed))
    return;

  // audio thread only touches the active and requested modes, so this one is safe to clear
  if (mode != _activeMode.load(std::memory_order_acquire))
    clearMode(mode);

  _requestedMode.store(mode, std::memory_order_release);
}

// skips fades and glides - only while the audio thread isn't running this instance
void Octaver::settle()
{
  _activeMode.store(_requestedMode.load());
  _fade = 1.0f;
  _onGain = _on ? 1.0f : 0.0f;
  _running.store(_on);
  _dry.snap();
  for (auto &level : _levels)
    level.snap();
  _toneGlide.snap();
  updateTone();
}

// only while the audio thread isn't running this instance
void Octaver::clear()
{
  _poly.clear();
  _analog.clear();
  _tone.clear();
}

void Octaver::clearMode(OctaverMode mode)
{
  switch (mode)
  {
  case POLYPHONIC:
    _poly.clear();
    break;
  case ANALOG:
    _analog.clear();
    break;
  default:
    break;
  }
}

void Octaver::process(const t_float *input, t_float *output, int frames)
{
  OctaverMode requested = _requestedMode.load(std::memory_order_acquire);
  OctaverMode active = _activeMode.load(std::memory_order_relaxed);

  // switch only once the old voices have faded out
  if (requested != active && _fade <= 0.0f)
  {
    active = requested;
    _activeMode.store(active, std::memory_order_release);
  }

  t_float fadeTarget = (requested == active) ? 1.0f : 0.0f;
  t_float onTarget = _on.load(std::memory_order_acquire) ? 1.0f : 0.0f;

  for (int start = 0; start < frames; start += OCTAVER_BLOCK)
  {
    int count = std::min(OCTAVER_BLOCK, frames - start);
    t_float voiceBuffers[OCTAVER_VOICES][OCTAVER_BLOCK] = {};
    t_float *voices[OCTAVER_VOICES];
    bool voiceActive[OCTAVER_VOICES];
    for (int v = 0; v < OCTAVER_VOICES; v++)
    {
      voices[v] = voiceBuffers[v];
      voiceActive[v] = _levels[v].current() > 0.0f || _levels[v].target() > 0.0f;
    }

    if (active == POLYPHONIC)
      _poly.process(input + start, voices, voiceActive, count);
    else
      _analog.process(input + start, voices, voiceActive, count);

    for (int n = 0; n < count; n++)
    {
      t_float wet = 0.0f;
      for (int v = 0; v < OCTAVER_VOICES; v++)
        wet += voices[v][n] * _levels[v].next();

      _fade += std::clamp(fadeTarget - _fade, -_fadeStep, _fadeStep);
      _onGain += std::clamp(onTarget - _onGain, -_fadeStep, _fadeStep);
      t_float dry = input[start + n];
      updateTone();
      t_float octaved = dry * _dry.next() + _tone.process(wet) * _fade;
      output[start + n] = dry + (octaved - dry) * _onGain;
    }
  }

  _running.store(_onGain > 0.0f, std::memory_order_release);
}
