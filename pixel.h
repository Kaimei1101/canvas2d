#ifndef PIXEL_H
#define PIXEL_H

#include <cstddef>
#include <type_traits>

#include "point.h"

class Point;

class Pixel
{
  public:
    int x;
    int y;

    constexpr Pixel(int x, int y) : x{x}, y{y} {}
    explicit Pixel(Point p);
    Pixel operator+(Pixel other) const;
    Pixel operator-(Pixel other) const;
    bool operator==(const Pixel &otehr) const = default;
};

#endif // PIXEL_H
