#pragma once
#include <3ds.h>

class WifiManager {
public:
    WifiManager();
    bool init();
    void shutdown();
    bool isReady() const { return ready; }
private:
    bool ready;
};
