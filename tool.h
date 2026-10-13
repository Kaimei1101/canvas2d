#ifndef TOOL_H
#define TOOL_H

#include <QString>

enum class Tool {
    Select,
    Line,
    Circle,
    Polygon,
    Fill,
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
    case Tool::Polygon:
        return "多边形";
    case Tool::Fill:
        return "填充";
    }
    return "未知";
}

#endif // TOOL_H
