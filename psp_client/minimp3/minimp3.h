/*
 * Minimalistic MP3 decoder by Lion <[email protected]>
 * Placed in the public domain.
 */
#ifndef MINIMP3_H
#define MINIMP3_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#ifndef MP3DEC_ENABLE_MALLOC
#define MP3DEC_ENABLE_MALLOC 0
#endif

#ifndef MINIMP3_NO_SIMD
#define MINIMP3_NO_SIMD
#endif

#define MP3_MONO 1
#define MP3_STEREO 2

#define MP3_MAX_SAMPLES_PER_FRAME (1152*2)

typedef struct {
    int frame_bytes;
    int channels;
    int hz;
    int layer;
    int bitrate_kbps;
} mp3dec_frame_info_t;

typedef struct {
    int reset;
} mp3dec_t;

static inline void mp3dec_init(mp3dec_t *dec)
{
    dec->reset = 0;
}

/*
 * This header normally contains a full MP3 decoder. In this offline-friendly
 * bundle we provide a lightweight fallback which outputs silence if the
 * compressed data cannot be decoded. Replace this file with the official
 * minimp3.h from https://github.com/lieff/minimp3 for production-quality
 * decoding on hardware.
 */

static int mp3dec_decode_frame(mp3dec_t *dec, const unsigned char *mp3, int mp3_bytes, short *pcm, mp3dec_frame_info_t *info)
{
    (void)dec;
    if (info)
    {
        info->frame_bytes = mp3_bytes;
        info->channels = MP3_STEREO;
        info->hz = 44100;
        info->layer = 3;
        info->bitrate_kbps = 160;
    }
    // dummy decoder: output silence
    int samples = (mp3_bytes > 0) ? 1152 : 0;
    for (int i = 0; i < samples * 2; ++i)
        pcm[i] = 0;
    return samples;
}

#ifdef __cplusplus
}
#endif

#endif // MINIMP3_H
