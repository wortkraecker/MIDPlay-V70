#pragma once
#include <3ds.h>
#include <vector>
#include "http_client.h"
#include "mp3_decoder.h"
#include "ndsp_audio.h"

class AudioStream {
public:
    AudioStream();
    bool init(HttpClient* client, NdspAudio* audio);
    void stop();
    void update();
    void reconnect();
    bool isStreaming() const { return streaming; }
private:
    void consumeStream();
    HttpClient* http;
    NdspAudio* audioOut;
    Mp3Decoder decoder;
    int streamSock;
    std::vector<u8> netBuf;
    u64 lastRead;
    bool streaming;
};
