#ifndef TOOL_H
#define TOOL_H

enum class Tool {
    Select,
    Line,
    Circle,
    Arc,
    Polygon,
    Curve,
    BezierCurve,
    Fill,
    Clip,
};

inline const char *toolName(Tool t)
{
    switch (t) {
    case Tool::Select:
        return "框选";
    case Tool::Line:
        return "直线";
    case Tool::Circle:
        return "圆";
    case Tool::Arc:
        return "圆弧";
    case Tool::Polygon:
        return "多边形";
    case Tool::Curve:
        return "曲线";
    case Tool::BezierCurve:
        return "Bezier曲线";
    case Tool::Fill:
        return "填充";
    case Tool::Clip:
        return "裁剪";
    }
    return "未知";
}

#endif // TOOL_H
