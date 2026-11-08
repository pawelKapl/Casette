#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#if defined(__x86_64__) || defined(__i386__)
#include <xmmintrin.h>
#endif

#include "json.hpp"

// JACK period on the pedal
static const int BLOCK_SIZE = 64;

struct Audio
{
  int sampleRate = 48000;
  std::vector<float> left;
  std::vector<float> right;
};

struct Change
{
  size_t frame;
  std::string key;
  int value;
};

template <typename Settings>
struct RenderArgs
{
  Audio audio;
  Settings settings;
  std::vector<Change> changes;
  float tail = 0.0f;
};

template <typename T>
static T readLE(const uint8_t *p)
{
  T value;
  std::memcpy(&value, p, sizeof(T));
  return value;
}

static bool readWav(const std::string &path, Audio &audio)
{
  std::ifstream file(path, std::ios::binary);
  if (!file)
    return false;

  std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  if (data.size() < 12 || std::memcmp(data.data(), "RIFF", 4) || std::memcmp(data.data() + 8, "WAVE", 4))
    return false;

  int format = 0, channels = 0, bits = 0;
  const uint8_t *samples = nullptr;
  size_t sampleBytes = 0;

  for (size_t pos = 12; pos + 8 <= data.size();)
  {
    size_t size = std::min<size_t>(readLE<uint32_t>(&data[pos + 4]), data.size() - pos - 8);
    const uint8_t *chunk = &data[pos + 8];

    if (!std::memcmp(&data[pos], "fmt ", 4) && size >= 16)
    {
      format = readLE<uint16_t>(chunk);
      channels = readLE<uint16_t>(chunk + 2);
      audio.sampleRate = readLE<uint32_t>(chunk + 4);
      bits = readLE<uint16_t>(chunk + 14);
      // WAVE_FORMAT_EXTENSIBLE keeps the real format in the sub-format GUID
      if (format == 0xFFFE && size >= 26)
        format = readLE<uint16_t>(chunk + 24);
    }
    else if (!std::memcmp(&data[pos], "data", 4))
    {
      samples = chunk;
      sampleBytes = size;
    }

    pos += 8 + size + (size & 1);
  }

  bool pcm = format == 1 && (bits == 16 || bits == 24 || bits == 32);
  bool ieee = format == 3 && bits == 32;
  if (!samples || channels < 1 || !(pcm || ieee))
  {
    std::fprintf(stderr, "unsupported wav (format %d, %d bit) - use 16/24/32 bit PCM or 32 bit float\n", format, bits);
    return false;
  }

  int bytes = bits / 8;
  size_t frames = sampleBytes / (bytes * channels);
  for (size_t f = 0; f < frames; f++)
  {
    for (int c = 0; c < std::min(channels, 2); c++)
    {
      const uint8_t *p = samples + (f * channels + c) * bytes;
      float value;
      if (ieee)
        value = readLE<float>(p);
      else if (bits == 16)
        value = readLE<int16_t>(p) / 32768.0f;
      else if (bits == 24)
        value = (int32_t)((uint32_t)p[0] << 8 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 24) / 2147483648.0f;
      else
        value = readLE<int32_t>(p) / 2147483648.0f;

      (c == 0 ? audio.left : audio.right).push_back(value);
    }
  }

  if (channels == 1)
    audio.right = audio.left;

  return true;
}

static bool writeWav(const std::string &path, const Audio &audio)
{
  std::ofstream file(path, std::ios::binary);
  if (!file)
    return false;

  std::vector<float> interleaved(audio.left.size() * 2);
  for (size_t i = 0; i < audio.left.size(); i++)
  {
    interleaved[i * 2] = audio.left[i];
    interleaved[i * 2 + 1] = audio.right[i];
  }

  uint32_t dataBytes = interleaved.size() * sizeof(float);
  auto u32 = [&](uint32_t v) { file.write((const char *)&v, 4); };
  auto u16 = [&](uint16_t v) { file.write((const char *)&v, 2); };

  // 32 bit float stereo
  file.write("RIFF", 4);
  u32(36 + dataBytes);
  file.write("WAVE", 4);
  file.write("fmt ", 4);
  u32(16);
  u16(3);
  u16(2);
  u32(audio.sampleRate);
  u32(audio.sampleRate * 2 * sizeof(float));
  u16(2 * sizeof(float));
  u16(32);
  file.write("data", 4);
  u32(dataBytes);
  file.write((const char *)interleaved.data(), dataBytes);

  return (bool)file;
}

template <typename Settings>
static bool setValue(Settings &settings, const std::string &key, int value)
{
  nlohmann::json j = settings;
  if (!j.contains(key))
  {
    std::fprintf(stderr, "unknown key: %s\n", key.c_str());
    return false;
  }
  j[key] = value;
  settings = j.get<Settings>();
  return true;
}

// <in.wav | impulse> <out.wav> [key=value | preset= | tail= | at=seconds:key=value ...]
template <typename Settings>
static bool parseArgs(int argc, char **argv, const char *usage, const char *presetKey, RenderArgs<Settings> &args)
{
  if (argc < 3)
  {
    std::fputs(usage, stderr);
    return false;
  }

  std::string inPath = argv[1];
  if (inPath == "impulse")
  {
    // single full scale sample 100 ms in
    args.audio.left.assign(args.audio.sampleRate / 10 + 1, 0.0f);
    args.audio.left.back() = 1.0f;
    args.audio.right = args.audio.left;
  }
  else if (!readWav(inPath, args.audio))
  {
    std::fprintf(stderr, "can't read %s\n", inPath.c_str());
    return false;
  }

  for (int i = 3; i < argc; i++)
  {
    std::string arg = argv[i];
    size_t eq = arg.find('=');
    if (eq == std::string::npos)
    {
      std::fputs(usage, stderr);
      return false;
    }

    std::string key = arg.substr(0, eq);
    std::string value = arg.substr(eq + 1);
    if (key == "preset")
    {
      std::ifstream file(value);
      if (!file)
      {
        std::fprintf(stderr, "can't read %s\n", value.c_str());
        return false;
      }
      nlohmann::json j;
      file >> j;
      args.settings = j.at(presetKey).get<Settings>();
    }
    else if (key == "tail")
    {
      args.tail = std::stof(value);
    }
    else if (key == "at")
    {
      size_t colon = value.find(':');
      size_t valueEq = value.find('=');
      if (colon == std::string::npos || valueEq == std::string::npos || valueEq < colon)
      {
        std::fputs(usage, stderr);
        return false;
      }
      std::string changeKey = value.substr(colon + 1, valueEq - colon - 1);
      Settings probe;
      if (!setValue(probe, changeKey, 0))
        return false;
      args.changes.push_back({size_t(std::stof(value.substr(0, colon)) * args.audio.sampleRate), changeKey, std::stoi(value.substr(valueEq + 1))});
    }
    else if (!setValue(args.settings, key, std::stoi(value)))
    {
      return false;
    }
  }

  size_t total = args.audio.left.size() + size_t(args.tail * args.audio.sampleRate);
  args.audio.left.resize(total, 0.0f);
  args.audio.right.resize(total, 0.0f);
  return true;
}

// like the UI: only the params whose value changed get applied
template <typename Settings, typename Apply>
static void applyChanges(RenderArgs<Settings> &args, size_t pos, Apply &&apply)
{
  for (const Change &change : args.changes)
  {
    if (change.frame < pos || change.frame >= pos + BLOCK_SIZE)
      continue;
    Settings before = args.settings;
    setValue(args.settings, change.key, change.value);
    for (int param = 0; param < Settings::paramCount; param++)
      if (*before.getParam(param) != *args.settings.getParam(param))
        apply(param);
  }
}

static void flushDenormals()
{
#if defined(__x86_64__) || defined(__i386__)
  // denormals in decaying tails are very slow on x86, the pi build measures without this
  _mm_setcsr(_mm_getcsr() | 0x8040);
#endif
}

static bool report(const char *path, const Audio &audio, double elapsed)
{
  size_t total = audio.left.size();
  float peak = 0.0f;
  bool finite = true;
  for (size_t i = 0; i < total; i++)
  {
    finite = finite && std::isfinite(audio.left[i]) && std::isfinite(audio.right[i]);
    peak = std::max({peak, std::abs(audio.left[i]), std::abs(audio.right[i])});
  }

  double audioSeconds = double(total) / audio.sampleRate;
  std::printf("%s: %.1f s, peak %.1f dBFS%s\n", path, audioSeconds, 20.0 * std::log10(std::max(peak, 1e-9f)),
              finite ? "" : ", NaN/Inf in output!");
  std::printf("cpu: %.2f%% of one core in real time (%.1f ns/frame)\n", 100.0 * elapsed / audioSeconds, 1e9 * elapsed / total);
  return finite;
}

template <typename Run>
static int guardedMain(Run &&run)
{
  try
  {
    return run();
  }
  catch (const std::exception &e)
  {
    std::fprintf(stderr, "error: %s\n", e.what());
    return 1;
  }
}
