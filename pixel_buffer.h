#ifndef PIXEL_BUFFER_H
#define PIXEL_BUFFER_H

#include <cstdint>
#include <vector>

#include "rgba.h"

template <typename T> class PixelBuffer
{
    int width;
    int height;
    std::vector<T> arr;

  public:
    PixelBuffer(int width, int height, T value = {})
        : width(width), height(height),
          arr(static_cast<size_t>(width) * height, value)
    {
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    T *data() { return arr.data(); }
    const T *data() const { return arr.data(); }

    const unsigned char *as_raw() const
    {
        return reinterpret_cast<const unsigned char *>(arr.data());
    }

    T &operator()(int x, int y)
    {
        return arr[static_cast<size_t>(y) * width + x];
    }

    const T &operator()(int x, int y) const
    {
        return arr[static_cast<size_t>(y) * width + x];
    }

    T *row_ptr(int y) { return arr.data() + static_cast<size_t>(y) * width; }
    const T *row_ptr(int y) const
    {
        return arr.data() + static_cast<size_t>(y) * width;
    }

    void clear(T color = T{}) { std::fill(arr.begin(), arr.end(), color); }
};

using RGBABuffer = PixelBuffer<RGBA>;
using MaskBuffer = PixelBuffer<std::uint8_t>;

#endif // PIXEL_BUFFER_H
