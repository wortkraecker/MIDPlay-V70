#include "mp3_decoder.h"
#include "utils.h"

Mp3Decoder::Mp3Decoder() {
    mp3dec_init(&decoder);
}

void Mp3Decoder::reset() {
    mp3dec_init(&decoder);
}

int Mp3Decoder::decodeFrame(const unsigned char* data, int bytes, short* pcmOut, int* samples) {
    mp3dec_frame_info_t info;
    int sampleCount = mp3dec_decode_frame(&decoder, data, bytes, pcmOut, &info);
    if (sampleCount <= 0) {
        *samples = 0;
        return info.frame_bytes;
    }
    *samples = sampleCount;
    return info.frame_bytes;
}
