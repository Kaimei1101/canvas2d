#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <functional>
#include <memory>
#include <utility>

#include "aabb.h"
#include "rendertarget.h"
#include "rgba.h"
#include "transform.h"

class Drawable
{
  protected:
    static constexpr double SEL_HIGHLIGHT_WIDTH = 4.0;
    static constexpr RGBA SEL_HIGHLIGHT_COLOR{30, 144, 255, 100};
    static constexpr double HIT_TEST_TOL = 5.0;

  public:
    virtual void drawStroke(RenderTarget &rt) const = 0;
    virtual void fillInner(RenderTarget &rt) const = 0;
    virtual void drawSelHighlight(RenderTarget &rt) const = 0;
    virtual bool hitTest(const Camera &camera, Point p) const = 0;
    // 图形重心（世界坐标），用于选择/变换的参考点
    virtual Point centroid() const = 0;
    // 是否为合法封闭图形（整圆、≥3 顶点的多边形）
    virtual bool isClosed() const = 0;
    // 点是否落在图形内部（非封闭图形恒为 false）
    virtual bool contains(Point p) const = 0;
    virtual void transform(Transform t) = 0;
    // 将图形填充属性修改为指定颜色（非封闭图形为空操作）
    virtual void setFill(RGBA color) = 0;
    virtual AABB bounds() const = 0;
    virtual std::vector<std::unique_ptr<Drawable>>
    clip(std::span<const Point> clipPoints) = 0;
    virtual std::optional<std::pair<Point, std::function<void(Point)>>>
    getModificationHandler(const Camera &camera, Point p) = 0;
    virtual ~Drawable() = default;
};

#endif // DRAWABLE_H
