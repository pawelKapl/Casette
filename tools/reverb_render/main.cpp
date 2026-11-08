#include "../common/render.h"
#include "dsp/reverb/reverb_settings.h"

static const char *USAGE =
    "usage: reverb_render <in.wav | impulse> <out.wav> [key=value ...]\n"
    "  key=value  any reverb.json field (reverbMode, mix, decay, roomSize, damping,\n"
    "             preDelay, hiPass, loPass), same units as the UI stores\n"
    "  preset=    stored preset to start from, e.g. preset=storage/default/reverb.json\n"
    "  tail=      seconds of silence appended so the tail can ring out (default 6)\n"
    "  at=        change a value mid-render like the UI does, e.g. at=2.5:reverbMode=0\n";

// same size as DELAY_MEMORY, which the reverb shares with the delay on the pedal
static const size_t MEMORY_SIZE = 4194304;

int main(int argc, char **argv)
{
  return guardedMain([&]()
  {
    RenderArgs<ReverbSettings> args;
    args.tail = 6.0f;
    if (!parseArgs(argc, argv, USAGE, "reverb", args))
      return 1;
    Audio &audio = args.audio;
    size_t total = audio.left.size();

    std::vector<float> memory(MEMORY_SIZE);
    Reverb reverb(audio.sampleRate);
    size_t memoryUsed = reverb.init(memory.data()) - memory.data();
    if (memoryUsed > MEMORY_SIZE)
    {
      std::fprintf(stderr, "reverb needs %zu floats of memory, only %zu available\n", memoryUsed, MEMORY_SIZE);
      return 1;
    }
    applyReverbSettings(reverb, args.settings);
    flushDenormals();

    auto start = std::chrono::steady_clock::now();
    for (size_t pos = 0; pos < total; pos += BLOCK_SIZE)
    {
      applyChanges(args, pos, [&](int param) { applyReverbParam(reverb, args.settings, param); });

      int frames = (int)std::min<size_t>(BLOCK_SIZE, total - pos);
      float *channels[2] = {audio.left.data() + pos, audio.right.data() + pos};
      reverb.process(channels, channels, frames, 2, 2);
    }
    double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

    if (!writeWav(argv[2], audio))
    {
      std::fprintf(stderr, "can't write %s\n", argv[2]);
      return 1;
    }

    bool finite = report(argv[2], audio, elapsed);
    std::printf("memory: %zu floats (%.1f MB) of %zu\n", memoryUsed, memoryUsed * 4 / 1048576.0, MEMORY_SIZE);
    return finite ? 0 : 2;
  });
}
