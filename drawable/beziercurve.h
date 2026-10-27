#ifndef BEZIERCURVE_H
#define BEZIERCURVE_H

#include <span>

#include "drawable.h"

class BezierCurve : public Drawable
{
  public:
    std::vector<Point> ctrlPts;
    double width;
    RGBA color;
    LineType lineType;

    BezierCurve(std::vector<Point> pts, double width = 1,
                RGBA color = RGBA{0, 0, 0}, LineType lineType = LineType{})
        : ctrlPts{std::move(pts)}, width{width}, color(color),
          lineType{lineType}
    {
    }

    void draw(RenderTarget &rt) const override;
    void drawSelHighlight(RenderTarget &rt) const override;
    bool hitTest(const Camera &camera, Point p) const override;
    Point centroid() const override;
    bool isClosed() const override { return false; }
    bool contains(Point p) const override { return false; }
    void transform(Transform t) override;
    void setFill(RGBA color) override {}
    std::vector<std::unique_ptr<Drawable>>
    clip(std::span<const Point> clipPoints);
    AABB bounds() const override;
};

#endif // BEZIERCURVE_H
