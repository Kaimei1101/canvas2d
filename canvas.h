#ifndef CANVAS_H
#define CANVAS_H

#include <memory>
#include <vector>

#include "drawable/drawable.h"
#include "point.h"
#include "rendertarget.h"

class Canvas
{
    RenderTarget target;
    std::vector<std::unique_ptr<Drawable>> shapes;
    bool dirty;

  public:
    Canvas(int width, int height);

    int getWidth() const;
    int getHeight() const;
    inline const std::vector<std::unique_ptr<Drawable>> &getShapes() const
    {
        return shapes;
    }
    inline const Camera &getCamera() const { return target.camera; }
    inline double getScale() const { return target.camera.scale; }
    inline Point getAnchor() const { return target.camera.anchor; }
    inline void move_camera(Point screen_vec)
    {
        if (screen_vec != Point{0, 0}) {
            target.camera.move(screen_vec);
            dirty = true;
        }
    }
    inline void setScale(double scale)
    {
        if (scale != target.camera.scale) {
            target.camera.scale = scale;
            dirty = true;
        }
    }
    inline void zoomAt(Point center, double factor)
    {
        if (factor != 1) {
            target.camera.zoomAt(center, factor);
            dirty = true;
        }
    }
    inline void addShape(std::unique_ptr<Drawable> shape)
    {
        shapes.push_back(std::move(shape));
        dirty = true;
    }
    bool isDirty() const;
    inline void markDirty() { dirty = true; }
    const RenderTarget &getRenderTarget() const;
    void resize(int width, int height);
    void redraw();
    inline void drawShape(const Drawable &shape)
    {
        shape.draw(target);
        dirty = true;
    }
};

#endif // CANVAS_H
