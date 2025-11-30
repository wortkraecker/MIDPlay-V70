#include "ui.h"
#include "utils.h"
#include "stb_image.h"
#include <cstdio>
#include <cstring>

extern "C" unsigned char* stbi_load_from_memory(const unsigned char* buffer, int len, int* x, int* y, int* comp, int req_comp) {
    // Stub: return null to indicate failure; replace with full stb_image for real decode
    (void)buffer; (void)len; (void)x; (void)y; (void)comp; (void)req_comp;
    return nullptr;
}

extern "C" void stbi_image_free(void* p) { (void)p; }

UI::UI() : hasCover(false), bgColor(0x202020FF) {
    memset(&coverTex, 0, sizeof(coverTex));
}

bool UI::init() {
    if (romfsInit() != 0) {
        // romfs optional; continue
    }
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    C2D_SceneBegin(C2D_GetScreenTarget(GFX_TOP, GFX_LEFT));
    return true;
}

void UI::shutdown() {
    if (hasCover) {
        C3D_TexDelete(&coverTex);
    }
    C2D_Fini();
    C3D_Fini();
    romfsExit();
}

void UI::updateCover(const std::vector<u8>& jpegData, const u8 avgColor[3]) {
    if (hasCover) { C3D_TexDelete(&coverTex); hasCover = false; }
    bgColor = C2D_Color32(avgColor[0], avgColor[1], avgColor[2], 0xFF);
    int w = 0, h = 0, comp = 0;
    unsigned char* img = stbi_load_from_memory(jpegData.data(), (int)jpegData.size(), &w, &h, &comp, 4);
    if (!img) {
        // fallback solid texture
        w = h = 128;
        std::vector<u8> solid(w * h * 4, 0xFF);
        C3D_TexInit(&coverTex, w, h, GPU_RGBA8);
        C3D_TexUpload(&coverTex, solid.data());
        C3D_TexSetFilter(&coverTex, GPU_LINEAR, GPU_LINEAR);
        coverImg.tex = &coverTex;
        coverImg.subtex = nullptr;
        hasCover = true;
        return;
    }
    C3D_TexInit(&coverTex, w, h, GPU_RGBA8);
    C3D_TexUpload(&coverTex, img);
    C3D_TexSetFilter(&coverTex, GPU_LINEAR, GPU_LINEAR);
    coverImg.tex = &coverTex;
    coverImg.subtex = nullptr;
    hasCover = true;
    stbi_image_free(img);
}

void UI::draw(const Metadata& meta, bool connected, int volume, bool streaming) {
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_Target* top = C2D_GetScreenTarget(GFX_TOP, GFX_LEFT);
    C2D_SceneBegin(top);
    C2D_Clear(bgColor);

    float x = 20, y = 20;
    if (hasCover) {
        C2D_DrawImageAt(coverImg, x, y, 0.5f, nullptr, 128.0f / coverTex.width, 128.0f / coverTex.height);
    }

    C2D_TextBuf buf = C2D_TextBufNew(4096);
    C2D_Text text;

    char line[256];
    snprintf(line, sizeof(line), "Track: %s", meta.track.c_str());
    C2D_TextParse(&text, buf, line);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_WithColor(C2D_AtBaseline(x + 140, y), 0.6f, C2D_Color32(255,255,255,255)), 0.6f, 0.6f);

    snprintf(line, sizeof(line), "Artist: %s", meta.artist.c_str());
    C2D_TextParse(&text, buf, line);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_WithColor(C2D_AtBaseline(x + 140, y + 28), 0.6f, C2D_Color32(220,220,220,255)), 0.6f, 0.6f);

    snprintf(line, sizeof(line), "Album: %s", meta.album.c_str());
    C2D_TextParse(&text, buf, line);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_WithColor(C2D_AtBaseline(x + 140, y + 56), 0.6f, C2D_Color32(200,200,200,255)), 0.6f, 0.6f);

    snprintf(line, sizeof(line), "%02d:%02d / %02d:%02d", meta.positionMs/60000, (meta.positionMs/1000)%60, meta.durationMs/60000, (meta.durationMs/1000)%60);
    C2D_TextParse(&text, buf, line);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_WithColor(C2D_AtBaseline(x + 140, y + 84), 0.6f, C2D_Color32(180,255,180,255)), 0.6f, 0.6f);

    snprintf(line, sizeof(line), "[Y] Play/Pause  [X] Prev  [A] Next  [B] Reconnect");
    C2D_TextParse(&text, buf, line);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_AtBaseline(x, y + 160), 0, 0.55f, 0.55f);

    snprintf(line, sizeof(line), "Volume: %d", volume);
    C2D_TextParse(&text, buf, line);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_AtBaseline(x, y + 190), 0, 0.55f, 0.55f);
    float barW = 200.0f * (float)volume / 100.0f;
    C2D_DrawRectSolid(x, y + 200, 0, barW, 8, C2D_Color32(0,255,120,255));
    C2D_DrawRectSolid(x + barW, y + 200, 0, 200 - barW, 8, C2D_Color32(60,60,60,255));

    snprintf(line, sizeof(line), "Status: %s / %s", connected ? "Connected" : "Offline", streaming ? "Streaming" : "Idle");
    C2D_TextParse(&text, buf, line);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_AtBaseline(x, y + 220), 0, 0.6f, 0.6f);

    C2D_TextBufDelete(buf);
    C3D_FrameEnd(0);
}
