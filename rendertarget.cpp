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
    pixels(p.x, p.y) = color;
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

// World coordinate system
void RenderTarget::drawLine(Point startWorld, Point endWorld, double widthWorld,
                            RGBA color, LineType lineType)
{
    auto start = camera.worldToCanvas(startWorld);
    auto end = camera.worldToCanvas(endWorld);

    auto [dx, dy] = end - start;

    // Adjust width for sloping line
    auto rawWidth = widthWorld * camera.scale;
    if (rawWidth > 1.5) {
        double len = std::sqrt(dx * dx + dy * dy);
        double widthFactor = len / std::max(std::abs(dx), std::abs(dy));
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

    drawLine(Pixel{start}, Pixel{end}, width, color, lineType);
}

// Canvas coordinate system
void RenderTarget::drawLine(Pixel start, Pixel end, int width, RGBA color,
                            LineType lineType)
{
    auto dx = std::abs(start.x - end.x);
    auto dy = -std::abs(start.y - end.y);
    auto sx = start.x > end.x ? -1 : 1;
    auto sy = start.y > end.y ? -1 : 1;
    int err = dx + dy;
    auto brushDir = dx > -dy ? &Pixel::y : &Pixel::x;

    LineTypeCursor cursor{lineType.forWidth(width)};
    for (auto i = 0; i <= std::max(dx, -dy); ++i) {
        if (cursor.next()) {
            for (auto j = -(width / 2); j < width - width / 2; ++j) {
                auto p = start;
                p.*brushDir += j;
                setPixel(p, color);
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

template <typename F> void bresenhamCircle(int radius, F brush)
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
    drawCircleOutline(
        camera.worldToPixel(center), std::round(radius * camera.scale),
        std::max(1, static_cast<int>(std::round(width * camera.scale))), color,
        lineType);
}

void RenderTarget::drawCircleOutline(Pixel center, int radius, int width,
                                     RGBA color, LineType lineType)
{
    if (radius <= width / 2) {
        fillCircle(center, radius, color);
        return;
    }

    LineTypeCursor cursor{lineType.forWidth(width)};
    bresenhamCircle(radius, [&](Pixel p) {
        if (cursor.next()) {
            for (auto j = -(width / 2); j < width - width / 2; ++j) {
                setPixel(center + Pixel{p.x + j, p.y}, color);
                setPixel(center + Pixel{-p.x + j, p.y}, color);
                setPixel(center + Pixel{p.x + j, -p.y}, color);
                setPixel(center + Pixel{-p.x + j, -p.y}, color);

                setPixel(center + Pixel{p.y, p.x + j}, color);
                setPixel(center + Pixel{-p.y, p.x + j}, color);
                setPixel(center + Pixel{p.y, -p.x + j}, color);
                setPixel(center + Pixel{-p.y, -p.x + j}, color);
            }
        }
    });
}

void RenderTarget::fillCircle(Point center, double radius, RGBA color)
{
    fillCircle(camera.worldToPixel(center), std::round(radius * camera.scale),
               color);
}

void RenderTarget::fillCircle(Pixel center, int radius, RGBA color)
{
    if (radius <= 0) {
        setPixel(center, color);
        return;
    }

    auto drawSpan = [&](int y, int x0, int x1) {
        if (y < 0 || y >= getHeight())
            return;
        x0 = std::max(x0, 0);
        x1 = std::min(x1, getWidth() - 1);
        for (int x = x0; x <= x1; ++x) {
            setPixel(Pixel{x, y}, color);
        }
    };

    bresenhamCircle(radius, [&](Pixel p) {
        drawSpan(center.y + p.y, center.x - p.x, center.x + p.x);
        drawSpan(center.y - p.y, center.x - p.x, center.x + p.x);
        drawSpan(center.y + p.x, center.x - p.y, center.x + p.y);
        drawSpan(center.y - p.x, center.x - p.y, center.x + p.y);
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
    drawPolygonOutline(
        std::vector(view.begin(), view.end()),
        std::max(1, static_cast<int>(std::round(width * camera.scale))), color,
        lineType);
}

void RenderTarget::drawPolygonOutline(std::vector<Pixel> verts, int width,
                                      RGBA color, LineType lineType)
{
    verts.push_back(verts[0]);
    for (std::size_t i = 0; i < verts.size(); i++) {
        drawLine(verts[i], verts[(i + 1) % verts.size()], width, color,
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
    fillPolygon(std::vector(view.begin(), view.end()), color);
}

void RenderTarget::fillPolygon(std::vector<Pixel> verts, RGBA color)
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