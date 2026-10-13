#include <algorithm>

#include <cmath>
#include <limits>

#include "pixel.h"

Pixel::Pixel(int x, int y) : x{x}, y{y} {}

Pixel::Pixel(Point p)
    : x{static_cast<int>(std::round(
          std::clamp(p.x, static_cast<double>(std::numeric_limits<int>::min()),
                     static_cast<double>(std::numeric_limits<int>::max()))))},
      y{static_cast<int>(std::round(
          std::clamp(p.y, static_cast<double>(std::numeric_limits<int>::min()),
                     static_cast<double>(std::numeric_limits<int>::max()))))}
{
}

Pixel Pixel::operator+(Pixel other) const
{
    return Pixel{x + other.x, y + other.y};
}

Pixel Pixel::operator-(Pixel other) const
{
    return Pixel{x - other.x, y - other.y};
}

bool Pixel::operator==(Pixel other) const
{
    return x == other.x && y == other.y;
}

bool Pixel::operator!=(Pixel other) const { return !(*this == other); }