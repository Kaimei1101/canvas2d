#include "bspline.h"

#include <span>

#include "line.h"

void BSpline::calKnots()
{
    int n = ctrlPts.size();
    int m = n + degree;
    knots.resize(m + 1);

    for (int i = 0; i <= degree; ++i)
        knots[i] = 0.0;
    for (int i = m - degree; i <= m; ++i)
        knots[i] = static_cast<double>(n - degree);
    for (int i = degree + 1; i < m - degree; ++i) {
        knots[i] = knots[i - 1] + 1.0;
    }
}

Point evalBSpline(std::span<const Point> localCtrl,
                  std::span<const double> localKnots, int degree, double u)
{
    std::vector<Point> d(degree + 1, Point{0, 0});

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

            d[j].x = (1.0 - alpha) * d[j - 1].x + alpha * d[j].x;
            d[j].y = (1.0 - alpha) * d[j - 1].y + alpha * d[j].y;
        }
    }

    return d[degree];
}

template <typename F>
bool forEachBSplineSegment(std::span<const Point> pts,
                           std::span<const double> knots, int degree, int steps,
                           F &&f)
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
    Point prev = evalBSpline(pts, knots, degree, uStart);
    for (int i = 1; i <= steps; ++i) {
        double u = uStart + (uEnd - uStart) * (static_cast<double>(i) / steps);
        while (k < n && u >= knots[k + 1]) {
            ++k;
        }
        auto localCtrl =
            std::span<const Point>(pts).subspan(k - degree, degree + 1);
        auto localKnots =
            std::span<const double>(knots).subspan(k - degree, 2 * degree + 1);
        Point curr = evalBSpline(localCtrl, localKnots, degree, u);
        if (!f(prev, curr)) {
            return false;
        }
        prev = curr;
    }
    return true;
}

void BSpline::draw(RenderTarget &rt) const
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
                        width + Drawable::selHighlightWidth / rt.camera.scale,
                        Drawable::selHighlightColor, LineType{});
            return true;
        });
}

bool BSpline::hitTest(const Camera &camera, Point p) const
{
    double tol = Drawable::hitTestTol / camera.scale + width / 2.0;
    return !forEachBSplineSegment(
        ctrlPts, knots, degree, 50 * (ctrlPts.size() - 1),
        [&](Point start, Point end) {
            return Geometry::pointToSegmentDistanceSqr(p, start, end) >
                   tol * tol;
        });
}

Point BSpline::centroid() const
{
    Point sum{0, 0};
    for (const auto &v : ctrlPts) {
        sum = sum + v;
    }
    return ctrlPts.empty() ? sum : sum / static_cast<double>(ctrlPts.size());
}

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