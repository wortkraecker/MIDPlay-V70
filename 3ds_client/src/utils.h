#pragma once
#include <3ds.h>
#include <string>

// Update with your VPS address
static constexpr const char* SERVER_IP = "192.168.1.10";
static constexpr u16 SERVER_PORT = 4000;

// Audio + streaming constants
static constexpr size_t STREAM_BUFFER_SIZE = 8192;    // bytes to pull per socket read
static constexpr size_t PCM_BUFFER_SAMPLES = 4096;    // NDSP nsamples per buffer
static constexpr size_t PCM_RING_BUFFERS  = 4;
static constexpr int    SAMPLE_RATE       = 44100;
static constexpr int    CHANNELS          = 2;

inline u64 now_ms() { return osGetTime(); }
inline void sleep_ms(u32 ms) { svcSleepThread((s64)ms * 1000000LL); }

inline std::string http_host() { return std::string(SERVER_IP); }
inline std::string http_base() { return "http://" + http_host() + ":" + std::to_string(SERVER_PORT); }

inline int clamp_int(int v, int minv, int maxv) {
    if (v < minv) return minv;
    if (v > maxv) return maxv;
    return v;
}
