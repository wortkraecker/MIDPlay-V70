#ifndef PSP_CLIENT_UI_H
#define PSP_CLIENT_UI_H

#include <psptypes.h>
#include "metadata.h"

namespace ui {
    void initialize();
    void shutdown();
    void render(const TrackMetadata& meta, const PlaybackStatus& status, int volume, bool connected, int coverWidth, int coverHeight, const unsigned char* coverData, u32 backgroundColor);
}

#endif
