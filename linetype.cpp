#include "linetype.h"

#include <limits>

LineType::LineType()
    : pat{std::numeric_limits<double>::infinity(),
          std::numeric_limits<double>::infinity(),
          std::numeric_limits<double>::infinity(),
          std::numeric_limits<double>::infinity()}
{
}

LineType::LineType(std::array<double, 4> pat) : pat{std::move(pat)} {}

LineType LineType::forWidth(int width)
{
    return LineType{
        {pat[0] * width, pat[1] * width, pat[2] * width, pat[3] * width}};
}

LineType LineType::scale(double factor)
{
    return LineType{
        {pat[0] * factor, pat[1] * factor, pat[2] * factor, pat[3] * factor}};
}