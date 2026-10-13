#include "polygon.h"

Polygon::Polygon(std::vector<Point> verts, bool fill, RGBA fillColor,
                 bool stroke, double width, RGBA strokeColor)
    : verts{std::move(verts)}, width{width}, strokeColor{strokeColor},
      fillColor{fillColor}, stroke{stroke}, fill{fill}
{
}

void Polygon::draw(RenderTarget &rt) const
{
    if (fill) {
        rt.fillPolygon(verts, fillColor);
    }
    if (stroke) {
        rt.drawPolygonOutline(verts, width, strokeColor);
    }
}