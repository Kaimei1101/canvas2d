#include "pixel_buffer.h"

PixelBuffer::PixelBuffer(int width, int height)
    : width{width}, height{height},
      arr(width * height, RGBA{255, 255, 255, 255})
{
}

int PixelBuffer::getwidth() const { return width; }

int PixelBuffer::getheight() const { return height; }

int PixelBuffer::getchannels() const { return channels; }

const std::uint8_t *PixelBuffer::as_raw() const
{
    return reinterpret_cast<const std::uint8_t *>(arr.data());
}

void PixelBuffer::clear()
{
    std::fill(arr.begin(), arr.end(), RGBA{255, 255, 255, 255});
}

RGBA &PixelBuffer::operator()(int x, int y) { return arr[y * width + x]; }

const RGBA &PixelBuffer::operator()(int x, int y) const
{
    return arr[y * width + x];
}