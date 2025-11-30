#ifndef PSP_CLIENT_WIFI_H
#define PSP_CLIENT_WIFI_H

#include <psptypes.h>

namespace wifi {
    int initialize();
    int connect(const char* profileName);
    void shutdown();
    bool isConnected();
}

#endif
