#include "metadata.h"
#include <cstdlib>

static int extractInt(const std::string& json, const std::string& key, int defVal) {
    size_t pos = json.find(key);
    if (pos == std::string::npos) return defVal;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return defVal;
    return std::atoi(json.c_str() + pos + 1);
}

static std::string extractString(const std::string& json, const std::string& key) {
    size_t pos = json.find(key);
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos + key.size());
    if (pos == std::string::npos) return "";
    size_t end = json.find('"', pos + 1);
    if (end == std::string::npos) return "";
    return json.substr(pos + 1, end - pos - 1);
}

Metadata::Metadata() : durationMs(0), positionMs(0), volume(50), playing(false) {
    avgColor[0] = avgColor[1] = avgColor[2] = 32;
}

bool Metadata::parseJson(const std::string& jsonText) {
    track = extractString(jsonText, "track");
    artist = extractString(jsonText, "artist");
    album = extractString(jsonText, "album");
    durationMs = extractInt(jsonText, "duration", durationMs);
    positionMs = extractInt(jsonText, "position", positionMs);
    volume = extractInt(jsonText, "volume", volume);
    playing = jsonText.find("\"playing\":true") != std::string::npos;
    size_t cpos = jsonText.find("average_color");
    if (cpos != std::string::npos) {
        int r = extractInt(jsonText.substr(cpos), "r", avgColor[0]);
        int g = extractInt(jsonText.substr(cpos), "g", avgColor[1]);
        int b = extractInt(jsonText.substr(cpos), "b", avgColor[2]);
        avgColor[0] = (u8)r; avgColor[1] = (u8)g; avgColor[2] = (u8)b;
    }
    return true;
}
