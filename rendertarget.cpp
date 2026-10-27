#include <algorithm>
#include <cmath>
#include <map>
#include <ranges>

#include "rendertarget.h"

RenderTarget::RenderTarget(int width, int height) : pixels{width, height} {}

int RenderTarget::getWidth() const { return pixels.getWidth(); }

int RenderTarget::getHeight() const { return pixels.getHeight(); }

const PixelBuffer &RenderTarget::getPixels() const { return pixels; }

void RenderTarget::resize(int width, int height)
{
    pixels = PixelBuffer{width, height};
}

void RenderTarget::clear() { pixels.clear(); }

void RenderTarget::setPixelUnchecked(Pixel p, RGBA color)
{
    auto &dst = pixels(p.x, p.y);
    if (color.a == 255) {
        dst = color;
        return;
    }
    if (color.a == 0) {
        return;
    }

    // 混合公式（整数运算）：
    // out = (src * srcA + dst * (255 - srcA)) / 255
    // 加上 127 是为了实现四舍五入，减少精度损失
    const uint32_t invA = 255 - color.a;

    dst.r =
        static_cast<uint8_t>((color.r * color.a + dst.r * invA + 126) / 255);
    dst.g =
        static_cast<uint8_t>((color.g * color.a + dst.g * invA + 127) / 255);
    dst.b =
        static_cast<uint8_t>((color.b * color.a + dst.b * invA + 127) / 255);
}

void RenderTarget::setPixel(Pixel p, RGBA color)
{
    auto [x, y] = p;
    if (x < 0 || x >= getWidth() || y < 0 || y >= getHeight()) {
        return;
    }
    setPixelUnchecked(p, color);
}

void RenderTarget::fillSpan(int y, int x0, int x1, RGBA color)
{
    if (y < 0 || y >= getHeight()) {
        return;
    }

    x0 = std::max(x0, 0);
    x1 = std::min(x1, getWidth() - 1);
    if (x0 > x1) {
        return;
    }

    for (int x = x0; x <= x1; ++x) {
        setPixelUnchecked(Pixel{x, y}, color);
    }
}

void RenderTarget::fillSpanVertical(int x, int y0, int y1, RGBA color)
{
    if (x < 0 || x >= getWidth()) {
        return;
    }

    y0 = std::max(y0, 0);
    y1 = std::min(y1, getHeight() - 1);
    if (y0 > y1) {
        return;
    }

    for (int y = y0; y <= y1; ++y) {
        setPixelUnchecked(Pixel{x, y}, color);
    }
}

struct LineTypeCursor {
    std::array<int, 4> pat;
    int cursor;
    int patIndex;

    LineTypeCursor(LineType lineType) : cursor{0}, patIndex{0}
    {
        for (int i = 0; i < 4; i++) {
            pat[i] = std::round(std::clamp(
                lineType.pat[i], 2.0,
                static_cast<double>(std::numeric_limits<int>::max())));
        }
    }

    bool next()
    {
        if (cursor++ >= pat[patIndex]) {
            patIndex = (patIndex + 1) % 4;
            cursor = 1;
        }
        return patIndex % 2 == 0;
    }
};

void RenderTarget::drawLine(Point startWorld, Point endWorld, double widthWorld,
                            RGBA color, LineType lineType)
{
    auto start = camera.worldToCanvas(startWorld);
    auto end = camera.worldToCanvas(endWorld);

    auto [dx, dy] = end - start;

    // Adjust width for sloping line
    auto rawWidth = widthWorld * camera.scale;
    double len = std::sqrt(dx * dx + dy * dy);
    double widthFactor = len / std::max(std::abs(dx), std::abs(dy));
    lineType = lineType.forWidth(std::max(1.0, std::round(rawWidth)))
                   .scale(1 / widthFactor);
    if (rawWidth > 1.5) {
        rawWidth *= widthFactor;
    }
    int width = std::max(1, static_cast<int>(std::round(rawWidth)));
    int halfWidth = width / 2;

    // Culling by Liang-Barsky algorithm
    double t0 = 0.0;
    double t1 = 1.0;

    auto clip = [&](double p, double q) -> bool {
        if (p == 0.0) {
            return q >= 0.0;
        }
        double r = q / p;
        if (p < 0.0) {
            if (r > t1) {
                return false;
            }
            if (r > t0) {
                t0 = r;
            }
        } else {
            if (r < t0) {
                return false;
            }
            if (r < t1) {
                t1 = r;
            }
        }
        return true;
    };

    if (!clip(-dx, start.x + halfWidth) ||
        !clip(dx, getWidth() + halfWidth - start.x) ||
        !clip(-dy, start.y + halfWidth) ||
        !clip(dy, getHeight() + halfWidth - start.y)) {
        return;
    }

    end = Point{start.x + t1 * dx, start.y + t1 * dy};
    start = Point{start.x + t0 * dx, start.y + t0 * dy};

    drawLineImpl(Pixel{start}, Pixel{end}, width, color, lineType);
}

void RenderTarget::drawLineImpl(Pixel start, Pixel end, int width, RGBA color,
                                LineType lineType)
{
    const auto dx = std::abs(start.x - end.x);
    const auto dy = -std::abs(start.y - end.y);
    auto sx = start.x > end.x ? -1 : 1;
    auto sy = start.y > end.y ? -1 : 1;
    int err = dx + dy;

    LineTypeCursor cursor{lineType};
    for (auto i = 0; i <= std::max(dx, -dy); ++i) {
        if (cursor.next()) {
            if (dx > -dy) {
                fillSpanVertical(start.x, start.y - width / 2,
                                 start.y + width - width / 2 - 1, color);
            } else {
                fillSpan(start.y, start.x - width / 2,
                         start.x + width - width / 2 - 1, color);
            }
        }

        int e2 = err + err;
        if (e2 >= dy) {
            err += dy;
            start.x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            start.y += sy;
        }
    }
}

template <typename F> void bresenhamCircle(int radius, F &&brush)
{
    int x = radius;
    int y = 0;
    int err = 1 - radius;

    while (x >= y) {
        brush(Pixel{x, y});

        ++y;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            --x;
            err += 2 * (y - x) + 1;
        }
    }
}

void RenderTarget::drawCircleOutline(Point center, double radius, double width,
                                     RGBA color, LineType lineType)
{
    drawCircleOutlineImpl(
        camera.worldToPixel(center), std::round(radius * camera.scale),
        std::max(1, static_cast<int>(std::round(width * camera.scale))), color,
        lineType);
}

void RenderTarget::drawCircleOutlineImpl(Pixel center, int radius, int width,
                                         RGBA color, LineType lineType)
{
    if (radius <= width / 2) {
        fillCircleImpl(center, radius, color);
        return;
    }

    LineTypeCursor cursor{lineType.forWidth(width)};
    bresenhamCircle(radius, [&](Pixel p) {
        if (cursor.next()) {
            int halfW = width / 2;
            int startOffset = -halfW;
            int endOffset = width - halfW - 1;

            int xLStart = center.x - p.x + startOffset;
            int xLEnd = center.x - p.x + endOffset;
            int xRStart = center.x + p.x + startOffset;
            int xREnd = center.x + p.x + endOffset;

            int yTStart = center.y - p.x + startOffset;
            int yTEnd = center.y - p.x + endOffset;
            int yBStart = center.y + p.x + startOffset;
            int yBEnd = center.y + p.x + endOffset;

            fillSpan(center.y + p.y, xRStart, xREnd, color);
            fillSpan(center.y + p.y, xLStart, xLEnd, color);
            fillSpan(center.y - p.y, xRStart, xREnd, color);
            fillSpan(center.y - p.y, xLStart, xLEnd, color);

            fillSpanVertical(center.x + p.y, yBStart, yBEnd, color);
            fillSpanVertical(center.x - p.y, yBStart, yBEnd, color);
            fillSpanVertical(center.x + p.y, yTStart, yTEnd, color);
            fillSpanVertical(center.x - p.y, yTStart, yTEnd, color);
        }
    });
}

void RenderTarget::fillCircle(Point center, double radius, RGBA color)
{
    fillCircleImpl(camera.worldToPixel(center),
                   std::round(radius * camera.scale), color);
}

void RenderTarget::fillCircleImpl(Pixel center, int radius, RGBA color)
{
    if (radius <= 0) {
        setPixel(center, color);
        return;
    }

    bresenhamCircle(radius, [&](Pixel p) {
        fillSpan(center.y + p.y, center.x - p.x, center.x + p.x, color);
        fillSpan(center.y - p.y, center.x - p.x, center.x + p.x, color);
        fillSpan(center.y + p.x, center.x - p.y, center.x + p.y, color);
        fillSpan(center.y - p.x, center.x - p.y, center.x + p.y, color);
    });
}

void RenderTarget::drawPolygonOutline(std::span<const Point> verts,
                                      double width, RGBA color,
                                      LineType lineType)
{
    if (verts.size() <= 1) {
        return;
    }
    if (verts.size() == 2) {
        drawLine(verts[0], verts[1], width, color, lineType);
    }
    auto view = verts | std::views::transform(
                            [this](Point p) { return camera.worldToPixel(p); });
    drawPolygonOutlineImpl(
        std::vector(view.begin(), view.end()),
        std::max(1, static_cast<int>(std::round(width * camera.scale))), color,
        lineType);
}

void RenderTarget::drawPolygonOutlineImpl(std::vector<Pixel> verts, int width,
                                          RGBA color, LineType lineType)
{
    verts.push_back(verts[0]);
    for (std::size_t i = 0; i < verts.size(); i++) {
        drawLineImpl(verts[i], verts[(i + 1) % verts.size()], width, color,
                     lineType);
    }
}

void RenderTarget::fillPolygon(std::span<const Point> verts, RGBA color)
{
    if (verts.size() <= 1) {
        return;
    }
    if (verts.size() == 2) {
        drawLine(verts[0], verts[1], 0, color, LineType{});
        return;
    }
    auto view = verts | std::views::transform(
                            [this](Point p) { return camera.worldToPixel(p); });
    fillPolygonImpl(std::vector(view.begin(), view.end()), color);
}

// 圆弧绘制（中点圆算法 + 叉积侧向过滤，参照 VectorCanvas2d Rasterizer::arc）
// 割线 ep1→ep2 把圆分成两段弧，保留 side 侧：
// cross(ep2−ep1, p−ep1) * side > 0；弧端点容差强制落笔补齐
void RenderTarget::drawArc(Point center, double radius, Point ep1, Point ep2,
                           int side, double width, RGBA color,
                           LineType lineType)
{
    Pixel centerPixel = camera.worldToPixel(center);
    int radiusPixel = std::round(radius * camera.scale);
    int widthPixel =
        std::max(1, static_cast<int>(std::round(width * camera.scale)));
    if (radiusPixel <= 0) {
        return;
    }

    Pixel ep1Pixel = camera.worldToPixel(ep1);
    Pixel ep2Pixel = camera.worldToPixel(ep2);
    const double dx = ep2Pixel.x - ep1Pixel.x;
    const double dy = ep2Pixel.y - ep1Pixel.y;
    const int keepSide = side >= 0 ? 1 : -1;

    // 侧向判定：cross(ep2−ep1, p−ep1) * side > 0，端点附近强制保留
    auto keep = [&](Pixel p) {
        const double d1 = std::hypot(p.x - ep1Pixel.x, p.y - ep1Pixel.y);
        const double d2 = std::hypot(p.x - ep2Pixel.x, p.y - ep2Pixel.y);
        if (d1 <= 1.5 || d2 <= 1.5) {
            return true; // 端点附近强制落笔
        }
        const double cross = dx * (p.y - ep1Pixel.y) - dy * (p.x - ep1Pixel.x);
        return cross * keepSide > 0;
    };

    LineTypeCursor cursor{lineType.forWidth(widthPixel)};
    int halfWidth = widthPixel / 2;
    int startOffset = -halfWidth;
    int endOffset = widthPixel - halfWidth - 1;
    bresenhamCircle(radiusPixel, [&](Pixel p) {
        if (!cursor.next()) {
            return;
        }
        Pixel horizontalOctants[4] = {
            centerPixel + Pixel{p.x, p.y},
            centerPixel + Pixel{-p.x, p.y},
            centerPixel + Pixel{p.x, -p.y},
            centerPixel + Pixel{-p.x, -p.y},
        };

        Pixel verticalOctants[4] = {
            centerPixel + Pixel{p.y, p.x},
            centerPixel + Pixel{-p.y, p.x},
            centerPixel + Pixel{p.y, -p.x},
            centerPixel + Pixel{-p.y, -p.x},
        };

        for (const Pixel &q : horizontalOctants) {
            if (keep(q)) {
                fillSpan(q.y, q.x + startOffset, q.x + endOffset, color);
            }
        }

        for (const Pixel &q : verticalOctants) {
            if (keep(q)) {
                fillSpanVertical(q.x, q.y + startOffset, q.y + endOffset,
                                 color);
            }
        }
    });
}

void RenderTarget::fillPolygonImpl(std::vector<Pixel> verts, RGBA color)
{
    struct EdgeInfo {
        int yMax;
        double x;
        double dx;
    };
    std::map<int, std::vector<EdgeInfo>> et;
    for (std::size_t i = 0; i < verts.size(); i++) {
        auto start = verts[i];
        auto end = verts[(i + 1) % verts.size()];
        if (start.y == end.y) {
            continue;
        }
        if (start.y > end.y) {
            std::swap(start, end);
        }
        if (start.y >= getHeight() || end.y <= 0) {
            continue;
        }
        double x = start.x;
        double dx = static_cast<double>(end.x - x) / (end.y - start.y);
        auto y = start.y;
        if (start.y < 0) {
            x += dx * -start.y;
            y = 0;
        }
        et[y].push_back(EdgeInfo{end.y, x, dx});
    }

    std::vector<EdgeInfo> aet;
    auto it = et.begin();
    auto y = it->first;
    do {
        if (it != et.end() && it->first == y) {
            aet.insert(aet.end(), it->second.begin(), it->second.end());
            it++;
        }
        std::sort(
            aet.begin(), aet.end(),
            [](const EdgeInfo &a, const EdgeInfo &b) { return a.x < b.x; });

        for (size_t i = 0; i + 1 < aet.size(); i += 2) {
            fillSpan(y, static_cast<int>(std::ceil(aet[i].x)),
                     static_cast<int>(std::floor(aet[i + 1].x)), color);
        }

        aet.erase(
            std::remove_if(aet.begin(), aet.end(),
                           [y](const EdgeInfo &e) { return e.yMax == y + 1; }),
            aet.end());
        for (auto &e : aet) {
            e.x += e.dx;
        }
        y++;
    } while (!aet.empty() && y < getHeight());
}

void RenderTarget::drawCross(Point p, double width, double radius, RGBA color)
{
    auto wRadius = radius / camera.scale;
    auto wWidth = width / camera.scale;
    drawLine(Point{p.x - wRadius, p.y}, Point{p.x + wRadius, p.y}, wWidth,
             color, LineType{});
    drawLine(Point{p.x, p.y - wRadius}, Point{p.x, p.y + wRadius}, wWidth,
             color, LineType{});
}