#include "line.h"

#include "rendertarget.h"

Line::Line(Point start, Point end, double width, RGBA color, LineType lineType)
    : start{start}, end{end}, width{width}, color{color}, lineType{lineType}
{
}

void Line::draw(RenderTarget &rt) const
{
    rt.drawLine(start, end, width, color, lineType);
}