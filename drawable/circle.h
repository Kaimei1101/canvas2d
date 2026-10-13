#ifndef CIRCLE_H
#define CIRCLE_H

#include "drawable.h"
#include "point.h"
#include "rgba.h"

class Circle : public Drawable
{
    Point center;
    double radius;
    double width;
    RGBA strokeColor;
    RGBA fillColor;
    bool stroke;
    bool fill;

  public:
    Circle(Point center, double radius, bool fill = false,
           RGBA fillColor = RGBA{255, 255, 255}, bool stroke = true,
           double width = 1, RGBA strokeColor = RGBA{0, 0, 0});
    Circle(Point center, Point edge, bool fill = false,
           RGBA fillColor = RGBA{255, 255, 255}, bool stroke = true,
           double width = 1, RGBA strokeColor = RGBA{0, 0, 0});
    void draw(RenderTarget &rt) const override;
};

#endif // CIRCLE_H
