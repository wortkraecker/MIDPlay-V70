#include "audio_stream.h"
#include "utils.h"
#include <cstring>

AudioStream::AudioStream() : http(nullptr), audioOut(nullptr), streamSock(-1), lastRead(0), streaming(false) {}

bool AudioStream::init(HttpClient* client, NdspAudio* audio) {
    http = client;
    audioOut = audio;
    decoder.init();
    netBuf.resize(STREAM_BUFFER_SIZE);
    streamSock = http->openStream("/stream.mp3");
    streaming = streamSock >= 0;
    lastRead = now_ms();
    return streaming;
}

void AudioStream::reconnect() {
    if (streamSock >= 0) http->closeStream(streamSock);
    streamSock = http->openStream("/stream.mp3");
    decoder.reset();
    streaming = streamSock >= 0;
    lastRead = now_ms();
}

void AudioStream::stop() {
    if (streamSock >= 0) http->closeStream(streamSock);
    streamSock = -1;
    streaming = false;
}

void AudioStream::consumeStream() {
    int read = http->readStream(streamSock, netBuf.data(), netBuf.size());
    if (read <= 0) { streaming = false; return; }
    lastRead = now_ms();
    std::vector<int16_t> pcm;
    int sr = SAMPLE_RATE, ch = CHANNELS;
    int samples = decoder.decodeFrame(netBuf.data(), read, pcm, sr, ch);
    if (samples > 0) {
        audioOut->queueSamples(pcm.data(), samples, sr, ch);
    }
}

void AudioStream::update() {
    if (!streaming) return;
    consumeStream();
    if (now_ms() - lastRead > 2000) { // timeout
        streaming = false;
    }
}
