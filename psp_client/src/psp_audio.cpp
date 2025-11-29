#include "psp_audio.h"
#include "utils.h"

#include <pspaudio.h>
#include <pspaudiolib.h>
#include <string.h>

namespace psp_audio {

static short buffers[BUFFER_COUNT][BUFFER_SAMPLES * 2];
static volatile int writeIndex = 0;
static volatile int readIndex = 0;
static int audioChannel = -1;

int initialize() {
    pspAudioInit();
    audioChannel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, BUFFER_SAMPLES, PSP_AUDIO_FORMAT_STEREO);
    if (audioChannel < 0) {
        LOG_ERROR("Audio reserve failed: %d", audioChannel);
        return audioChannel;
    }
    memset(buffers, 0, sizeof(buffers));
    writeIndex = 0;
    readIndex = 0;
    return 0;
}

void shutdown() {
    if (audioChannel >= 0) {
        sceAudioChRelease(audioChannel);
        audioChannel = -1;
    }
    pspAudioEnd();
}

void enqueue(const short* pcm, int samples) {
    if (samples <= 0) return;
    int next = (writeIndex + 1) % BUFFER_COUNT;
    if (next == readIndex) {
        // buffer full, drop oldest
        readIndex = (readIndex + 1) % BUFFER_COUNT;
    }
    int toCopy = samples * 2;
    if (toCopy > BUFFER_SAMPLES * 2) toCopy = BUFFER_SAMPLES * 2;
    memcpy(buffers[writeIndex], pcm, toCopy * sizeof(short));
    if (toCopy < BUFFER_SAMPLES * 2) {
        memset(buffers[writeIndex] + toCopy, 0, (BUFFER_SAMPLES * 2 - toCopy) * sizeof(short));
    }
    writeIndex = next;
}

void mixAndPlay() {
    if (audioChannel < 0) return;
    if (readIndex == writeIndex) {
        short silence[BUFFER_SAMPLES * 2];
        memset(silence, 0, sizeof(silence));
        sceAudioOutputPannedBlocking(audioChannel, 0x8000, 0x8000, silence);
        return;
    }
    sceAudioOutputPannedBlocking(audioChannel, 0x8000, 0x8000, buffers[readIndex]);
    readIndex = (readIndex + 1) % BUFFER_COUNT;
}

}
