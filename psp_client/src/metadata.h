#ifndef PSP_CLIENT_METADATA_H
#define PSP_CLIENT_METADATA_H

#include <psptypes.h>

struct TrackMetadata {
    char title[128];
    char artist[128];
    char album[128];
    int duration_ms;
    int position_ms;
    int average_r;
    int average_g;
    int average_b;
};

struct PlaybackStatus {
    int playing;
    int volume;
};

class MetadataClient {
public:
    MetadataClient(const char* host, int port);
    bool fetchMetadata(TrackMetadata& meta, unsigned char* buffer, size_t bufSize);
    bool fetchStatus(PlaybackStatus& status, unsigned char* buffer, size_t bufSize);
    int fetchCover(unsigned char* buffer, size_t bufSize);

private:
    void parseJsonValue(const char* json, const char* key, char* out, size_t outSize);
    int parseJsonInt(const char* json, const char* key);
    const char* m_host;
    int m_port;
};

#endif
