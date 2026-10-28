#include "point.h"

#include <cmath>

#include "transform.h"

Point::Point(double x, double y) : x{x}, y{y} {}

Point::Point(Pixel p) : x{static_cast<double>(p.x)}, y{static_cast<double>(p.y)}
{
}

Point Point::operator+(Point other) const
{
    return Point{x + other.x, y + other.y};
}

Point Point::operator-(Point other) const
{
    return Point{x - other.x, y - other.y};
}

Point Point::operator*(double scale) const
{
    return Point{x * scale, y * scale};
}

Point Point::operator/(double scale) const
{
    return Point{x / scale, y / scale};
}

double Point::distance(Point other) const
{
    return std::sqrt(distanceSqr(other));
}

double Point::distanceSqr(Point other) const
{
    auto vec = other - *this;
    return vec.x * vec.x + vec.y * vec.y;
}

Point Point::transform(Transform t) const { return t.apply(*this); }