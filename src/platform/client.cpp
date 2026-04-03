#include "client.h"

AudioClient::~AudioClient()
{
    log_info << "Shutting down JACK client...";
    jack_deactivate(client);
}

int AudioClient::init(int (*JackProcessCallback)(j_int nframes, void *arg))
{
    client = jack_client_open("casette", JackNoStartServer, NULL);
    if (client == NULL)
    {
        log_error << "Failed to create JACK client";
        return -1;
    }

    in_l = jack_port_register(client, "input_l", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    // in_r = jack_port_register(client, "input_r", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    out_l = jack_port_register(client, "output_l", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    out_r = jack_port_register(client, "output_r", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    if (in_l == NULL || out_l == NULL || out_r == NULL)
    {
        log_error << "Failed to register JACK ports";
        return -1;
    }

    jack_set_process_callback(client, JackProcessCallback, NULL);

    if (jack_activate(client))
    {
        log_error << "Failed to activate JACK client";
        return -1;
    }

    if (jack_connect(client, "system:capture_1", jack_port_name(in_l)) != 0)
    {
        log_error << "Failed to connect to system capture port 1!";
        return -1;
    }

    // if (jack_connect(client, "system:capture_2", jack_port_name(in_r)) != 0)
    // {
    //     log_error << "Failed to connect to system capture port 2!";
    //     return -1;
    // }

    if (jack_connect(client, jack_port_name(out_l), "system:playback_1") != 0)
    {
        log_error << "Failed to connect to system playback port 1!";
        return -1;
    }

    if (jack_connect(client, jack_port_name(out_r), "system:playback_2") != 0)
    {
        log_error << "Failed to connect to system playback port 2!";
        return -1;
    }

    return 0;
}