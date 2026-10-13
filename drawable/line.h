#ifndef LINE_H
#define LINE_H

#include "drawable.h"
#include "linetype.h"
#include "point.h"
#include "rgba.h"

class Line : public Drawable
{
    Point start;
    Point end;
    double width;
    RGBA color;
    LineType lineType;

  public:
    Line(Point start, Point end, double width = 1, RGBA color = RGBA{0, 0, 0},
         LineType lineType = LineType{});
    void draw(RenderTarget &rt) const override;
};

#endif // LINE_H
