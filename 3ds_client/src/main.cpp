#include <3ds.h>
#include <citro2d.h>
#include <string>
#include "utils.h"
#include "wifi.h"
#include "http_client.h"
#include "metadata.h"
#include "ndsp_audio.h"
#include "audio_stream.h"
#include "ui.h"

static bool fetch_metadata(HttpClient& http, Metadata& meta) {
    HttpResponse resp{};
    if (!http.get("/api/metadata", resp)) return false;
    std::string body(resp.body.begin(), resp.body.end());
    meta.parseJson(body);
    return true;
}

static bool fetch_status(HttpClient& http, Metadata& meta) {
    HttpResponse resp{};
    if (!http.get("/api/status", resp)) return false;
    std::string body(resp.body.begin(), resp.body.end());
    meta.parseJson(body);
    return true;
}

static bool fetch_cover(HttpClient& http, Metadata& meta) {
    HttpResponse resp{};
    if (!http.get("/api/cover", resp)) return false;
    meta.coverJpeg = resp.body;
    return true;
}

static void post_action(HttpClient& http, const char* path, const std::string& payload = "") {
    HttpResponse resp{};
    http.post(path, payload, resp);
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);

    WifiManager wifi;
    if (!wifi.init()) {
        printf("Wi-Fi init failed.\n");
        sleep_ms(2000);
        return 0;
    }

    NdspAudio audio;
    if (!audio.init()) {
        printf("NDSP init failed.\n");
        sleep_ms(2000);
        return 0;
    }

    HttpClient http;
    http.init();

    AudioStream stream;
    stream.init(&http, &audio);
    stream.start();

    UI ui;
    ui.init();

    Metadata meta;
    fetch_metadata(http, meta);
    fetch_cover(http, meta);
    ui.updateCover(meta.coverJpeg, meta.avgColor);
    std::string lastTrack = meta.track;

    u64 lastMetaPoll = now_ms();
    bool running = true;

    while (aptMainLoop() && running) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if (kDown & KEY_START) { running = false; }
        if (kDown & KEY_Y) { post_action(http, "/api/play"); fetch_status(http, meta); }
        if (kDown & KEY_X) { post_action(http, "/api/prev"); }
        if (kDown & KEY_A) { post_action(http, "/api/next"); }
        if (kDown & KEY_B) { stream.reconnect(); }
        if (kHeld & KEY_DUP) { meta.volume = clamp_int(meta.volume + 1, 0, 100); post_action(http, "/api/volume", std::to_string(meta.volume)); }
        if (kHeld & KEY_DDOWN) { meta.volume = clamp_int(meta.volume - 1, 0, 100); post_action(http, "/api/volume", std::to_string(meta.volume)); }

        stream.update();
        audio.update();

        u64 now = now_ms();
        if (now - lastMetaPoll > 1000) {
            fetch_status(http, meta);
            fetch_metadata(http, meta);
            if (meta.track != lastTrack) {
                fetch_cover(http, meta);
                ui.updateCover(meta.coverJpeg, meta.avgColor);
                lastTrack = meta.track;
            }
            lastMetaPoll = now;
        }

        ui.draw(meta, wifi.isReady(), meta.volume, stream.isStreaming());
        gspWaitForVBlank();
    }

    stream.stop();
    ui.shutdown();
    http.exit();
    audio.shutdown();
    wifi.shutdown();
    gfxExit();
    return 0;
}
