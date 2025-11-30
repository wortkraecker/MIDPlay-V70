#include "audio_stream.h"
#include <cstring>
#include <cstdint>

AudioStream::AudioStream()
    : http(nullptr), audioOut(nullptr), streamSock(-1), streaming(false), exitThread(false), thread(nullptr), lastRead(0) {}

bool AudioStream::init(HttpClient* client, NdspAudio* audio) {
    http = client;
    audioOut = audio;
    decoder.init();
    netBuf.resize(STREAM_BUFFER_SIZE);
    pcmBuf.resize(1152 * CHANNELS);
    return true;
}

void AudioStream::start() {
    if (thread) return;
    exitThread = false;
    streaming = false;
    streamSock = -1;
    lastRead = now_ms();
    thread = threadCreate(AudioStream::threadMain, this, 32 * 1024, 0x2F, -2, false);
}

void AudioStream::stop() {
    exitThread = true;
    if (thread) {
        threadJoin(thread, UINT64_MAX);
        threadFree(thread);
        thread = nullptr;
    }
    if (streamSock >= 0) http->closeStream(streamSock);
    streamSock = -1;
    streaming = false;
}

void AudioStream::reconnect() {
    if (streamSock >= 0) http->closeStream(streamSock);
    streamSock = http->openStream("/stream.mp3");
    decoder.reset();
    streaming = streamSock >= 0;
    lastRead = now_ms();
}

void AudioStream::update() {
    if (!streaming && (now_ms() - lastRead) > 1500) {
        reconnect();
    }
}

void AudioStream::threadMain(void* arg) {
    static_cast<AudioStream*>(arg)->run();
}

void AudioStream::run() {
    reconnect();
    while (!exitThread) {
        if (streamSock < 0) {
            sleep_ms(200);
            continue;
        }
        int read = http->readStream(streamSock, netBuf.data(), netBuf.size());
        if (read <= 0) {
            streaming = false;
            sleep_ms(200);
            continue;
        }
        lastRead = now_ms();
        streaming = true;

        int sr = SAMPLE_RATE, ch = CHANNELS;
        int samples = decoder.decodeFrame(netBuf.data(), read, pcmBuf, sr, ch);
        if (samples > 0) {
            audioOut->queueSamples(pcmBuf.data(), samples, sr, ch);
        }
    }
}
