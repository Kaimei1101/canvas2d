#ifndef LINE_H
#define LINE_H

#include "drawable.h"
#include "point.h"
#include "rgba.h"

class Line : public Drawable
{
    Point start;
    Point end;
    double width;
    RGBA color;

  public:
    Line(Point start, Point end, double width = 1, RGBA color = RGBA{0, 0, 0});
    void draw(RenderTarget &rt) const override;
};

#endif // LINE_H
