#pragma once
#include <3ds.h>
#include <vector>
#include "minimp3/minimp3.h"

class Mp3Decoder {
public:
    Mp3Decoder();
    bool init();
    void reset();
    int decodeFrame(const u8* data, size_t len, std::vector<int16_t>& pcmOut, int& sampleRate, int& channels);
private:
    mp3dec_t dec;
};
