#include "line.h"

#include "geometry.h"
#include "rendertarget.h"

Line::Line(Point start, Point end, double width, RGBA color, LineType lineType)
    : start{start}, end{end}, width{width}, color{color}, lineType{lineType}
{
}

void Line::draw(RenderTarget &rt) const
{
    rt.drawLine(start, end, width, color, lineType);
}

void Line::drawSelHighlight(RenderTarget &rt) const
{
    rt.drawLine(start, end,
                width + Drawable::selHighlightWidth / rt.camera.scale,
                Drawable::selHighlightColor, LineType{});
}

bool Line::hitTest(const Camera &camera, Point p) const
{
    double tol = Drawable::hitTestTol / camera.scale + width / 2.0;
    double dist = Geometry::pointToSegmentDistanceSqr(p, start, end);
    return dist <= tol * tol;
}

void Line::transform(Transform t)
{
    start = start.transform(t);
    end = end.transform(t);
    width *= t.getScale();
}