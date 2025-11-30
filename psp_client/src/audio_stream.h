#ifndef PSP_CLIENT_AUDIO_STREAM_H
#define PSP_CLIENT_AUDIO_STREAM_H

#include <psptypes.h>
#include "mp3_decoder.h"
#include "psp_audio.h"
#include "http_client.h"

class AudioStream {
public:
    AudioStream(const char* host, int port);
    void start();
    void stop();
    void update();
    bool isRunning() const { return running; }

private:
    void handleStream();

    char m_host[64];
    int m_port;
    HttpClient client;
    Mp3Decoder decoder;
    bool running;
    unsigned char streamBuffer[4096];
};

#endif
