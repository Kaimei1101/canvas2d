#ifndef POINT_H
#define POINT_H

#include "pixel.h"

class Pixel;
class Transform;

class Point
{
  public:
    double x;
    double y;

    Point(double x, double y);
    explicit Point(Pixel);
    Point operator+(Point other) const;
    Point operator-(Point other) const;
    Point operator*(double scale) const;
    Point operator/(double scale) const;
    bool operator==(Point other) const;
    bool operator!=(Point other) const;
    double distance(Point other) const;
    double distanceSqr(Point other) const;
    Point transform(Transform) const;
};

#endif // POINT_H
