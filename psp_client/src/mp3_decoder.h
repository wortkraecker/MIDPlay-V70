#ifndef PSP_CLIENT_MP3_DECODER_H
#define PSP_CLIENT_MP3_DECODER_H

#include <psptypes.h>
#include <stddef.h>

extern "C" {
#include "../minimp3/minimp3.h"
}

class Mp3Decoder {
public:
    Mp3Decoder();
    void reset();
    int decodeFrame(const unsigned char* data, int bytes, short* pcmOut, int* samples); // samples is per channel count

private:
    mp3dec_t decoder;
};

#endif
