#include "wifi.h"
#include <cstdlib>
#include <cstring>
#include <3ds/services/soc.h>

#define SOC_ALIGN 0x1000
#define SOC_BUFFERSIZE 0x100000

static u32* socBuffer = nullptr;

WifiManager::WifiManager() : ready(false) {}

bool WifiManager::init() {
    if (ready) return true;
    socBuffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    if (!socBuffer) return false;
    if (socInit(socBuffer, SOC_BUFFERSIZE) != 0) {
        free(socBuffer);
        socBuffer = nullptr;
        return false;
    }
    ready = true;
    return true;
}

void WifiManager::shutdown() {
    if (!ready) return;
    socExit();
    free(socBuffer);
    socBuffer = nullptr;
    ready = false;
}
