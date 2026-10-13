#ifndef POLYGON_H
#define POLYGON_H

#include <vector>

#include "drawable.h"
#include "linetype.h"
#include "point.h"
#include "rgba.h"

class Polygon : public Drawable
{
    std::vector<Point> verts;
    double width;
    RGBA strokeColor;
    RGBA fillColor;
    bool stroke;
    bool fill;
    LineType lineType;

  public:
    Polygon(std::vector<Point> verts, bool fill = false,
            RGBA fillColor = RGBA{255, 255, 255}, bool stroke = true,
            double width = 1, RGBA strokeColor = RGBA{0, 0, 0},
            LineType lineType = LineType{});
    void draw(RenderTarget &rt) const override;
};

#endif // POLYGON_H
