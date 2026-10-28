#ifndef CAMERA_H
#define CAMERA_H

#include <cstddef>

#include "pixel.h"
#include "point.h"

class Camera
{
  public:
    Point anchor;
    // Number of screen pixels per one unit of world length.
    double scale;

    Camera();
    Pixel worldToPixel(Point p) const;
    Point pixelToWorld(Pixel p) const;
    Point worldToCanvas(Point p) const;
    Point screenToWorld(Point p) const;
    void move(Point screen_vec);
    // Zooms around the specified screen point,
    // so the world position under that point remains the same after zooming.
    void zoomAt(Point center, double factor);
};

#endif // CAMERA_H
