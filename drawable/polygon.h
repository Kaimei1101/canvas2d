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
    void drawSelHighlight(RenderTarget &rt) const override;
    bool hitTest(const Camera &camera, Point p) const override;
    // 重心取顶点算术平均
    Point centroid() const override;
    bool isClosed() const override { return verts.size() >= 3; }
    bool contains(Point p) const override;
    void transform(Transform t) override;
    inline void setFill(RGBA color) override
    {
        fill = true;
        fillColor = color;
    }
    inline AABB bounds() const override { return AABB{verts}; }

    const std::vector<Point> &getVerts() const { return verts; }
    bool isFilled() const { return fill; }
    bool isStroked() const { return stroke; }
    RGBA getFillColor() const { return fillColor; }
    RGBA getStrokeColor() const { return strokeColor; }
    double getLineWidth() const { return width; }
    LineType getLineType() const { return lineType; }
};

#endif // POLYGON_H
