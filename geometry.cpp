#include <geometry.h>

namespace Geometry
{
// 叉积 (b-a) × (c-a)
double cross(Point a, Point b, Point c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

// 有符号面积的两倍；y 向下坐标系中 <0 对应屏幕逆时针绕序
double signedArea2(std::span<Point> p)
{
    double s = 0;
    for (std::size_t i = 0; i < p.size(); ++i) {
        const Point &a = p[i];
        const Point &b = p[(i + 1) % p.size()];
        s += a.x * b.y - b.x * a.y;
    }
    return s;
}

double pointToSegmentDistanceSqr(Point p, Point a, Point b)
{
    auto ab = b - a;
    auto [dx, dy] = ab;
    double len2 = dx * dx + dy * dy;

    // 如果线段退化为一个点，直接返回点到该点的距离
    if (len2 < 1e-12) {
        auto [px, py] = p - a;
        return px * px + py * py;
    }

    // 计算投影参数 t：AP 在 AB 上的投影长度占 AB 长度的比例
    // t = (AP · AB) / |AB|^2
    double t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / len2;

    // 保证最近点在线段上，而不是无限长直线上
    t = std::clamp(t, 0.0, 1.0);

    auto closest = a + ab * t;
    auto [diffX, diffY] = p - closest;
    return diffX * diffX + diffY * diffY;
}

Point centroid(std::span<const Point> p)
{
    Point s{0, 0};
    for (const auto &q : p) {
        s = s + q;
    }
    return p.empty() ? s : s / static_cast<double>(p.size());
}

int convexInsertEdge(const std::vector<Point> &pts, Point p)
{
    const std::size_t n = pts.size();
    if (n < 3) {
        return -1;
    }
    const Point c = centroid(pts);
    int outside = -1;
    for (std::size_t i = 0; i < n; ++i) {
        const Point &a = pts[i];
        const Point &b = pts[(i + 1) % n];
        const double sc = cross(a, b, c);
        const double sp = cross(a, b, p);
        if (sc * sp < 0) { // 严格异侧
            if (outside != -1) {
                return -1; // 多于一条边异侧：不合法
            }
            outside = static_cast<int>(i);
        }
    }
    return outside; // -1 表示点在内部/边上，同样不合法
}

bool makeTriangle(Point a, Point b, Point p, std::vector<Point> &out)
{
    if (std::abs(cross(a, b, p)) < 1e-9) {
        return false;
    }
    out = {a, b, p};
    if (signedArea2(out) > 0) {
        std::swap(out[1], out[2]); // 统一为屏幕逆时针绕序
    }
    return true;
}

double edgeIntersectParam(Point s, Point e, Point a, Point b)
{
    const double ux = e.x - s.x;
    const double uy = e.y - s.y;
    const double vx = b.x - a.x;
    const double vy = b.y - a.y;
    const double denom = ux * vy - uy * vx;
    return ((a.x - s.x) * vy - (a.y - s.y) * vx) / denom;
}

std::vector<Point> clipPolygon(std::vector<Point> subject,
                               std::span<const Point> clip)
{
    const std::size_t m = clip.size();
    for (std::size_t i = 0; i < m && !subject.empty(); ++i) {
        const Point a = clip[i];
        const Point b = clip[(i + 1) % m];
        std::vector<Point> in = std::move(subject);
        subject.clear();
        // 屏幕逆时针绕序：内部满足 cross(a,b,p) <= 0
        auto inside = [&](Point p) { return cross(a, b, p) <= 1e-9; };
        Point s = in.back();
        bool sin = inside(s);
        for (const Point &e : in) {
            const bool ein = inside(e);
            if (sin) {
                if (ein) {
                    subject.push_back(e); // 内->内
                } else {
                    const double t = edgeIntersectParam(s, e, a, b);
                    subject.push_back(
                        Point{s.x + t * (e.x - s.x), s.y + t * (e.y - s.y)});
                }
            } else if (ein) {
                const double t = edgeIntersectParam(s, e, a, b);
                subject.push_back(
                    Point{s.x + t * (e.x - s.x), s.y + t * (e.y - s.y)});
                subject.push_back(e); // 外->内
            }
            s = e;
            sin = ein;
        }
    }
    return subject;
}

std::vector<Point> circleToPolygon(Point center, double radius, int seg)
{
    std::vector<Point> pts;
    pts.reserve(seg);
    for (int i = 0; i < seg; ++i) {
        const double t = 2.0 * std::numbers::pi * i / seg;
        pts.push_back(Point{center.x + radius * std::cos(t),
                            center.y + radius * std::sin(t)});
    }
    return pts;
}

std::optional<std::pair<Point, Point>>
clipLineSegment(Point s, Point e, std::span<const Point> clip)
{
    double t0 = 0.0;
    double t1 = 1.0;
    const Point d = e - s;
    for (std::size_t i = 0; i < clip.size(); ++i) {
        const Point a = clip[i];
        const Point b = clip[(i + 1) % clip.size()];
        // cross(a,b,P(t)) = (b-a)×(P(t)-a) = C0 + t*C1
        const double C0 = cross(a, b, s);
        const double C1 = (b.x - a.x) * d.y - (b.y - a.y) * d.x;
        // 内部条件 C0 + t*C1 <= 0
        if (std::abs(C1) < 1e-12) {
            if (C0 > 1e-9) {
                return std::nullopt; // 平行且在外侧
            }
        } else if (C1 > 0.0) {
            t1 = std::min(t1, -C0 / C1); // 出边
        } else {
            t0 = std::max(t0, -C0 / C1); // 入边
        }
    }
    if (t0 > t1 + 1e-9) {
        return std::nullopt;
    }
    t0 = std::max(t0, 0.0);
    t1 = std::min(t1, 1.0);
    if (t1 - t0 < 1e-9) {
        return std::nullopt;
    }
    return std::make_pair(Point{s.x + t0 * d.x, s.y + t0 * d.y},
                          Point{s.x + t1 * d.x, s.y + t1 * d.y});
}

std::vector<double> circleEdgeAngles(Point center, double radius, Point a,
                                     Point b)
{
    std::vector<double> out;
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    const double fx = a.x - center.x;
    const double fy = a.y - center.y;
    const double A = dx * dx + dy * dy;
    if (A < 1e-12) {
        return out;
    }
    const double B = 2.0 * (fx * dx + fy * dy);
    const double C = fx * fx + fy * fy - radius * radius;
    const double disc = B * B - 4.0 * A * C;
    if (disc < 0.0) {
        return out;
    }
    const double sq = std::sqrt(disc);
    for (const double t : {(-B - sq) / (2.0 * A), (-B + sq) / (2.0 * A)}) {
        const Point p{a.x + t * dx, a.y + t * dy};
        out.push_back(std::atan2(p.y - center.y, p.x - center.x));
    }
    return out;
}

std::vector<ArcSeg> clipArc(Point center, double radius, Point ep1, Point ep2,
                            int side, std::span<const Point> clip)
{
    std::vector<ArcSeg> result;
    if (radius <= 0.0 || ep1 == ep2) {
        return result;
    }
    const int keep = side >= 0 ? 1 : -1;
    const double th1 = std::atan2(ep1.y - center.y, ep1.x - center.x);
    const double th2 = std::atan2(ep2.y - center.y, ep2.x - center.x);
    auto sample = [&](double th) {
        return Point{center.x + radius * std::cos(th),
                     center.y + radius * std::sin(th)};
    };
    auto onKeepSide = [&](double th) {
        const Point p = sample(th);
        const double cr =
            (ep2.x - ep1.x) * (p.y - ep1.y) - (ep2.y - ep1.y) * (p.x - ep1.x);
        return cr * keep > 1e-9;
    };

    // 确定原弧方向：th1→th2 沿 +Δ 还是 -Δ（取各自中点判定哪段在保留侧）
    double d1 = th2 - th1;
    while (d1 < 0.0) {
        d1 += 2.0 * std::numbers::pi;
    }
    while (d1 >= 2.0 * std::numbers::pi) {
        d1 -= 2.0 * std::numbers::pi;
    }
    double thStart, thEnd;
    if (onKeepSide(th1 + d1 * 0.5)) {
        thStart = th1;
        thEnd = th1 + d1; // 递增方向
    } else {
        thStart = th1;
        thEnd = th1 - (2.0 * std::numbers::pi - d1); // 递减方向
    }
    // 统一成递增区间 [thStart, thEnd]（cos/sin 周期，交换端点不影响采样点）
    if (thEnd < thStart) {
        std::swap(thStart, thEnd);
    }

    // 收集分割角：原弧两个端点 + 窗口每条边与圆的交点（平移到 [thStart,
    // thEnd]）
    std::vector<double> cuts{thStart, thEnd};
    for (std::size_t i = 0; i < clip.size(); ++i) {
        const Point a = clip[i];
        const Point b = clip[(i + 1) % clip.size()];
        for (double ang : circleEdgeAngles(center, radius, a, b)) {
            double angIn = ang;
            while (angIn < thStart - 1e-9) {
                angIn += 2.0 * std::numbers::pi;
            }
            while (angIn > thEnd + 2.0 * std::numbers::pi + 1e-9) {
                angIn -= 2.0 * std::numbers::pi;
            }
            // 在 (thStart, thEnd] 或 (thStart-2π, thEnd-2π]
            // 两个候选副本中取落在区间内的
            for (double cand : {angIn, angIn - 2.0 * std::numbers::pi}) {
                if (cand >= thStart - 1e-9 && cand <= thEnd + 1e-9) {
                    cuts.push_back(cand);
                }
            }
        }
    }
    std::sort(cuts.begin(), cuts.end());
    cuts.erase(
        std::unique(cuts.begin(), cuts.end(),
                    [](double a, double b) { return std::abs(a - b) < 1e-6; }),
        cuts.end());

    // 凸多边形内部判定
    auto inWindow = [&](Point p) {
        for (std::size_t i = 0; i < clip.size(); ++i) {
            if (cross(clip[i], clip[(i + 1) % clip.size()], p) > 1e-9) {
                return false;
            }
        }
        return true;
    };

    for (std::size_t i = 0; i + 1 < cuts.size(); ++i) {
        const double a = cuts[i];
        const double b = cuts[i + 1];
        if (b - a < 1e-6) {
            continue;
        }
        if (inWindow(sample((a + b) * 0.5))) {
            // 新割线 ep1=sample(a) → ep2=sample(b) 与原割线不同，原 side
            // 不再适用； 取弧段中点独立算 side：让 drawArc 保留这段短弧本身
            const Point aPt = sample(a);
            const Point bPt = sample(b);
            const Point midPt = sample((a + b) * 0.5);
            const double cr = (bPt.x - aPt.x) * (midPt.y - aPt.y) -
                              (bPt.y - aPt.y) * (midPt.x - aPt.x);
            const int sideNew = cr >= 0.0 ? 1 : -1;
            result.push_back({aPt, bPt, sideNew});
        }
    }
    return result;
}

std::vector<Pixel> traceContour(const MaskBuffer &mask)
{
    static constexpr Pixel DIRS[8] = {{0, -1}, {1, -1}, {1, 0},  {1, 1},
                                      {0, 1},  {-1, 1}, {-1, 0}, {-1, -1}};

    int w = mask.getWidth();
    int h = mask.getHeight();

    Pixel start = {-1, -1};
    auto data = mask.data();
    for (int i = 0; i < static_cast<size_t>(w) * h; ++i) {
        if (data[i] != 0) {
            start = {i % w, i / w};
            goto FIND;
        }
    }
    return {};

FIND:
    std::vector<Pixel> contour;
    Pixel curr = start;
    int prevDir = 1; // Pixel {1, -1}

    do {
        contour.push_back(curr);

        bool found = false;
        for (int i = 1; i <= 8; ++i) {
            int dirIdx = (prevDir + i) % 8;
            auto [nx, ny] = curr + DIRS[dirIdx];

            if (nx >= 0 && nx < w && ny >= 0 && ny < h && mask(nx, ny) != 0) {
                prevDir = (dirIdx + 4) % 8;
                curr = {nx, ny};
                found = true;
                break;
            }
        }
        if (!found) {
            break;
        }
    } while (curr != start);

    return contour;
}
}; // namespace Geometry