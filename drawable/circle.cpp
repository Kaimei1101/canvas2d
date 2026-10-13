#include "circle.h"

Circle::Circle(Point center, double radius, bool fill, RGBA fillColor,
               bool stroke, double width, RGBA strokeColor)
    : center{center}, radius{radius}, width{width}, strokeColor{strokeColor},
      fillColor{fillColor}, stroke{stroke}, fill{fill}
{
}

Circle::Circle(Point center, Point edge, bool fill, RGBA fillColor, bool stroke,
               double width, RGBA strokeColor)
    : Circle{center,     center.distance(edge), fill, fillColor, stroke, width,
             strokeColor}
{
}

void Circle::draw(RenderTarget &rt) const
{
    if (fill) {
        rt.fillCircle(center, radius, fillColor);
    }
    if (stroke) {

        rt.drawCircleOutline(center, radius, width, strokeColor);
    }
}