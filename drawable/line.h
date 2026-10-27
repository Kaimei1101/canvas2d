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
    void drawSelHighlight(RenderTarget &rt) const override;
    bool hitTest(const Camera &camera, Point p) const override;
    Point centroid() const override { return (start + end) / 2; }
    bool isClosed() const override { return false; }
    bool contains(Point) const override { return false; }
    void transform(Transform t) override;
    inline void setFill(RGBA) override {}
    AABB bounds() const override;
    std::vector<std::unique_ptr<Drawable>>
    clip(std::span<const Point> clipPoints);

    Point getStart() const { return start; }
    Point getEnd() const { return end; }
    double getWidth() const { return width; }
    RGBA getColor() const { return color; }
    LineType getLineType() const { return lineType; }
};

#endif // LINE_H
