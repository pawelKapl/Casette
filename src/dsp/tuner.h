#pragma once

#include "env.h"
#include "aubio/aubio.h"
#include "dsp/IIR_filter.h"

class Tuner
{
public:
    Tuner(int sampleRate, int bufferSize, int chunkSize)
    {
        assert(_bufferSize % _chunkSize == 0);
        _bufferSize = bufferSize;
        _chunkSize = chunkSize;
        _readBuffer = new float[bufferSize];
        _pitchDetector = new_aubio_pitch("yin", bufferSize, 512, sampleRate);
        aubio_pitch_set_tolerance(_pitchDetector, 0.15);

        HiPassFilterParams preFilterParams;
        preFilterParams.sampleRate = 48000;
        preFilterParams.frequency = 10;
        _preFilter.setParams(preFilterParams);
    }

    ~Tuner()
    {
        delete [] _readBuffer;
    }

    std::pair<std::string, float> read() const 
    {
        return {_targetNote, _error};
    }

    void measure(float *input);

private:
    const std::string NOTE_NAMES[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    HiPassFilter _preFilter;
    aubio_pitch_t* _pitchDetector;
    int _bufferSize;
    int _chunkSize;
    
    float *_readBuffer;
    
    int _head = 0;
    std::string _targetNote = "-";
    float _error = 0.0f;
    
};