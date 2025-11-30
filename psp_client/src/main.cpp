#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspgu.h>
#include <string.h>

#include "utils.h"
#include "wifi.h"
#include "psp_audio.h"
#include "audio_stream.h"
#include "metadata.h"
#include "ui.h"
#include "http_client.h"

PSP_MODULE_INFO("SpotifyConnectPSP", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

static const char* SERVER_HOST = "192.168.1.10"; // replace with server IP
static const int SERVER_PORT = 4000;

int exit_callback(int arg1, int arg2, void* common) {
    sceKernelExitGame();
    return 0;
}

int CallbackThread(SceSize args, void* argp) {
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int SetupCallbacks(void) {
    int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
    if (thid >= 0) sceKernelStartThread(thid, 0, 0);
    return thid;
}

int main(int argc, char* argv[]) {
    pspDebugScreenInit();
    SetupCallbacks();

    if (wifi::initialize() < 0) {
        LOG_ERROR("Wi-Fi init failed");
        return 0;
    }

    wifi::connect(NULL);

    psp_audio::initialize();

    AudioStream stream(SERVER_HOST, SERVER_PORT);
    MetadataClient metaClient(SERVER_HOST, SERVER_PORT);
    TrackMetadata metadata = {"","","",0,0,0,0,0};
    PlaybackStatus status = {0, 50};
    unsigned char metaBuffer[4096];
    unsigned char statusBuffer[2048];
    unsigned char coverBuffer[40 * 1024];
    int coverSize = 0;
    int volume = 50;

    ui::initialize();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    u32 bgColor = 0x303030FF;

    stream.start();

    int frames = 0;
    while (1) {
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(&pad, 1);
        if (pad.Buttons & PSP_CTRL_START) {
            break;
        }
        if (pad.Buttons & PSP_CTRL_TRIANGLE) {
            if (status.playing) {
                HttpClient h(SERVER_HOST, SERVER_PORT);
                HttpResponse r; unsigned char buf[512];
                h.post("/api/pause", "{}", r, buf, sizeof(buf));
                status.playing = 0;
            } else {
                HttpClient h(SERVER_HOST, SERVER_PORT);
                HttpResponse r; unsigned char buf[512];
                h.post("/api/play", "{}", r, buf, sizeof(buf));
                status.playing = 1;
            }
        }
        if (pad.Buttons & PSP_CTRL_SQUARE) {
            HttpClient h(SERVER_HOST, SERVER_PORT);
            HttpResponse r; unsigned char buf[512];
            h.post("/api/prev", "{}", r, buf, sizeof(buf));
        }
        if (pad.Buttons & PSP_CTRL_CIRCLE) {
            stream.stop();
            stream.start();
        }
        if (pad.Buttons & PSP_CTRL_CROSS) {
            HttpClient h(SERVER_HOST, SERVER_PORT);
            HttpResponse r; unsigned char buf[512];
            h.post("/api/next", "{}", r, buf, sizeof(buf));
        }
        if (pad.Buttons & PSP_CTRL_UP) {
            volume = clamp_int(volume + 2, 0, 100);
            char body[32];
            snprintf(body, sizeof(body), "{\"volume\":%d}", volume);
            HttpClient h(SERVER_HOST, SERVER_PORT);
            HttpResponse r; unsigned char buf[512];
            h.post("/api/volume", body, r, buf, sizeof(buf));
        }
        if (pad.Buttons & PSP_CTRL_DOWN) {
            volume = clamp_int(volume - 2, 0, 100);
            char body[32];
            snprintf(body, sizeof(body), "{\"volume\":%d}", volume);
            HttpClient h(SERVER_HOST, SERVER_PORT);
            HttpResponse r; unsigned char buf[512];
            h.post("/api/volume", body, r, buf, sizeof(buf));
        }

        stream.update();
        psp_audio::mixAndPlay();

        // metadata fetch every ~1 second
        if (frames % 60 == 0) {
            if (metaClient.fetchMetadata(metadata, metaBuffer, sizeof(metaBuffer))) {
                bgColor = ((metadata.average_r & 0xFF) << 24) | ((metadata.average_g & 0xFF) << 16) | ((metadata.average_b & 0xFF) << 8) | 0xFF;
            }
            metaClient.fetchStatus(status, statusBuffer, sizeof(statusBuffer));
        }

        if (frames % 240 == 0) {
            int size = metaClient.fetchCover(coverBuffer, sizeof(coverBuffer));
            if (size > 0) coverSize = size;
        }

        ui::render(metadata, status, volume, wifi::isConnected(), 128, 128, coverBuffer, bgColor);

        frames++;
    }

    stream.stop();
    psp_audio::shutdown();
    ui::shutdown();
    wifi::shutdown();

    sceKernelExitGame();
    return 0;
}
