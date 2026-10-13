#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include <span>

#include "camera.h"
#include "linetype.h"
#include "pixel_buffer.h"

class RenderTarget
{
    PixelBuffer pixels;

  public:
    Camera camera;

  public:
    RenderTarget(int width, int height);
    int getWidth() const;
    int getHeight() const;
    const PixelBuffer &getPixels() const;
    void resize(int width, int height);
    void clear();
    void setPixel(Pixel p, RGBA color);
    void fillSpan(int y, int x0, int x1, RGBA color);
    void drawLine(Point start, Point end, double width, RGBA color,
                  LineType lineType);
    void drawCircleOutline(Point center, double radius, double width,
                           RGBA color, LineType lineType);
    void fillCircle(Point center, double radius, RGBA color);
    void drawPolygonOutline(std::span<const Point> verts, double width,
                            RGBA color, LineType lineType);
    void fillPolygon(std::span<const Point> verts, RGBA color);

  private:
    void setPixelUnchecked(Pixel p, RGBA color);
    void drawLine(Pixel start, Pixel end, int width, RGBA color,
                  LineType lineType);
    void drawCircleOutline(Pixel center, int radius, int width, RGBA color,
                           LineType lineType);
    void fillCircle(Pixel center, int radius, RGBA color);
    void drawPolygonOutline(std::vector<Pixel> verts, int width, RGBA color,
                            LineType lineType);
    void fillPolygon(std::vector<Pixel> verts, RGBA color);
};

#endif // RENDERTARGET_H
