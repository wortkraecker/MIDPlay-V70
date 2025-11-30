#pragma once
#include <3ds.h>
#include <vector>
#include <3ds/svc.h>
#include <3ds/types.h>
#include <3ds/thread.h>
#include "http_client.h"
#include "ndsp_audio.h"
#include "mp3_decoder.h"
#include "utils.h"

class AudioStream {
public:
    AudioStream();
    bool init(HttpClient* client, NdspAudio* audio);
    void start();
    void stop();
    void reconnect();
    void update(); // called from main loop to detect timeouts
    bool isStreaming() const { return streaming; }

private:
    static void threadMain(void* arg);
    void run();

    HttpClient* http;
    NdspAudio* audioOut;
    Mp3Decoder decoder;

    int streamSock;
    bool streaming;
    bool exitThread;
    Thread thread;
    std::vector<u8> netBuf;
    std::vector<int16_t> pcmBuf;
    u64 lastRead;
};
