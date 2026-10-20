#include "delay.h"
#include <algorithm>

Delay::Delay(int samplingFrequency)
  : _samplingFrequency(samplingFrequency), 
  _lfoWow(0.5, samplingFrequency, 54, 0.25),
  _lfoFlutter(10, samplingFrequency, 1.6, 0.25)
{
  recalculate();
}

t_float *Delay::init(t_float *memory)
{
  t_float *currentPtr = memory;
  for (int i = 0; i < 3; i++)
    currentPtr = initSingleDelay(&_circularDelays[i], currentPtr);
  for (int i = 0; i < 2; i++)
    currentPtr = initSingleDelay(&_dualTapDelays[i], currentPtr);
  for (int i = 0; i < 1; i++)
    currentPtr = initSingleDelay(&_generalDelay[i], currentPtr);
  currentPtr = initSingleDelay(&_reverseDelay, currentPtr);

  _diffuserL.init(_samplingFrequency);
  _diffuserR.init( _samplingFrequency);
  _diffuserC1.init(_samplingFrequency);
  _diffuserC2.init(_samplingFrequency);

  clear();

  return currentPtr;
}

t_float* Delay::initSingleDelay(SingleDelay *delay, t_float *ptr)
{
  t_float *currentMemoryPtr = ptr;

  delay->buffer = currentMemoryPtr;
  currentMemoryPtr += BUFFER_SIZE;

  return currentMemoryPtr;
}

void Delay::clear()
{
  for (int i = 0; i < 3; i++)
    clearSingleDelay(&_circularDelays[i]);
  for (int i = 0; i < 2; i++)
    clearSingleDelay(&_dualTapDelays[i]);
  for (int i = 0; i < 1; i++)
    clearSingleDelay(&_generalDelay[i]);
  clearSingleDelay(&_reverseDelay);

  _grains[0] = {};
  _grains[1] = {};
  _reversePhase = 0;
}

void Delay::clearSingleDelay(SingleDelay *delay)
{
  for (int i = 0; i < BUFFER_SIZE; i++)
    delay->buffer[i] = 0;
}

void Delay::process(const t_float* const* input, t_float** output, int frames, int inChannels, int outChannels)
{
  switch (_mode)
  {
  case CIRCULAR:
    singleDelay(input, output, CIRCULAR, frames, 0, -1, inChannels, outChannels);
    singleDelay(input, output, CIRCULAR, frames, 1, 0, inChannels, outChannels);
    singleDelay(input, output, CIRCULAR, frames, 2, 1, inChannels, outChannels);
    break;
  case PING_PONG:
    singleDelay(input, output, PING_PONG, frames, 0, -1, inChannels, outChannels);
    singleDelay(input, output, PING_PONG, frames, 1, 1, inChannels, outChannels);
    break;
  case DUAL_TAP:
    singleDelay(input, output, DUAL_TAP, frames, 0, 0, inChannels, outChannels);
    singleDelay(input, output, DUAL_TAP, frames, 1, 0, inChannels, outChannels);
    break;
  case MONO:
  case TAPE:
    singleDelay(input, output, MONO, frames, 0, 0, inChannels, outChannels);
    break;
  case REVERSE:
    reverseDelay(input, output, frames, inChannels, outChannels);
    break;
  }
}

void Delay::startGrain(ReverseGrain &grain)
{
  // start at the newest recorded sample and walk backwards from there
  grain.start = (_reverseDelay.writeHead - 1 + BUFFER_SIZE) % BUFFER_SIZE;
  grain.pos = 0;
  grain.length = _reverseLength;
  grain.active = true;
}

void Delay::reverseDelay(const t_float* const* input, t_float** output, int frames, int inChannels, int outChannels)
{
  SingleDelay *delay = &_reverseDelay;

  for (int i = 0; i < frames; i++)
  {
    t_float inputSample = 0;
    if (inChannels == 2)
      inputSample = (input[0][i] * 0.5f) + (input[1][i] * 0.5f);
    else
      inputSample = input[0][i];

    if (_reversePhase == 0)
    {
      _reverseLength = delay->bufferLength;
      startGrain(_grains[0]);
    }
    if (_reversePhase == _reverseLength / 2)
      startGrain(_grains[1]);

    _reversePhase = (_reversePhase + 1) % _reverseLength;

    t_float delayedSample = 0;
    for (auto &grain : _grains)
    {
      if (!grain.active)
        continue;

      int index = (grain.start - grain.pos + BUFFER_SIZE) % BUFFER_SIZE;
      t_float w = std::sin(M_PI * grain.pos / grain.length);
      delayedSample += delay->buffer[index] * w * w;

      if (++grain.pos >= grain.length)
        grain.active = false;
    }

    t_float feedbackSample = delayedSample;

    delay->hiPassFilter.filter(feedbackSample, &feedbackSample);
    delay->lowPassFilter.filter(feedbackSample, &feedbackSample);

    delay->buffer[delay->writeHead] = inputSample + feedbackSample * delay->feedbackLevel;
    delay->writeHead = (delay->writeHead + 1) % BUFFER_SIZE;

    t_float out = delayedSample * _delayMix;
    if (_diffusion) _diffuserC1.process(out, out);

    t_float modifier = outChannels > 1 ? 0.95f : 1.0f;
    if (_reverseWetOnly)
    {
      output[0][i] = out * modifier;
      if (outChannels > 1) output[1][i] = out * modifier;
    }
    else
    {
      output[0][i] += out * modifier;
      if (outChannels > 1) output[1][i] += out * modifier;
    }

    normalize(&output[0][i]);
    if (outChannels > 1)
      normalize(&output[1][i]);
  }
}

void Delay::singleDelay(const t_float* const* input, t_float** output, DelayMode mode, int frames, int delayIndex, int panning, int inChannels, int outChannels)
{
  bool firstTap = (delayIndex == 1);
  SingleDelay* delay;
  switch (_mode)
  {
  case CIRCULAR:
    delay = &_circularDelays[delayIndex];
    break;
  case DUAL_TAP:
    delay = &_dualTapDelays[delayIndex];
    break;
  case MONO:
  case TAPE:
    delay = &_generalDelay[delayIndex];
    break;
  case PING_PONG:
    delay = &_circularDelays[delayIndex];
    break;
  case REVERSE:
    return; // handled by reverseDelay
  }

  for (int i = 0; i < frames; i++)
  {
    t_float inputSample = 0;
    if (inChannels == 2)
      inputSample = (input[0][i] * 0.5f) + (input[1][i] * 0.5f);
    else
      inputSample = input[0][i];

    t_float readHead = (t_float)delay->writeHead - delay->bufferLength;

    if (_modDepth)
    {
      delay->lfo.tick();
      readHead += delay->lfo.getOffset();
    } 

    if (_mode == TAPE)
    {
      _lfoFlutter.tick();
      _lfoWow.tick();
      readHead += _lfoWow.getOffset() + _lfoFlutter.getOffset();
    }

    readHead = fmod(readHead + BUFFER_SIZE, BUFFER_SIZE);

    int indexA = (int) readHead;
    int indexB = (indexA + 1) % BUFFER_SIZE;
    t_float frac = readHead - indexA;

    t_float delayedSample = (1.0 - frac) * delay->buffer[indexA] + frac * delay->buffer[indexB];

    t_float feedbackSample = delayedSample;

    delay->hiPassFilter.filter(feedbackSample, &feedbackSample);
    delay->lowPassFilter.filter(feedbackSample, &feedbackSample);

    t_float writeSample =
        inputSample +
        feedbackSample * delay->feedbackLevel;

    if (_mode == TAPE)
        writeSample = fast_tanh(writeSample);

    delay->buffer[delay->writeHead] = writeSample;

    delay->writeHead = (delay->writeHead + 1) % BUFFER_SIZE;

    t_float lOut = delayedSample * _delayMix;

    if (panning == -1)
    {
      if (_diffusion) _diffuserL.process(lOut, lOut);
      lOut *= 0.95f;
      output[0][i] += lOut;
    }
    else if (panning == 1)
    {
      if (_diffusion) _diffuserR.process(lOut, lOut);
      lOut *= 0.95f;
      if (outChannels > 1) output[1][i] += lOut;
    }
    else if (panning == 0)
    {
      if (mode == DUAL_TAP && _diffusion)
      {
        firstTap ? _diffuserC1.process(lOut, lOut) : _diffuserC2.process(lOut, lOut);
      }
      else
      {
        if (_diffusion) _diffuserC1.process(lOut, lOut);
      }

      t_float modifier = outChannels > 1 ? 0.95f : 1.0f;
      output[0][i] += lOut * modifier;
      if (outChannels > 1) output[1][i] += lOut * modifier;
    }

    normalize(&output[0][i]);
    if (outChannels > 1)
      normalize(&output[1][i]);
  }
}

void Delay::recalculate()
{
  _circularDelays[0].bufferLength = std::clamp(int(_baseTime * _samplingFrequency), 1, BUFFER_SIZE - 2);
  _circularDelays[0].feedbackLevel = _feedbackLvl;

  _circularDelays[1].bufferLength = std::clamp(int(1.97 * _baseTime * _samplingFrequency), 1, BUFFER_SIZE - 2);
  _circularDelays[1].feedbackLevel = _feedbackLvl;

  _circularDelays[2].bufferLength = std::clamp(int(2.94 * _baseTime * _samplingFrequency), 1, BUFFER_SIZE - 2);;
  _circularDelays[2].feedbackLevel = _feedbackLvl;

  _dualTapDelays[0].bufferLength = std::clamp(int(_baseTime * _samplingFrequency), 1, BUFFER_SIZE - 2);
  _dualTapDelays[0].feedbackLevel = _feedbackLvl;

  _dualTapDelays[1].bufferLength = std::clamp(int(_baseTime2 * _samplingFrequency), 1, BUFFER_SIZE - 2);
  _dualTapDelays[1].feedbackLevel = _feedbackLvl;

  _generalDelay[0].bufferLength = std::clamp(int(_baseTime * _samplingFrequency), 1, BUFFER_SIZE - 2);
  _generalDelay[0].feedbackLevel = _feedbackLvl;

  // a grain reads up to 2x its length behind the write head
  _reverseDelay.bufferLength = std::clamp(int(_baseTime * _samplingFrequency), 2, BUFFER_SIZE / 2 - 1);
  _reverseDelay.feedbackLevel = _feedbackLvl;
}

void Delay::setHighPass(int freq)
{
  for (int i = 0; i < 3; i++)
    _circularDelays[i].hiPassFilter.setCutOff(freq, _samplingFrequency);
  for (int i = 0; i < 2; i++)
    _dualTapDelays[i].hiPassFilter.setCutOff(freq, _samplingFrequency);
  for (int i = 0; i < 1; i++)
    _generalDelay[i].hiPassFilter.setCutOff(freq, _samplingFrequency);
  _reverseDelay.hiPassFilter.setCutOff(freq, _samplingFrequency);
}

void Delay::setLowPass(int freq)
{
  for (int i = 0; i < 3; i++)
    _circularDelays[i].lowPassFilter.setCutOff(freq, _samplingFrequency);
  for (int i = 0; i < 2; i++)
    _dualTapDelays[i].lowPassFilter.setCutOff(freq, _samplingFrequency);
  for (int i = 0; i < 1; i++)
    _generalDelay[i].lowPassFilter.setCutOff(freq, _samplingFrequency);
  _reverseDelay.lowPassFilter.setCutOff(freq, _samplingFrequency);
}
