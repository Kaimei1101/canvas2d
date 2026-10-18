#include "transform.h"

Transform::Transform(Point pivot, Point translate, double scale, double rotate)
{
    double c = std::cos(rotate);
    double s = std::sin(rotate);

    m00 = c * scale;
    m01 = -s * scale;
    m10 = s * scale;
    m11 = c * scale;

    tx = translate.x + pivot.x - (pivot.x * m00 + pivot.y * m01);
    ty = translate.y + pivot.y - (pivot.x * m10 + pivot.y * m11);
}
