#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <cmath>

#include "point.h"

// 先缩放，再旋转，最后平移
class Transform
{
    double m00 = 1.0, m01 = 0.0, tx = 0.0;
    double m10 = 0.0, m11 = 1.0, ty = 0.0;

    inline Transform(double m00, double m01, double tx, double m10, double m11,
                     double ty)
        : m00{m00}, m01{m01}, tx{tx}, m10{m10}, m11{m11}, ty{ty}
    {
    }

    inline Transform(double c, double s, Point translate, double scale)
        : Transform{c * scale, -s * scale, translate.x,
                    s * scale, c * scale,  translate.y}
    {
    }

  public:
    Transform() = default;

    inline Transform(Point translate, double scale, double rotate)
        : Transform{std::cos(rotate), std::sin(rotate), translate, scale}
    {
    }

    Transform(Point pivot, Point translate, double scale, double rotate);

    inline Point apply(const Point &p) const
    {
        return Point{p.x * m00 + p.y * m01 + tx, p.x * m10 + p.y * m11 + ty};
    }

    inline Transform operator*(const Transform &o) const
    {
        return Transform{
            m00 * o.m00 + m01 * o.m10,    // m00
            m00 * o.m01 + m01 * o.m11,    // m01
            m00 * o.tx + m01 * o.ty + tx, // tx
            m10 * o.m00 + m11 * o.m10,    // m10
            m10 * o.m01 + m11 * o.m11,    // m11
            m10 * o.tx + m11 * o.ty + ty  // ty
        };
    }

    inline double getScale() { return std::hypot(m00, m10); }
};

#endif // TRANSFORM_H
