#ifndef CIRCLE_H
#define CIRCLE_H

#include "drawable.h"
#include "linetype.h"
#include "point.h"
#include "rgba.h"

class Circle : public Drawable
{
  public:
    // 圆弧数据：弧端点（割线与圆的交点）+ 保留侧符号
    // side 语义：相对 ep1→ep2 方向，cross(ep2−ep1, p−ep1) * side > 0
    // 的像素被保留
    struct ArcData {
        Point ep1, ep2;    // 弧端点
        int side;          // 保留侧符号（+1 或 -1）
        bool isFullCircle; // 是否为整圆
    };

  private:
    Point center;
    double radius;
    ArcData arcData;

    double width;
    RGBA strokeColor;
    RGBA fillColor;
    bool stroke;
    bool fill;
    LineType lineType;

  public:
    // 整圆构造函数
    Circle(Point center, double radius, bool fill = false,
           RGBA fillColor = RGBA{255, 255, 255}, bool stroke = true,
           double width = 1, RGBA strokeColor = RGBA{0, 0, 0},
           LineType lineType = LineType{});

    // 圆心+边缘点构造整圆
    Circle(Point center, Point edge, bool fill = false,
           RGBA fillColor = RGBA{255, 255, 255}, bool stroke = true,
           double width = 1, RGBA strokeColor = RGBA{0, 0, 0},
           LineType lineType = LineType{});

    // 圆弧构造函数（圆心、半径、弧端点1、弧端点2、保留侧符号）
    Circle(Point center, double radius, Point ep1, Point ep2, int side,
           bool fill = false, RGBA fillColor = RGBA{255, 255, 255},
           bool stroke = true, double width = 1,
           RGBA strokeColor = RGBA{0, 0, 0}, LineType lineType = LineType{});

    const ArcData &getArcData() const { return arcData; }

    bool isFullCircle() const { return arcData.isFullCircle; }

    void draw(RenderTarget &rt) const override;
    void drawSelHighlight(RenderTarget &rt) const override;
    bool hitTest(const Camera &camera, Point p) const override;
    Point centroid() const override { return center; }
    bool isClosed() const override { return arcData.isFullCircle; }
    bool contains(Point p) const override
    {
        return arcData.isFullCircle && center.distance(p) <= radius;
    }
    void transform(Transform t) override;
    void setFill(RGBA color) override
    {
        if (arcData.isFullCircle) {
            fill = true;
            fillColor = color;
        }
    }
    AABB bounds() const override;

    Point getCenter() const { return center; }
    double getRadius() const { return radius; }
    bool isFilled() const { return fill; }
    bool isStroked() const { return stroke; }
    RGBA getFillColor() const { return fillColor; }
    RGBA getStrokeColor() const { return strokeColor; }
    double getLineWidth() const { return width; }
    LineType getLineType() const { return lineType; }
};

#endif // CIRCLE_H
