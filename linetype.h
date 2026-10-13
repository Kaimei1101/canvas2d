#ifndef LINETYPE_H
#define LINETYPE_H

#include <array>

class LineType
{
    static const int PAT_SIZE = 4;

  public:
    std::array<double, 4> pat;
    LineType();
    LineType(std::array<double, 4> pat);
    LineType forWidth(int width);
    LineType scale(double factor);
};

#endif // LINETYPE_H
