#pragma once

#include "env.h"
#include <sndfile.h>

class Wav
{
public:
    Wav() {}
    Wav(const std::string &filepath)
    {
        SF_INFO sfinfo;
        SNDFILE* file = sf_open(filepath.c_str(), SFM_READ, &sfinfo);
        if (!file) {
            throw std::runtime_error("Failed to open WAV file: " + filepath);
        }

        _samplerate = sfinfo.samplerate;
        _data = std::vector<float>(sfinfo.frames);
        sf_readf_float(file, _data.data(), sfinfo.frames);
        sf_close(file);
    }

    inline float* data() { return _data.data(); }
    inline size_t size() { return _data.size(); }

private:
    std::vector<float> _data = {};
    int _samplerate = 0;

};