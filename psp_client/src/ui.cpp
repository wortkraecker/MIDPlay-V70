#include "ui.h"
#include "utils.h"

#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <string.h>

static unsigned int __attribute__((aligned(16))) list[262144];

namespace ui {

void initialize() {
    sceGuInit();
    sceGuStart(GU_DIRECT, list);
    sceGuDrawBuffer(GU_PSM_8888, (void*)0, 512);
    sceGuDispBuffer(480, 272, (void*)0x88000, 512);
    sceGuDepthBuffer((void*)0x110000, 512);
    sceGuOffset(2048 - (480 / 2), 2048 - (272 / 2));
    sceGuViewport(2048, 2048, 480, 272);
    sceGuDepthMask(GU_TRUE);
    sceGuDisable(GU_DEPTH_TEST);
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
    pspDebugScreenInit();
}

void shutdown() {
    sceGuTerm();
}

static void drawRect(int x, int y, int w, int h, unsigned int color) {
    sceGuDisable(GU_TEXTURE_2D);
    struct Vertex { float x, y, z; unsigned int color; };
    Vertex* vertices = (Vertex*)sceGuGetMemory(2 * sizeof(Vertex));
    vertices[0].x = x; vertices[0].y = y; vertices[0].z = 0; vertices[0].color = color;
    vertices[1].x = x + w; vertices[1].y = y + h; vertices[1].z = 0; vertices[1].color = color;
    sceGuColor(color);
    sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, vertices);
}

static void drawText(int x, int y, const char* text, unsigned int color) {
    pspDebugScreenSetXY(x / 8, y / 8);
    pspDebugScreenSetTextColor(color);
    pspDebugScreenPrintf("%s", text);
}

void render(const TrackMetadata& meta, const PlaybackStatus& status, int volume, bool connected, int coverWidth, int coverHeight, const unsigned char* coverData, u32 backgroundColor) {
    sceGuStart(GU_DIRECT, list);
    sceGuClearColor(backgroundColor);
    sceGuClear(GU_COLOR_BUFFER_BIT);

    // Background tint
    drawRect(0, 0, 480, 272, backgroundColor);

    // Cover placeholder (actual JPEG decoding should upload to texture)
    drawRect(20, 20, 128, 128, 0xFFFFFFFF);

    char line[128];
    snprintf(line, sizeof(line), "%s", meta.title);
    drawText(170, 30, line, 0xFFFFFFFF);
    snprintf(line, sizeof(line), "%s", meta.artist);
    drawText(170, 50, line, 0xFFFFFFFF);
    snprintf(line, sizeof(line), "%s", meta.album);
    drawText(170, 70, line, 0xFFFFFFFF);
    snprintf(line, sizeof(line), "%02d:%02d / %02d:%02d", meta.position_ms / 60000, (meta.position_ms / 1000) % 60, meta.duration_ms / 60000, (meta.duration_ms / 1000) % 60);
    drawText(170, 90, line, 0xFFFFFFFF);

    drawText(170, 130, status.playing ? "Playing" : "Paused", 0xFFFFFFFF);
    snprintf(line, sizeof(line), "Volume: %d", volume);
    drawText(170, 150, line, 0xFFFFFFFF);
    drawText(170, 170, connected ? "Connected" : "Disconnected", 0xFFFFFFFF);

    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
}

}
