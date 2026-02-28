#include <stb_image_write.h>
#include "save.h"

int saveFramebufferToImage(const SRPFramebuffer* fb, const char* outputPath)
{
    int width = fb->width;
    int height = fb->height;

    unsigned char* pixels = malloc(width * height * 4);
    if (!pixels)
        return 1;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            uint32_t c = fb->color[y * width + x];

            unsigned char r = (c >> 24) & 0xFF;
            unsigned char g = (c >> 16) & 0xFF;
            unsigned char b = (c >> 8)  & 0xFF;

            int i = (y * width + x) * 4;
            pixels[i + 0] = r;
            pixels[i + 1] = g;
            pixels[i + 2] = b;
            pixels[i + 3] = 0xFF;
        }
    }

    int ok = stbi_write_png(outputPath, width, height, 4, pixels, width * 4);
    free(pixels);
    return ok;
}
