#ifndef AABB_H
#define AABB_H

#include <algorithm>
#include <span>

#include "point.h"

class AABB
{
    Point min;
    Point max;

  public:
    inline AABB(double x1, double y1, double x2, double y2)
        : min{std::min(x1, x2), std::min(y1, y2)},
          max{std::max(x1, x2), std::max(y1, y2)} {};

    inline AABB(Point a, Point b)
        : min{std::min(a.x, b.x), std::min(a.y, b.y)},
          max{std::max(a.x, b.x), std::max(a.y, b.y)} {};

    AABB(std::span<const Point> pts);

    inline bool intersect(AABB other)
    {
        return (max.x >= other.min.x && min.x <= other.max.x) &&
               (max.y >= other.min.y && min.y <= other.max.y);
    }

    inline void expand(Point p)
    {
        min.x = std::min(min.x, p.x);
        min.y = std::min(min.y, p.y);
        max.x = std::max(max.x, p.x);
        max.y = std::max(max.y, p.y);
    }

    inline bool contains(const AABB &o) const
    {
        return min.x <= o.min.x && max.x >= o.max.x && min.y <= o.min.y &&
               max.y >= o.max.y;
    }
};

#endif // AABB_H
