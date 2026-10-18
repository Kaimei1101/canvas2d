#ifndef CANVAS_H
#define CANVAS_H

#include <cstddef>
#include <memory>
#include <unordered_set>
#include <vector>

#include "drawable/drawable.h"
#include "point.h"
#include "rendertarget.h"

class Canvas
{
    RenderTarget target;
    std::vector<std::unique_ptr<Drawable>> shapes;
    std::unordered_set<std::size_t> selected;
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
    // 删除指定下标的图形（裁剪替换时使用）
    inline void removeShape(std::size_t index)
    {
        if (index < shapes.size()) {
            shapes.erase(shapes.begin() + static_cast<std::ptrdiff_t>(index));
            selected.clear();
            dirty = true;
        }
    }
    bool isDirty() const;
    inline void markDirty() { dirty = true; }
    const RenderTarget &getRenderTarget() const;
    void resize(int width, int height);
    void redraw();
    inline const std::unordered_set<std::size_t> &getSelected() const
    {
        return selected;
    }
    void select(std::size_t index);
    void unselect(std::size_t index);
    bool isSelected(std::size_t index) const
    {
        return selected.contains(index);
    };
    void toggleSelection(std::size_t index);
    void clearSelection();
    void drawCentroids();
    void drawCentroidMark(Point p);
    void drawGroupCentroidMark(Point p);
    void applyTransform(Transform t);
    void removeShapes();
    void applyClip(std::span<const Point> clipPoints);
    inline void drawShape(const Drawable &shape)
    {
        shape.draw(target);
        dirty = true;
    }
};

#endif // CANVAS_H
