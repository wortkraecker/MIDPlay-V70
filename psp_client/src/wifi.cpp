#include "wifi.h"
#include "utils.h"

#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspwlan.h>
#include <pspctrl.h>
#include <string.h>

namespace wifi {

static int g_netInitialized = 0;
static int g_connected = 0;

int initialize() {
    if (g_netInitialized) return 0;
    int err = sceNetInit(0x20000, 42, 0, 0, 0);
    if (err < 0) {
        LOG_ERROR("sceNetInit failed: %08x", err);
        return err;
    }
    err = sceNetInetInit();
    if (err < 0) {
        LOG_ERROR("sceNetInetInit failed: %08x", err);
        return err;
    }
    err = sceNetApctlInit(0x8000, 48);
    if (err < 0) {
        LOG_ERROR("sceNetApctlInit failed: %08x", err);
        return err;
    }
    g_netInitialized = 1;
    return 0;
}

int connect(const char* profileName) {
    if (!g_netInitialized) {
        int r = initialize();
        if (r < 0) return r;
    }

    int config = sceNetApctlGetInfo(8, NULL); // dummy to ensure module loaded
    (void)config;

    int err = sceNetApctlConnect(0);
    if (err < 0) {
        LOG_ERROR("ApctlConnect failed: %08x", err);
        return err;
    }
    int stateLast = -1;
    while (true) {
        int state;
        err = sceNetApctlGetState(&state);
        if (err != 0) {
            LOG_ERROR("ApctlGetState failed: %08x", err);
            break;
        }
        if (state != stateLast) {
            LOG("Wi-Fi state=%d", state);
            stateLast = state;
        }
        if (state == 4) { // GOT_IP
            g_connected = 1;
            break;
        }
        sceKernelDelayThread(50 * 1000);
    }
    return err;
}

void shutdown() {
    if (!g_netInitialized) return;
    sceNetApctlDisconnect();
    sceNetApctlTerm();
    sceNetInetTerm();
    sceNetTerm();
    g_netInitialized = 0;
    g_connected = 0;
}

bool isConnected() {
    return g_connected != 0;
}

}
