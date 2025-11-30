#include "mp3_decoder.h"

Mp3Decoder::Mp3Decoder() { }

bool Mp3Decoder::init() {
    mp3dec_init(&dec);
    return true;
}

void Mp3Decoder::reset() {
    mp3dec_init(&dec);
}

int Mp3Decoder::decodeFrame(const u8* data, size_t len, std::vector<int16_t>& pcmOut, int& sampleRate, int& channels) {
    mp3dec_frame_info_t info{};
    pcmOut.resize(1152 * 2);
    int samples = mp3dec_decode_frame(&dec, data, len, pcmOut.data(), &info);
    sampleRate = info.hz > 0 ? info.hz : SAMPLE_RATE;
    channels = info.channels > 0 ? info.channels : CHANNELS;
    pcmOut.resize(samples * channels);
    return samples;
}
