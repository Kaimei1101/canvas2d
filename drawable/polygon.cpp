#include "polygon.h"

#include "geometry.h"

Polygon::Polygon(std::vector<Point> verts, bool fill, RGBA fillColor,
                 bool stroke, double width, RGBA strokeColor, LineType lineType)
    : verts{std::move(verts)}, width{width}, strokeColor{strokeColor},
      fillColor{fillColor}, stroke{stroke}, fill{fill}, lineType{lineType}
{
}

void Polygon::draw(RenderTarget &rt) const
{
    if (fill) {
        rt.fillPolygon(verts, fillColor);
    }
    if (stroke) {
        rt.drawPolygonOutline(verts, width, strokeColor, lineType);
    }
}

void Polygon::drawSelHighlight(RenderTarget &rt) const
{
    rt.drawPolygonOutline(verts,
                          width + Drawable::selHighlightWidth / rt.camera.scale,
                          Drawable::selHighlightColor, LineType{});
}

bool Polygon::hitTest(const Camera &camera, Point p) const
{
    if (verts.empty())
        return false;

    double tol = Drawable::hitTestTol / camera.scale + width / 2.0;

    if (isFilled()) {
        if (contains(p)) {
            return true;
        }
    }

    for (std::size_t i = 0, j = verts.size() - 1; i < verts.size(); j = i++) {
        Point a = verts[j];
        Point b = verts[i];
        if (Geometry::pointToSegmentDistanceSqr(p, a, b) <= tol * tol) {
            return true;
        }
    }

    return false;
}

Point Polygon::centroid() const
{
    Point sum{0, 0};
    for (const auto &v : verts) {
        sum = sum + v;
    }
    return verts.empty() ? sum : sum / static_cast<double>(verts.size());
}

void Polygon::transform(Transform t)
{
    for (auto &p : verts) {
        p = p.transform(t);
    }
    width *= t.getScale();
}

// 射线法（奇偶规则）：从 p 向右作水平射线，统计穿边次数
bool Polygon::contains(Point p) const
{
    if (verts.size() < 3) {
        return false;
    }
    bool inside = false;
    for (std::size_t i = 0, j = verts.size() - 1; i < verts.size(); j = i++) {
        const Point &a = verts[j];
        const Point &b = verts[i];
        // 边跨越 p.y 且交点在 p.x 右侧
        if ((a.y > p.y) != (b.y > p.y)) {
            const double xCross = a.x + (b.x - a.x) * (p.y - a.y) / (b.y - a.y);
            if (xCross > p.x) {
                inside = !inside;
            }
        }
    }
    return inside;
}

AABB Polygon::bounds() const
{

    if (verts.empty()) {
        return AABB{0, 0, 0, 0};
    }

    AABB bounds = AABB{verts[0], verts[0]};

    for (size_t i = 1; i < verts.size(); ++i) {
        bounds.expand(verts[i]);
    }

    return bounds;
}