#ifndef BSPLINE_H
#define BSPLINE_H

#include "aabb.h"
#include "drawable.h"
#include "geometry.h"

class BSpline : public Drawable
{
  public:
  private:
    std::vector<Point> ctrlPts;
    std::vector<double> knots;
    int degree;
    double width;
    RGBA color;
    LineType lineType;

  public:
    inline BSpline(std::vector<Point> pts, int degree, double width = 1,
                   RGBA color = RGBA{0, 0, 0}, LineType lineType = LineType{})
        : ctrlPts{std::move(pts)}, degree{degree}, width{width}, color(color),
          lineType{lineType}
    {
        calKnots();
    }

    void calKnots();

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

    AABB bounds() const override { return AABB{ctrlPts}; }
};

#endif // BSPLINE_H
