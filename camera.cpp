#include "camera.h"

Camera::Camera() : center{0, 0}, scale{1} {}

Pixel Camera::worldToPixel(Point p) const { return Pixel{worldToCanvas(p)}; }

Point Camera::worldToCanvas(Point p) const { return (p - center) * scale; }

Point Camera::screenToWorld(Point p) const { return (p / scale) + center; }

void Camera::move(Point screen_vec) { center = center + screen_vec / scale; }

void Camera::zoomAt(Point c, double factor)
{
    auto zoomed_c = c / factor;
    move(c - zoomed_c);
    scale = scale * factor;
}