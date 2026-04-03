#include "dsp/tuner.h"

void Tuner::measure(float *in)
{
    for (int i = 0; i < _chunkSize; i++)
    {
        _readBuffer[_head] = _preFilter.processSample(in[i]);
        _head++;
    }

    if (_head >= _bufferSize)
    {
        _head = 0;

        fvec_t* output = new_fvec(1); 
        fvec_t* input = new_fvec(_bufferSize);
        input->data = _readBuffer;
        aubio_pitch_do(_pitchDetector, input, output);
    
        float freq = output->data[0];
    
        if (freq == 0)
        {
            _targetNote = "-";
            _error = 0.0f;
            return;
        }
    
        int midiNote = std::round(69 + 12 * std::log2(freq / 440.0));
        _targetNote = NOTE_NAMES[midiNote % 12];
        
        float targetFreq = 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
        float centOffset = 1200.0f * std::log2(freq / targetFreq);
        _error = (centOffset / 100.0f) * 100.0f;
    }
}