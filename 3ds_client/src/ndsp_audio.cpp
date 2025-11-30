#include "ndsp_audio.h"
#include <cstring>

NdspAudio::NdspAudio() : current(0) {}

bool NdspAudio::init() {
    if (ndspInit() != 0) return false;
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnReset(0);
    ndspChnSetInterp(0, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(0, SAMPLE_RATE);
    ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);

    for (size_t i = 0; i < PCM_RING_BUFFERS; ++i) {
        buffers[i].resize(PCM_BUFFER_SAMPLES * CHANNELS);
        memset(&waveBufs[i], 0, sizeof(ndspWaveBuf));
        waveBufs[i].data_vaddr = buffers[i].data();
        waveBufs[i].nsamples = PCM_BUFFER_SAMPLES;
        waveBufs[i].looping = false;
    }
    return true;
}

void NdspAudio::shutdown() {
    ndspChnWaveBufClear(0);
    ndspExit();
}

bool NdspAudio::queueSamples(const int16_t* data, size_t samples, int sampleRate, int channels) {
    if (channels != 2) return false; // expect stereo
    // find a completed buffer slot
    for (size_t tries = 0; tries < PCM_RING_BUFFERS; ++tries) {
        ndspWaveBuf* wb = &waveBufs[current];
        if (wb->status != NDSP_WBUF_DONE && wb->status != 0) {
            current = (current + 1) % PCM_RING_BUFFERS;
            continue;
        }
        size_t copySamples = samples > PCM_BUFFER_SAMPLES ? PCM_BUFFER_SAMPLES : samples;
        memcpy(buffers[current].data(), data, copySamples * sizeof(int16_t) * channels);
        wb->data_vaddr = buffers[current].data();
        wb->nsamples = copySamples;
        wb->looping = false;
        ndspChnSetRate(0, sampleRate);
        DSP_FlushDataCache(buffers[current].data(), copySamples * sizeof(int16_t) * channels);
        ndspChnWaveBufAdd(0, wb);
        current = (current + 1) % PCM_RING_BUFFERS;
        return true;
    }
    return false;
}

void NdspAudio::update() {
    // polling wave buffer status keeps NDSP moving; nothing else required here
}
