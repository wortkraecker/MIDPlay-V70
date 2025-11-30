#ifndef STB_IMAGE_H
#define STB_IMAGE_H
// Minimal stub of stb_image for offline builds. Replace with full upstream stb_image.h for real JPEG decoding.
extern "C" unsigned char* stbi_load_from_memory(const unsigned char* buffer, int len, int* x, int* y, int* comp, int req_comp);
extern "C" void stbi_image_free(void* retval_from_stbi_load);
#endif
