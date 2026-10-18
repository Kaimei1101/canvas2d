#include "circle.h"

#include <algorithm>

#include "geometry.h"

Circle::Circle(Point center, double radius, bool fill, RGBA fillColor,
               bool stroke, double width, RGBA strokeColor, LineType lineType)
    : center{center}, radius{radius},
      arcData{Point{0, 0}, Point{0, 0}, 1, true}, // 整圆：无端点
      width{width}, strokeColor{strokeColor}, fillColor{fillColor},
      stroke{stroke}, fill{fill}, lineType{lineType}
{
}

Circle::Circle(Point center, Point edge, bool fill, RGBA fillColor, bool stroke,
               double width, RGBA strokeColor, LineType lineType)
    : Circle{center, center.distance(edge), fill,    fillColor, stroke,
             width,  strokeColor,           lineType}
{
}

Circle::Circle(Point center, double radius, Point ep1, Point ep2, int side,
               bool fill, RGBA fillColor, bool stroke, double width,
               RGBA strokeColor, LineType lineType)
    : center{center}, radius{radius},
      arcData{ep1, ep2, side >= 0 ? 1 : -1, false}, width{width},
      strokeColor{strokeColor}, fillColor{fillColor}, stroke{stroke},
      fill{fill}, lineType{lineType}
{
}

void Circle::draw(RenderTarget &rt) const
{
    if (fill && arcData.isFullCircle) {
        rt.fillCircle(center, radius, fillColor);
    }
    if (stroke) {
        if (arcData.isFullCircle) {
            rt.drawCircleOutline(center, radius, width, strokeColor, lineType);
        } else {
            rt.drawArc(center, radius, arcData.ep1, arcData.ep2, arcData.side,
                       width, strokeColor, lineType);
        }
    }
}

void Circle::drawSelHighlight(RenderTarget &rt) const
{
    auto highlightWidth = width + Drawable::selHighlightWidth / rt.camera.scale;
    if (arcData.isFullCircle) {
        rt.drawCircleOutline(center, radius, highlightWidth,
                             Drawable::selHighlightColor, LineType{});
    } else {
        rt.drawArc(center, radius, arcData.ep1, arcData.ep2, arcData.side,
                   highlightWidth, Drawable::selHighlightColor, LineType{});
    }
}

bool Circle::hitTest(const Camera &camera, Point p) const
{
    double tol = Drawable::hitTestTol / camera.scale + width / 2.0;
    double distToCenter = p.distance(center);

    if (std::abs(distToCenter - radius) <= tol) {
        if (isFullCircle()) {
            return true;
        }
        return Geometry::cross(arcData.ep1, arcData.ep2, p) * arcData.side >= 0;
    }

    if (isFilled() && distToCenter <= radius) {
        return true;
    }
    return false;
}

void Circle::transform(Transform t)
{
    center = center.transform(t);
    auto scale = t.getScale();
    radius *= scale;
    width *= scale;
}