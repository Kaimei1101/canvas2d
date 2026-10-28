#ifndef RGBA_H
#define RGBA_H

#include <cstdint>

class RGBA
{
  public:
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;

    inline constexpr RGBA(std::uint8_t r, std::uint8_t g, std::uint8_t b,
                          std::uint8_t a = 255)
        : r{r}, g{g}, b{b}, a{a}
    {
    }

    bool operator==(const RGBA &) const = default;
};

#endif // RGBA_H
