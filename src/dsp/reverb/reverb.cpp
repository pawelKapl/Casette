#include "reverb.h"

Reverb::Reverb(int samplingFrequency)
  : _samplingFrequency(samplingFrequency)
{
  _roomReverb[0].right = true;

  _modeFadeStep = 1.0f / (MODE_FADE_TIME * samplingFrequency);
  _wetGain.setTime(MIX_SMOOTHING_TIME, samplingFrequency);
  _dryGain.setTime(MIX_SMOOTHING_TIME, samplingFrequency);
  setMix(0.36f);
  settle();
}

t_float *Reverb::init(t_float *memory)
{
  t_float *currentPtr = memory;

  currentPtr = _roomReverb[0].init(currentPtr, _samplingFrequency);
  currentPtr = _roomReverb[1].init(currentPtr, _samplingFrequency);
  currentPtr = _spring.init(currentPtr, _samplingFrequency);
  currentPtr = _hall.init(currentPtr, _samplingFrequency);

  return currentPtr;
}

void Reverb::process(const t_float* const* input, t_float** output, int frames, int inChannels, int outChannels)
{
  ReverbMode requested = _requestedMode.load(std::memory_order_acquire);
  ReverbMode active = _activeMode.load(std::memory_order_relaxed);

  bool restructure = requested != active || (active == SPRING && _spring.countChangePending());

  // switch only once the old wet has faded out
  if (restructure && _modeFade <= 0.0f)
  {
    active = requested;
    _activeMode.store(active, std::memory_order_release);
    if (active == SPRING)
      _spring.applyCount();
    restructure = false;
  }

  t_float fadeTarget = restructure ? 0.0f : 1.0f;

  switch (active)
  {
    case ROOM:
      processBlock(input, output, frames, inChannels, outChannels, fadeTarget,
                   [this](const t_float *inL, const t_float *inR, t_float *outL, t_float *outR, int count)
                   {
                     for (int i = 0; i < count; i++)
                     {
                       outL[i] = _roomReverb[0].process(inL[i]);
                       outR[i] = _roomReverb[1].process(inR[i]);
                     }
                   });
      break;
    case SPRING:
      processBlock(input, output, frames, inChannels, outChannels, fadeTarget,
                   [this](const t_float *inL, const t_float *inR, t_float *outL, t_float *outR, int count)
                   {
                     _spring.process(inL, inR, outL, outR, count);
                   });
      break;
    case HALL:
      processBlock(input, output, frames, inChannels, outChannels, fadeTarget,
                   [this](const t_float *inL, const t_float *inR, t_float *outL, t_float *outR, int count)
                   {
                     _hall.process(inL, inR, outL, outR, count);
                   });
      break;
    default:
      break;
  }
}

void Reverb::setMode(ReverbMode mode)
{
  if (mode < 0 || mode >= REVERB_MODE_COUNT)
    mode = ROOM;

  if (mode == _requestedMode.load(std::memory_order_relaxed))
    return;

  // audio thread only touches the active and requested modes, so this one is safe to clear
  if (mode != _activeMode.load(std::memory_order_acquire))
    clearMode(mode);

  _requestedMode.store(mode, std::memory_order_release);
}

// skips fades and glides - only while the audio thread isn't running this instance
void Reverb::settle()
{
  _activeMode.store(_requestedMode.load());
  _modeFade = 1.0f;
  _wetGain.snap();
  _dryGain.snap();
  _roomReverb[0].settle();
  _roomReverb[1].settle();
  _spring.settle();
  _hall.settle();
}

void Reverb::clearMode(ReverbMode mode)
{
  switch (mode)
  {
    case ROOM:
      _roomReverb[0].clear();
      _roomReverb[1].clear();
      break;
    case SPRING:
      _spring.clear();
      break;
    case HALL:
      _hall.clear();
      break;
    default:
      break;
  }
}

void Reverb::setHighPass(int freq)
{
  _hiPassFilter[0].setCutOff(freq, _samplingFrequency);
  _hiPassFilter[1].setCutOff(freq, _samplingFrequency);
}

void Reverb::setLowPass(int freq)
{
  _lowPassFilter[0].setCutOff(freq, _samplingFrequency);
  _lowPassFilter[1].setCutOff(freq, _samplingFrequency);
}
