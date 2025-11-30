#pragma once
#include <3ds.h>
#include <string>
#include <vector>

struct Metadata {
    std::string track;
    std::string artist;
    std::string album;
    int durationMs;
    int positionMs;
    int volume;
    bool playing;
    u8 avgColor[3];
    std::vector<u8> coverJpeg;

    Metadata();
    bool parseJson(const std::string& jsonText);
};
