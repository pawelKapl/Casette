#pragma once

#include "dsp_math.h"

enum FilterType
{
    LOW_SHELF, PEAKING, HIGH_SHELF, HI_PASS
};

class BaseIIRFilter
{
public:
  BaseIIRFilter(const int inputDegree, const int outputDegree)
  {
    _inputCoefficientsSize = inputDegree;
    _inputCoefficients = std::vector<float>(inputDegree, 0);
    _outputCoefficientsSize = outputDegree;
    _outputCoefficients = std::vector<float>(outputDegree, 0);
    _inputHistory = std::vector<float>(inputDegree - 1, 0);
    _outputHistory = std::vector<float>(outputDegree - 1, 0);
  }

  void process(float* inputs, float* outputs, const size_t numFrames);
  float processSample(float input);

protected:

  std::vector<float> _inputCoefficients;
  std::vector<float> _outputCoefficients;
  std::vector<float> _inputHistory;
  std::vector<float> _outputHistory;

  long _inputStart = 0;
  long _iutputStart = 0;

  int _inputCoefficientsSize;
  int _outputCoefficientsSize;

}; // BaseIIRFilter

struct BiQuadFilterParams
{
    FilterType type;
    float sampleRate;
    float frequency;
    float quality;
    float gainDB;
};

class BiQuadFilter : public BaseIIRFilter
{
public:
    BiQuadFilter() : BaseIIRFilter(3, 3) {}

    void setGain(float gainDB);
    void setParams(const BiQuadFilterParams &params);

private:
    BiQuadFilterParams _params;
    

}; // BiQuadFilter

struct HiPassFilterParams
{
    float sampleRate;
    float frequency;
};

class HiPassFilter : public BaseIIRFilter
{
public:
  HiPassFilter() : BaseIIRFilter(2, 2) {}
  void setParams(const HiPassFilterParams &params);

private:
    HiPassFilterParams _params;

}; // HiPassFilter