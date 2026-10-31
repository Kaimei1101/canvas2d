#include "beziercurve.h"

#include "geometry.h"
#include "line.h"

// Use external pre-allocated d buffer to avoid frequent allocation
inline Point evalBezier(std::span<const Point> ctrlPts, double t,
                        std::span<Point> d)
{
    int n = static_cast<int>(ctrlPts.size());

    for (int i = 0; i < n; ++i) {
        d[i] = ctrlPts[i];
    }

    for (int r = 1; r < n; ++r) {
        for (int i = 0; i < n - r; ++i) {
            d[i] = d[i] * (1 - t) + d[i + 1] * t;
        }
    }
    return d[0];
}

template <typename F>
bool forEachBezierSegment(std::span<const Point> ctrlPts, int steps, F &&f)
{
    if (ctrlPts.size() < 2) {
        return true;
    }

    double tStart = 0.0;
    double tEnd = 1.0;
    double dt = (tEnd - tStart) / steps;
    std::vector d(ctrlPts.size(), Point{0, 0});

    Point prev = evalBezier(ctrlPts, tStart, d);
    for (int i = 1; i <= steps; ++i) {
        double t = tStart + i * dt;
        Point curr = evalBezier(ctrlPts, t, d);

        if (!f(prev, curr)) {
            return false;
        }
        prev = curr;
    }
    return true;
}

void BezierCurve::drawStroke(RenderTarget &rt) const
{

    forEachBezierSegment(ctrlPts, 50 * (ctrlPts.size() - 1),
                         [&](Point start, Point end) {
                             rt.drawLine(start, end, width, color, lineType);
                             return true;
                         });
}

void BezierCurve::drawSelHighlight(RenderTarget &rt) const
{
    forEachBezierSegment(
        ctrlPts, 50 * (ctrlPts.size() - 1), [&](Point start, Point end) {
            rt.drawLine(start, end,
                        width + Drawable::SEL_HIGHLIGHT_WIDTH / rt.camera.scale,
                        Drawable::SEL_HIGHLIGHT_COLOR, LineType{});
            return true;
        });
    for (auto p : ctrlPts) {
        rt.drawCross(p);
    }
}

bool BezierCurve::hitTest(const Camera &camera, Point p) const
{
    double tol = Drawable::HIT_TEST_TOL / camera.scale + width / 2.0;
    return !forEachBezierSegment(
        ctrlPts, 20 * (ctrlPts.size() - 1), [&](Point start, Point end) {
            return Geometry::pointToSegmentDistanceSqr(p, start, end) >
                   tol * tol;
        });
}

Point BezierCurve::centroid() const { return Geometry::centroid(ctrlPts); }

void BezierCurve::transform(Transform t)
{
    for (auto &p : ctrlPts) {
        p = p.transform(t);
    }
    width *= t.getScale();
}

std::vector<std::unique_ptr<Drawable>>
BezierCurve::clip(std::span<const Point> clipPoints)
{
    std::vector<std::unique_ptr<Drawable>> res;
    forEachBezierSegment(
        ctrlPts, 50 * (ctrlPts.size() - 1), [&](Point start, Point end) {
            auto seg = Geometry::clipLineSegment(start, end, clipPoints);
            if (seg && seg->first != seg->second) {
                res.push_back(std::make_unique<Line>(seg->first, seg->second,
                                                     width, color, lineType));
            }
            return true;
        });
    return res;
}

AABB BezierCurve::bounds() const
{
    if (ctrlPts.empty()) {
        return AABB{0, 0, 0, 0};
    }

    AABB bound{ctrlPts[0], ctrlPts[0]};
    forEachBezierSegment(ctrlPts, 10 * (ctrlPts.size() - 1),
                         [&](Point, Point end) {
                             bound.expand(end);
                             return true;
                         });

    return bound;
}

std::optional<std::pair<Point, std::function<void(Point)>>>
BezierCurve::getModificationHandler(const Camera &camera, Point p)
{
    double tol = Drawable::HIT_TEST_TOL / camera.scale;
    for (auto &c : ctrlPts) {
        if (c.distanceSqr(p) <= tol * tol) {
            return std::make_pair(c, [&](Point v) { c = v; });
        }
    }
    return std::nullopt;
}