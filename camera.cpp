#include "camera.h"

Camera::Camera() : anchor{0, 0}, scale{1} {}

Pixel Camera::worldToPixel(Point p) const { return Pixel{worldToCanvas(p)}; }

Point Camera::pixelToWorld(Pixel p) const
{
    return (Point{p} / scale) + anchor;
}

Point Camera::worldToCanvas(Point p) const { return (p - anchor) * scale; }

Point Camera::screenToWorld(Point p) const { return (p / scale) + anchor; }

void Camera::move(Point screen_vec) { anchor = anchor + screen_vec / scale; }

void Camera::zoomAt(Point c, double factor)
{
    auto zoomed_c = c / factor;
    move(c - zoomed_c);
    scale = scale * factor;
}