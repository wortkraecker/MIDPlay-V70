#ifndef PSP_CLIENT_AUDIO_H
#define PSP_CLIENT_AUDIO_H

#include <psptypes.h>

namespace psp_audio {

static const int BUFFER_COUNT = 4;
static const int BUFFER_SAMPLES = 4096; // per channel

int initialize();
void shutdown();

// push PCM samples (stereo interleaved) into ring buffer
void enqueue(const short* pcm, int samples);
void mixAndPlay();

}

#endif
