#pragma once
#include <3ds.h>
#include <citro2d.h>
#include <string>
#include "metadata.h"

class UI {
public:
    UI();
    bool init();
    void shutdown();
    void draw(const Metadata& meta, bool connected, int volume, bool streaming);
    void updateCover(const std::vector<u8>& jpegData, const u8 avgColor[3]);
private:
    C2D_Image coverImg;
    C3D_Tex coverTex;
    bool hasCover;
    u32 bgColor;
};
