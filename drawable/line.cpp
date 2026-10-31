#include "line.h"

#include "geometry.h"
#include "rendertarget.h"

Line::Line(Point start, Point end, double width, RGBA color, LineType lineType)
    : start{start}, end{end}, width{width}, color{color}, lineType{lineType}
{
}

void Line::drawStroke(RenderTarget &rt) const
{
    rt.drawLine(start, end, width, color, lineType);
}

void Line::drawSelHighlight(RenderTarget &rt) const
{
    rt.drawLine(start, end,
                width + Drawable::SEL_HIGHLIGHT_WIDTH / rt.camera.scale,
                Drawable::SEL_HIGHLIGHT_COLOR, LineType{});
    rt.drawCross(start);
    rt.drawCross(end);
}

bool Line::hitTest(const Camera &camera, Point p) const
{
    double tol = Drawable::HIT_TEST_TOL / camera.scale + width / 2.0;
    double dist = Geometry::pointToSegmentDistanceSqr(p, start, end);
    return dist <= tol * tol;
}

void Line::transform(Transform t)
{
    start = start.transform(t);
    end = end.transform(t);
    width *= t.getScale();
}

AABB Line::bounds() const { return AABB{start, end}; }

std::vector<std::unique_ptr<Drawable>>
Line::clip(std::span<const Point> clipPoints)
{
    auto seg = Geometry::clipLineSegment(start, end, clipPoints);
    if (seg && seg->first != seg->second) {
        auto res = std::vector<std::unique_ptr<Drawable>>{};
        res.push_back(std::make_unique<Line>(seg->first, seg->second, width,
                                             color, lineType));
        return res;
    }
    return {};
}

std::optional<std::pair<Point, std::function<void(Point)>>>
Line::getModificationHandler(const Camera &camera, Point p)
{
    double tol = Drawable::HIT_TEST_TOL / camera.scale;
    for (auto *pc : {&start, &end}) {
        auto &c = *pc;
        if (c.distanceSqr(p) <= tol * tol) {
            return std::make_pair(c, [&](Point v) { c = v; });
        }
    }
    return std::nullopt;
}