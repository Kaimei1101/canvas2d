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

    Pixel(int x, int y);
    explicit Pixel(Point p);
    Pixel operator+(Pixel other) const;
    Pixel operator-(Pixel other) const;
    bool operator==(Pixel otehr) const;
    bool operator!=(Pixel other) const;
};

#endif // PIXEL_H
