#include "../common/render.h"
#include "dsp/octaver/octaver_settings.h"

static const char *USAGE =
    "usage: octaver_render <in.wav | impulse> <out.wav> [key=value ...]\n"
    "  key=value  any octaver.json field (mode, dry, sub2, sub1, up1, up2, tone),\n"
    "             same units as the UI stores, input is the dry guitar (left channel)\n"
    "  preset=    stored preset to start from, e.g. preset=storage/default/octaver.json\n"
    "  tail=      seconds of silence appended (default 1)\n"
    "  at=        change a value mid-render like the UI does, e.g. at=2.5:mode=1\n";

int main(int argc, char **argv)
{
  return guardedMain([&]()
  {
    RenderArgs<OctaverSettings> args;
    args.tail = 1.0f;
    // renders the effect unless asked otherwise, enabled=0 / at=..:enabled=.. still work
    args.settings.enabled = true;
    if (!parseArgs(argc, argv, USAGE, "octaver", args))
      return 1;
    Audio &audio = args.audio;
    size_t total = audio.left.size();

    Octaver octaver(audio.sampleRate);
    applyOctaverSettings(octaver, args.settings);
    octaver.settle();
    flushDenormals();

    auto start = std::chrono::steady_clock::now();
    for (size_t pos = 0; pos < total; pos += BLOCK_SIZE)
    {
      applyChanges(args, pos, [&](int param) { applyOctaverParam(octaver, args.settings, param); });

      int frames = (int)std::min<size_t>(BLOCK_SIZE, total - pos);
      octaver.process(audio.left.data() + pos, audio.left.data() + pos, frames);
    }
    double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    audio.right = audio.left;

    if (!writeWav(argv[2], audio))
    {
      std::fprintf(stderr, "can't write %s\n", argv[2]);
      return 1;
    }

    return report(argv[2], audio, elapsed) ? 0 : 2;
  });
}
