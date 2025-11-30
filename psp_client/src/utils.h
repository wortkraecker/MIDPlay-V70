#ifndef PSP_CLIENT_UTILS_H
#define PSP_CLIENT_UTILS_H

#include <pspkernel.h>
#include <pspdebug.h>
#include <stdio.h>

#define LOG(msg, ...) pspDebugScreenPrintf("[LOG] " msg "\n", ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) pspDebugScreenPrintf("[ERR] " msg "\n", ##__VA_ARGS__)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

inline int clamp_int(int v, int min_v, int max_v) {
    if (v < min_v) return min_v;
    if (v > max_v) return max_v;
    return v;
}

#endif
