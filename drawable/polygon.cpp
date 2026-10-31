#include "polygon.h"

#include "geometry.h"

Polygon::Polygon(std::vector<Point> verts, bool fill, RGBA fillColor,
                 bool stroke, double width, RGBA strokeColor, LineType lineType)
    : verts{std::move(verts)}, width{width}, strokeColor{strokeColor},
      fillColor{fillColor}, stroke{stroke}, fill{fill}, lineType{lineType}
{
}

void Polygon::drawStroke(RenderTarget &rt) const
{
    if (stroke) {
        rt.drawPolygonOutline(verts, width, strokeColor, lineType);
    }
}

void Polygon::fillInner(RenderTarget &rt) const
{
    if (fill) {
        rt.fillPolygon(verts, fillColor);
    }
}

void Polygon::drawSelHighlight(RenderTarget &rt) const
{
    rt.drawPolygonOutline(
        verts, width + Drawable::SEL_HIGHLIGHT_WIDTH / rt.camera.scale,
        Drawable::SEL_HIGHLIGHT_COLOR, LineType{});
    for (auto p : verts) {
        rt.drawCross(p);
    }
}

bool Polygon::hitTest(const Camera &camera, Point p) const
{
    if (verts.empty())
        return false;

    double tol = Drawable::HIT_TEST_TOL / camera.scale + width / 2.0;

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

Point Polygon::centroid() const { return Geometry::centroid(verts); }

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

std::vector<std::unique_ptr<Drawable>>
Polygon::clip(std::span<const Point> clipPoints)
{
    std::vector<Point> result = Geometry::clipPolygon(verts, clipPoints);
    if (result.size() <= 2) {
        return {};
    }
    auto res = std::vector<std::unique_ptr<Drawable>>{};
    res.push_back(std::make_unique<Polygon>(std::move(result), fillInner,
                                            fillColor, stroke, width,
                                            strokeColor, lineType));
    return res;
}

std::optional<std::pair<Point, std::function<void(Point)>>>
Polygon::getModificationHandler(const Camera &camera, Point p)
{
    double tol = Drawable::HIT_TEST_TOL / camera.scale;
    for (auto &c : verts) {
        if (c.distanceSqr(p) <= tol * tol) {
            return std::make_pair(c, [&](Point v) { c = v; });
        }
    }
    return std::nullopt;
}