#include "bspline.h"

#include <numeric>
#include <span>

#include "line.h"

void BSpline::calKnots()
{
    int n = ctrlPts.size();
    int m = n + degree;
    knots.resize(m + 1);

    std::fill(knots.begin(), knots.begin() + degree + 1, 0);
    std::iota(knots.begin() + degree + 1, knots.begin() + m - degree, 1);
    std::fill(knots.begin() + m - degree, knots.end(), n - degree);
}

// Use external pre-allocated d buffer to avoid frequent allocation
static Point evalBSpline(std::span<const Point> localCtrl,
                         std::span<const double> localKnots, int degree,
                         double u, std::span<Point> d)
{
    for (int j = 0; j <= degree; ++j) {
        d[j] = localCtrl[j];
    }

    for (int r = 1; r <= degree; ++r) {
        for (int j = degree; j >= r; --j) {
            double denominator = localKnots[j + 1 + degree - r] - localKnots[j];

            double alpha = 0.0;
            if (std::abs(denominator) > 1e-12) {
                alpha = (u - localKnots[j]) / denominator;
            }

            d[j] = d[j - 1] * (1.0 - alpha) + d[j] * alpha;
        }
    }

    return d[degree];
}

template <typename F>
static bool forEachBSplineSegment(std::span<const Point> pts,
                                  std::span<const double> knots, int degree,
                                  int steps, F &&f)
{
    if (pts.size() <= degree) {
        return true;
    }

    double uStart = knots[degree];
    double uEnd = knots[pts.size()];

    if (uEnd <= uStart) {
        return true;
    }

    int n = pts.size() - 1;
    int k = degree;
    std::vector<Point> d(degree + 1, Point{0, 0});
    Point prev = evalBSpline(pts, knots, degree, uStart, d);
    for (int i = 1; i <= steps; ++i) {
        double u = uStart + (uEnd - uStart) * (static_cast<double>(i) / steps);
        while (k < n && u >= knots[k + 1]) {
            ++k;
        }
        auto localCtrl =
            std::span<const Point>(pts).subspan(k - degree, degree + 1);
        auto localKnots =
            std::span<const double>(knots).subspan(k - degree, 2 * degree + 1);
        Point curr = evalBSpline(localCtrl, localKnots, degree, u, d);
        if (!f(prev, curr)) {
            return false;
        }
        prev = curr;
    }
    return true;
}

void BSpline::drawStroke(RenderTarget &rt) const
{
    forEachBSplineSegment(ctrlPts, knots, degree, 50 * (ctrlPts.size() - 1),
                          [&](Point start, Point end) {
                              rt.drawLine(start, end, width, color, lineType);
                              return true;
                          });
}

void BSpline::drawSelHighlight(RenderTarget &rt) const
{
    forEachBSplineSegment(
        ctrlPts, knots, degree, 50 * (ctrlPts.size() - 1),
        [&](Point start, Point end) {
            rt.drawLine(start, end,
                        width + Drawable::SEL_HIGHLIGHT_WIDTH / rt.camera.scale,
                        Drawable::SEL_HIGHLIGHT_COLOR, LineType{});
            return true;
        });
    for (auto p : ctrlPts) {
        rt.drawCross(p);
    }
}

bool BSpline::hitTest(const Camera &camera, Point p) const
{
    double tol = Drawable::HIT_TEST_TOL / camera.scale + width / 2.0;
    return !forEachBSplineSegment(
        ctrlPts, knots, degree, 20 * (ctrlPts.size() - 1),
        [&](Point start, Point end) {
            return Geometry::pointToSegmentDistanceSqr(p, start, end) >
                   tol * tol;
        });
}

Point BSpline::centroid() const { return Geometry::centroid(ctrlPts); }

void BSpline::transform(Transform t)
{
    for (auto &p : ctrlPts) {
        p = p.transform(t);
    }
    width *= t.getScale();
}

std::vector<std::unique_ptr<Drawable>>
BSpline::clip(std::span<const Point> clipPoints)
{
    std::vector<std::unique_ptr<Drawable>> res;
    forEachBSplineSegment(
        ctrlPts, knots, degree, 50 * (ctrlPts.size() - 1),
        [&](Point start, Point end) {
            auto seg = Geometry::clipLineSegment(start, end, clipPoints);
            if (seg && seg->first != seg->second) {
                res.push_back(std::make_unique<Line>(seg->first, seg->second,
                                                     width, color, lineType));
            }
            return true;
        });
    return res;
}

AABB BSpline::bounds() const
{
    if (ctrlPts.empty()) {
        return AABB{0, 0, 0, 0};
    }

    if (ctrlPts.size() <= degree) {
        return AABB{ctrlPts};
    }

    AABB bound{ctrlPts[0], ctrlPts[0]};
    forEachBSplineSegment(ctrlPts, knots, degree, 10 * (ctrlPts.size() - 1),
                          [&](Point, Point end) {
                              bound.expand(end);
                              return true;
                          });

    return bound;
}

std::optional<std::pair<Point, std::function<void(Point)>>>
BSpline::getModificationHandler(const Camera &camera, Point p)
{
    double tol = Drawable::HIT_TEST_TOL / camera.scale;
    for (auto &c : ctrlPts) {
        if (c.distanceSqr(p) <= tol * tol) {
            return std::make_pair(c, [&](Point v) { c = v; });
        }
    }
    return std::nullopt;
}