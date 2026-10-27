#include "aabb.h"

AABB::AABB(std::span<const Point> pts) : AABB{0, 0, 0, 0}
{
    if (pts.empty()) {
        return;
    }

    *this = AABB{pts[0], pts[0]};

    for (size_t i = 1; i < pts.size(); ++i) {
        expand(pts[i]);
    }
}