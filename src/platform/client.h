#pragma once

#include "env.h"
#include <jack/jack.h>

class AudioClient
{
public:
    ~AudioClient();

    int init(int (*JackProcessCallback)(j_int nframes, void *arg));

    jack_default_audio_sample_t *getInBuffer(j_int nframes, bool left)
    {
        if (left)
            return (jack_default_audio_sample_t *)jack_port_get_buffer(in_l, nframes);
        else
            return (jack_default_audio_sample_t *)jack_port_get_buffer(in_r, nframes);
    }

    jack_default_audio_sample_t *getOutBuffer(j_int nframes, bool left)
    {
        if (left)
            return (jack_default_audio_sample_t *)jack_port_get_buffer(out_l, nframes);
        else
            return (jack_default_audio_sample_t *)jack_port_get_buffer(out_r, nframes);
    }

private:
    jack_port_t* out_l;
    jack_port_t* out_r;
    jack_port_t* in_l;
    jack_port_t* in_r;
    jack_client_t *client;
};