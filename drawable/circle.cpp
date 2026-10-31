#include "circle.h"

#include <algorithm>

#include "geometry.h"
#include "polygon.h"

Circle::Circle(Point center, double radius, bool fill, RGBA fillColor,
               bool stroke, double width, RGBA strokeColor, LineType lineType)
    : center{center}, radius{radius},
      arcData{Point{0, 0}, Point{0, 0}, 1, true}, // 整圆：无端点
      width{width}, strokeColor{strokeColor}, fillColor{fillColor},
      stroke{stroke}, fill{fill}, lineType{lineType}
{
}

Circle::Circle(Point center, Point edge, bool fill, RGBA fillColor, bool stroke,
               double width, RGBA strokeColor, LineType lineType)
    : Circle{center, center.distance(edge), fill,    fillColor, stroke,
             width,  strokeColor,           lineType}
{
}

Circle::Circle(Point center, double radius, Point ep1, Point ep2, int side,
               bool stroke, double width, RGBA strokeColor, LineType lineType)
    : center{center}, radius{radius},
      arcData{ep1, ep2, side >= 0 ? 1 : -1, false}, width{width},
      strokeColor{strokeColor}, fillColor{0, 0, 0}, stroke{stroke}, fill{false},
      lineType{lineType}
{
}

std::optional<Circle> Circle::fromThreePoints(Point p1, Point p2, Point p3,
                                              bool stroke, double width,
                                              RGBA strokeColor,
                                              LineType lineType)
{
    auto center = Geometry::circleFrom3Points(p1, p2, p3);
    if (!center) {
        return std::nullopt;
    }
    return Circle{center->first,
                  center->second,
                  p1,
                  p3,
                  Geometry::cross(p1, p3, p2) > 0 ? 1 : -1,
                  stroke,
                  width,
                  strokeColor,
                  lineType};
}

void Circle::drawStroke(RenderTarget &rt) const
{
    if (stroke) {
        if (arcData.isFullCircle) {
            rt.drawCircleOutline(center, radius, width, strokeColor, lineType);
        } else {
            rt.drawArc(center, radius, arcData.ep1, arcData.ep2, arcData.side,
                       width, strokeColor, lineType);
        }
    }
}

void Circle::fillInner(RenderTarget &rt) const
{
    if (fill && arcData.isFullCircle) {
        rt.fillCircle(center, radius, fillColor);
    }
}

void Circle::drawSelHighlight(RenderTarget &rt) const
{
    auto highlightWidth =
        width + Drawable::SEL_HIGHLIGHT_WIDTH / rt.camera.scale;
    if (arcData.isFullCircle) {
        rt.drawCircleOutline(center, radius, highlightWidth,
                             Drawable::SEL_HIGHLIGHT_COLOR, LineType{});
        for (auto p :
             {Point{radius, -1}, {0, radius}, {-radius, 0}, {0, -radius}}) {
            rt.drawCross(center + p);
        }
    } else {
        for (auto p : {arcData.ep1, arcData.ep2, getArcRadiusModPoint()}) {
            rt.drawCross(p);
        }
        rt.drawArc(center, radius, arcData.ep1, arcData.ep2, arcData.side,
                   highlightWidth, Drawable::SEL_HIGHLIGHT_COLOR, LineType{});
    }
}

bool Circle::hitTest(const Camera &camera, Point p) const
{
    double tol = Drawable::HIT_TEST_TOL / camera.scale + width / 2.0;
    double distToCenter = p.distance(center);

    if (std::abs(distToCenter - radius) <= tol) {
        if (isFullCircle()) {
            return true;
        }
        return Geometry::cross(arcData.ep1, arcData.ep2, p) * arcData.side >= 0;
    }

    if (isFilled() && distToCenter <= radius) {
        return true;
    }
    return false;
}

void Circle::transform(Transform t)
{
    center = center.transform(t);
    auto scale = t.getScale();
    radius *= scale;
    width *= scale;
}

AABB Circle::bounds() const
{
    if (isFullCircle()) {
        return AABB{center - Point{radius, radius},
                    center + Point{radius, radius}};
    }

    return Geometry::arcBounds(
        center, radius,
        Geometry::ArcSeg{arcData.ep1, arcData.ep2, arcData.side});
}

std::vector<std::unique_ptr<Drawable>>
Circle::clip(std::span<const Point> clipPoints)
{
    if (arcData.isFullCircle) {
        auto verts = Geometry::circleToPolygon(center, radius);
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

    std::vector<Geometry::ArcSeg> segs = Geometry::clipArc(
        center, radius, arcData.ep1, arcData.ep2, arcData.side, clipPoints);
    if (segs.empty()) {
        return {};
    }
    std::vector<std::unique_ptr<Drawable>> res;
    for (const auto &sg : segs) {
        res.push_back(std::make_unique<Circle>(center, radius, sg.a, sg.b,
                                               sg.side, stroke, width,
                                               strokeColor, lineType));
    }
    return res;
}

std::optional<std::pair<Point, std::function<void(Point)>>>
Circle::getModificationHandler(const Camera &camera, Point p)
{
    double tol = Drawable::HIT_TEST_TOL / camera.scale;
    if (arcData.isFullCircle) {
        for (auto d :
             {Point{radius, 0}, {0, radius}, {-radius, 0}, {0, -radius}}) {
            auto c = center + d;
            if (c.distanceSqr(p) <= tol * tol) {
                return std::make_pair(
                    c, [&](Point mod) { radius = mod.distance(center); });
            }
        }
    } else {
        if (arcData.ep1.distanceSqr(p) <= tol * tol) {
            return std::make_pair(p, [&](Point mod) {
                auto newEp =
                    Geometry::intersectCircle(mod, arcData.ep2, center, radius);
                if (!newEp) {
                    return;
                }
                arcData.ep1 = newEp->first;
                arcData.ep2 = newEp->second;
            });
        }
        if (arcData.ep2.distanceSqr(p) <= tol * tol) {
            return std::make_pair(p, [&](Point mod) {
                auto newEp =
                    Geometry::intersectCircle(arcData.ep1, mod, center, radius);
                if (!newEp) {
                    return;
                }
                arcData.ep1 = newEp->first;
                arcData.ep2 = newEp->second;
            });
        }
        auto c = getArcRadiusModPoint();
        if (c.distanceSqr(p) <= tol * tol) {
            return std::make_pair(c, [&](Point mod) {
                auto newRadius = mod.distance(center);
                auto newEp = Geometry::intersectCircle(arcData.ep1, arcData.ep2,
                                                       center, newRadius);
                if (!newEp) {
                    return;
                }
                radius = newRadius;
                arcData.ep1 = newEp->first;
                arcData.ep2 = newEp->second;
            });
        }
    }
    return std::nullopt;
}

Point Circle::getArcRadiusModPoint() const
{
    auto secant = arcData.ep2 - arcData.ep1;
    auto normal = Point{secant.y, -secant.x};
    auto len = std::hypot(normal.x, normal.y);
    if (len < 1e-9) {
        return (arcData.ep1 + arcData.ep2) / 2;
    }
    return center + normal / len * radius * (arcData.side > 0 ? -1 : 1);
}