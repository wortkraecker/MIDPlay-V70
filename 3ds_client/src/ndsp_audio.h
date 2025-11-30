#pragma once
#include <3ds.h>
#include <vector>
#include <array>
#include "utils.h"

class NdspAudio {
public:
    NdspAudio();
    bool init();
    void shutdown();
    bool queueSamples(const int16_t* data, size_t samples, int sampleRate, int channels);
    void update();
private:
    std::array<ndspWaveBuf, PCM_RING_BUFFERS> waveBufs{};
    std::array<std::vector<int16_t>, PCM_RING_BUFFERS> buffers;
    int current;
};
