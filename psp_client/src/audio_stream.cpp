#include "audio_stream.h"
#include "utils.h"

#include <string.h>

AudioStream::AudioStream(const char* host, int port)
    : m_port(port), client(host, port), running(false) {
    strncpy(m_host, host, sizeof(m_host));
    m_host[sizeof(m_host) - 1] = '\0';
}

void AudioStream::start() {
    if (running) return;
    running = true;
    decoder.reset();
    client.closeStream();
    client.openStream("/stream.mp3");
}

void AudioStream::stop() {
    running = false;
    client.closeStream();
}

void AudioStream::update() {
    if (!running) return;
    if (!client.isStreamOpen()) {
        LOG("Reconnecting stream...");
        client.openStream("/stream.mp3");
        decoder.reset();
        return;
    }

    int readBytes = client.readStream(streamBuffer, sizeof(streamBuffer));
    if (readBytes <= 0) {
        LOG_ERROR("Stream lost, reconnecting");
        client.closeStream();
        return;
    }
    int offset = 0;
    while (offset < readBytes) {
        short pcm[MP3_MAX_SAMPLES_PER_FRAME];
        int samples = 0;
        int used = decoder.decodeFrame(streamBuffer + offset, readBytes - offset, pcm, &samples);
        if (used <= 0) break;
        offset += used;
        if (samples > 0) {
            psp_audio::enqueue(pcm, samples);
        }
    }
}

