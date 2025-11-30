#include "metadata.h"
#include "http_client.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>

MetadataClient::MetadataClient(const char* host, int port) : m_host(host), m_port(port) {}

void MetadataClient::parseJsonValue(const char* json, const char* key, char* out, size_t outSize) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":\"", key);
    const char* pos = strstr(json, search);
    if (!pos) {
        out[0] = '\0';
        return;
    }
    pos += strlen(search);
    size_t i = 0;
    while (*pos && *pos != '"' && i < outSize - 1) {
        out[i++] = *pos++;
    }
    out[i] = '\0';
}

int MetadataClient::parseJsonInt(const char* json, const char* key) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);
    const char* pos = strstr(json, search);
    if (!pos) return 0;
    pos += strlen(search);
    return atoi(pos);
}

bool MetadataClient::fetchMetadata(TrackMetadata& meta, unsigned char* buffer, size_t bufSize) {
    HttpClient client(m_host, m_port);
    HttpResponse resp;
    if (client.get("/api/metadata", resp, buffer, bufSize - 1, false) < 0) return false;
    const char* json = (const char*)resp.data;
    parseJsonValue(json, "title", meta.title, sizeof(meta.title));
    parseJsonValue(json, "artist", meta.artist, sizeof(meta.artist));
    parseJsonValue(json, "album", meta.album, sizeof(meta.album));
    meta.duration_ms = parseJsonInt(json, "duration");
    meta.position_ms = parseJsonInt(json, "position");
    meta.average_r = parseJsonInt(json, "average_color_r");
    meta.average_g = parseJsonInt(json, "average_color_g");
    meta.average_b = parseJsonInt(json, "average_color_b");
    return true;
}

bool MetadataClient::fetchStatus(PlaybackStatus& status, unsigned char* buffer, size_t bufSize) {
    HttpClient client(m_host, m_port);
    HttpResponse resp;
    if (client.get("/api/status", resp, buffer, bufSize - 1, false) < 0) return false;
    const char* json = (const char*)resp.data;
    status.playing = parseJsonInt(json, "playing");
    status.volume = parseJsonInt(json, "volume");
    return true;
}

int MetadataClient::fetchCover(unsigned char* buffer, size_t bufSize) {
    HttpClient client(m_host, m_port);
    HttpResponse resp;
    if (client.get("/api/cover", resp, buffer, bufSize, true) < 0) return -1;
    return resp.data_size;
}

