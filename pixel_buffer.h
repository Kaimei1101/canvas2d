#ifndef PIXEL_BUFFER_H
#define PIXEL_BUFFER_H

#include <cstdint>
#include <vector>

#include "rgba.h"

class PixelBuffer
{
    int width;
    int height;
    int channels;
    std::vector<RGBA> arr;

  public:
    PixelBuffer(int width, int height);
    int getWidth() const;
    int getHeight() const;
    int getChannels() const;
    const std::uint8_t *as_raw() const;
    void clear();
    RGBA &operator()(int x, int y);
    const RGBA &operator()(int x, int y) const;
};

#endif // PIXEL_BUFFER_H
