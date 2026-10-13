#include "point.h"

#include <cmath>

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

bool Point::operator==(Point other) const
{
    return x == other.x && y == other.y;
}

bool Point::operator!=(Point other) const { return !(*this == other); }

double Point::distance(Point other) const
{
    auto vec = other - *this;
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}