#include "delay.h"

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

  currentPtr = _diffuserL.init(currentPtr, _samplingFrequency);
  currentPtr = _diffuserR.init(currentPtr, _samplingFrequency);
  currentPtr = _diffuserC1.init(currentPtr, _samplingFrequency);
  currentPtr = _diffuserC2.init(currentPtr, _samplingFrequency);

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
  }
}

void Delay::singleDelay(const t_float* const* input, t_float** output, DelayMode mode, int frames, int delayIndex, int panning, int inChannels, int outChannels)
{
  static bool firstTap = false;
  SingleDelay* delay;
  switch (_mode)
  {
  case CIRCULAR:
    delay = &_circularDelays[delayIndex];
    break;
  case DUAL_TAP:
    delay = &_dualTapDelays[delayIndex];
    firstTap = !firstTap;
    break;
  case MONO:
  case TAPE:
    delay = &_generalDelay[delayIndex];
    break;
  case PING_PONG:
    delay = &_circularDelays[delayIndex];
    break;
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
    normalize(&output[1][i]);
  }
}

void Delay::recalculate()
{
  int bufferLength = _baseTime * _samplingFrequency;
  _circularDelays[0].bufferLength = bufferLength;
  _circularDelays[0].feedbackLevel = _feedbackLvl;

  bufferLength = 2 * _baseTime * _samplingFrequency;
  _circularDelays[1].bufferLength = bufferLength;
  _circularDelays[1].feedbackLevel = _feedbackLvl;

  bufferLength = 3 * _baseTime * _samplingFrequency;
  _circularDelays[2].bufferLength = bufferLength;
  _circularDelays[2].feedbackLevel = _feedbackLvl;

  _dualTapDelays[0].bufferLength = _baseTime * _samplingFrequency;
  _dualTapDelays[0].feedbackLevel = _feedbackLvl;

  _dualTapDelays[1].bufferLength = _baseTime2 * _samplingFrequency;
  _dualTapDelays[1].feedbackLevel = _feedbackLvl;

  _generalDelay[0].bufferLength = _baseTime * _samplingFrequency;
  _generalDelay[0].feedbackLevel = _feedbackLvl;
}

void Delay::setHighPass(int freq)
{
  for (int i = 0; i < 3; i++)
    _circularDelays[i].hiPassFilter.setCutOff(freq, _samplingFrequency);
  for (int i = 0; i < 2; i++)
    _dualTapDelays[i].hiPassFilter.setCutOff(freq, _samplingFrequency);
  for (int i = 0; i < 1; i++)
    _generalDelay[i].hiPassFilter.setCutOff(freq, _samplingFrequency);
}

void Delay::setLowPass(int freq)
{
  for (int i = 0; i < 3; i++)
    _circularDelays[i].lowPassFilter.setCutOff(freq, _samplingFrequency);
  for (int i = 0; i < 2; i++)
    _dualTapDelays[i].lowPassFilter.setCutOff(freq, _samplingFrequency);
  for (int i = 0; i < 1; i++)
    _generalDelay[i].lowPassFilter.setCutOff(freq, _samplingFrequency);
}
