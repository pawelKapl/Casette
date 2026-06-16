#include "env.h"
#include <csignal>
#include "platform/gpio_button.h"
#include "ui/gui.h"
#include "platform/client.h"
#include "effects_chain.h"
#include "dsp/cab_simulator.h"
#include "dsp/noise_gate.h"
#include "dsp/tuner.h"
#include "dsp/amp.h"
#include "state.h"

int setup();

struct PreInit
{
    PreInit()
    {
        State::get().activePreset = Storage::get().loadActivePreset();
        auto fsAssignements = Storage::get().loadFsAssignements();
        State::get().leftFsAssignement = fsAssignements[0];
        State::get().midFsAssignement = fsAssignements[1];
        State::get().rightFsAssignement = fsAssignements[2];
    }
} preinit;

const char *GPIO_CHIP = "gpiochip0";
gpiod_chip *chip;

std::array<std::unique_ptr<GPIOButton>, 3> footswitches;

std::atomic<bool> running{true};
std::atomic<bool> bypass{false};
AudioClient client;

CabSimulator cabSim;
Amp amp{&cabSim};
NoiseGate noiseGate;
FxChain fxChain;
Tuner tuner{48000, 4096, 64};

GUI gui(&fxChain, &cabSim, &amp, &noiseGate, &tuner, &footswitches);

void signalHandler(int signum)
{
    running = false;
}

int audio_callback(j_int nframes, void *arg)
{
    if (!running)
        return 0;

    auto *inL = client.getInBuffer(nframes, true);
    auto *outL = client.getOutBuffer(nframes, true);
    auto *outR = client.getOutBuffer(nframes, false);

    if (bypass)
    {
        tuner.measure(inL);

        for (int i = 0; i < nframes; i++)
        {
            outL[i] = inL[i];
            outR[i] = inL[i];
        }
    }
    else
    {
        amp.process(inL, outL, nframes);

        if (noiseGate.isEnabled())
        {
            noiseGate.process(outL, outL, nframes);
        }

        std::memcpy(outR, outL, nframes * sizeof(float));

        float *in[2] = {outL, outR};
        float *out[2] = {outL, outR};
        fxChain.process(in, out, nframes, 2, 2);
    }

    return 0;
}

void programLoop()
{
    gui.update();
    usleep(5000);
}

int main()
{
    if (setup())
        return -1;

    while (running)
    {
        programLoop();
        if (!bypass)
        {
            for (int i = 0; i < 3; i++)
                footswitches[i]->debounce();
        }
        else
        {
            footswitches[2]->debounce(true);
        }
    }

    log_info << "Exit...";

    return 0;
}

int setup()
{
    std::signal(SIGINT, signalHandler);
    log_info << "Start!";

    if (client.init(audio_callback))
        return -1;

    log_info << "JACK client running...";

    chip = gpiod_chip_open_by_name(GPIO_CHIP);

    if (!chip)
    {
        log_error << "Failed to open GPIO chip\n";
    }

    footswitches[0] = std::make_unique<GPIOButton>(chip, 14);
    footswitches[0]->setShortPressCallback([=]()
                                           { State::get().changePreset(State::get().leftFsAssignement); Storage::get().persistActivePreset(); });
    footswitches[1] = std::make_unique<GPIOButton>(chip, 15);
    footswitches[1]->setShortPressCallback([=]()
                                           { State::get().changePreset(State::get().midFsAssignement); Storage::get().persistActivePreset(); });
    footswitches[2] = std::make_unique<GPIOButton>(chip, 23);
    footswitches[2]->setShortPressCallback([=]()
                                           { State::get().changePreset(State::get().rightFsAssignement); Storage::get().persistActivePreset(); });
    footswitches[2]->setLongPressCallback([&]()
                                          { bypass = !bypass; gui.bypassMode(bypass); });

    amp.reload();
    gui.init();

    return 0;
}