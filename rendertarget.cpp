#include <cmath>
#include <ranges>

#include "rendertarget.h"

RenderTarget::RenderTarget(int width, int height) : pixels{width, height} {}

int RenderTarget::getWidth() const { return pixels.getwidth(); }

int RenderTarget::getHeight() const { return pixels.getheight(); }

const PixelBuffer &RenderTarget::getPixels() const { return pixels; }

void RenderTarget::resize(int width, int height)
{
    pixels = PixelBuffer{width, height};
}

void RenderTarget::clear() { pixels.clear(); }

void RenderTarget::setPixel(Pixel p, RGBA color)
{
    auto [x, y] = p;
    if (x < 0 || x >= getWidth() || y < 0 || y >= getHeight()) {
        return;
    }
    pixels(x, y) = color;
}

// World coordinate system
void RenderTarget::drawLine(Point startWorld, Point endWorld, double widthWorld,
                            RGBA color)
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

    drawLine(Pixel{start}, Pixel{end}, width, color);
}

// Canvas coordinate system
void RenderTarget::drawLine(Pixel start, Pixel end, int width, RGBA color)
{
    auto dx = std::abs(start.x - end.x);
    auto dy = -std::abs(start.y - end.y);
    auto sx = start.x > end.x ? -1 : 1;
    auto sy = start.y > end.y ? -1 : 1;
    int err = dx + dy;
    auto brushDir = dx > -dy ? &Pixel::y : &Pixel::x;

    for (auto i = 0; i <= std::max(dx, -dy); ++i) {
        for (auto j = -(width / 2); j < width - width / 2; ++j) {
            auto p = start;
            p.*brushDir += j;
            setPixel(p, color);
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
                                     RGBA color)
{
    drawCircleOutline(
        camera.worldToPixel(center), std::round(radius * camera.scale),
        std::max(1, static_cast<int>(std::round(width * camera.scale))), color);
}

void RenderTarget::drawCircleOutline(Pixel center, int radius, int width,
                                     RGBA color)
{
    if (radius <= width / 2) {
        fillCircle(center, radius, color);
        return;
    }

    bresenhamCircle(radius, [&](Pixel p) {
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
                                      double width, RGBA color)
{
    auto view = verts | std::views::transform([this](Point p) {
                    return camera.worldToCanvas(p);
                });
    auto pixels = std::vector(view.begin(), view.end());
    drawPolygonOutline(
        pixels, std::max(1, static_cast<int>(std::round(width * camera.scale))),
        color);
}

void RenderTarget::drawPolygonOutline(std::vector<Pixel> verts, int width,
                                      RGBA color)
{
}

void RenderTarget::fillPolygon(std::span<const Point> verts, RGBA color)
{
    auto view = verts | std::views::transform([this](Point p) {
                    return camera.worldToCanvas(p);
                });
    auto pixels = std::vector(view.begin(), view.end());
    fillPolygon(pixels, color);
}

struct EdgeInfo {
    int ymax;
    int x;
    int dx;
};

void RenderTarget::fillPolygon(std::vector<Pixel> verts, RGBA color)
{
    // std::vector<std::vector<EdgeInfo>> et(getHeight());
    // for (auto p : verts) {
    //     // et.
    // }
}