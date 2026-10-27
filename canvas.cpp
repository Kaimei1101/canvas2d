#include "canvas.h"

#include "drawable/circle.h"
#include "drawable/line.h"
#include "drawable/polygon.h"
#include "geometry.h"
#include "transform.h"

Canvas::Canvas(int width, int height) : target{width, height}, dirty{false}
{
    shapes.push_back(std::make_unique<Line>(Point{100, 100}, Point{0, 0}));
    shapes.push_back(std::make_unique<Line>(Point{100, 100}, Point{100, 0}));
    shapes.push_back(std::make_unique<Line>(Point{100, 100}, Point{0, 100}));
    shapes.push_back(std::make_unique<Line>(Point{100, 100}, Point{200, 300}));
    shapes.push_back(std::make_unique<Circle>(Point{100, 100}, 50));
    dirty = true;
}

int Canvas::getWidth() const { return target.getWidth(); }

int Canvas::getHeight() const { return target.getHeight(); }

bool Canvas::isDirty() const { return dirty; }

const RenderTarget &Canvas::getRenderTarget() const { return target; }

void Canvas::resize(int width, int height)
{
    target.resize(width, height);
    dirty = true;
}

void Canvas::redraw()
{
    if (!dirty) {
        return;
    }
    target.clear();
    for (const auto &s : shapes) {
        s->draw(target);
    }
    for (auto idx : selected) {
        shapes[idx]->drawSelHighlight(target);
    }
    drawCentroids();
}

void Canvas::select(std::size_t index)
{
    if (index >= shapes.size()) {
        return;
    }
    if (selected.insert(index).second) {
        dirty = true;
    }
}

void Canvas::unselect(std::size_t index)
{
    if (index >= shapes.size()) {
        return;
    }
    if (selected.erase(index) > 0) {
        dirty = true;
    }
}

void Canvas::toggleSelection(std::size_t index)
{
    if (index >= shapes.size()) {
        return;
    }
    if (selected.contains(index)) {
        selected.erase(index);
    } else {
        selected.insert(index);
    }
    dirty = true;
}

void Canvas::clearSelection()
{
    if (!selected.empty()) {
        selected.clear();
        dirty = true;
    }
}

Point Canvas::selCentroid()
{
    Point sum{0, 0};
    for (size_t idx : selected) {
        sum = sum + shapes[idx]->centroid();
    }
    return sum / static_cast<double>(selected.size());
}

void Canvas::drawCentroids()
{
    for (auto i : selected) {
        drawCentroidMark(shapes[i]->centroid());
    }
    // 组重心：选中图形重心的算术平均
    if (selected.size() >= 2) {

        drawGroupCentroidMark(selCentroid());
    }
}

// 重心标记：方块（选中为红色，未选中为蓝色），边长以屏幕像素固定
void Canvas::drawCentroidMark(Point p)
{
    // 以屏幕像素固定的标记尺寸：转换回世界坐标
    const double half = 4.0 / getScale();
    const RGBA color = RGBA{220, 0, 0};
    std::vector<Point> verts = {
        Point{p.x - half, p.y - half},
        Point{p.x + half, p.y - half},
        Point{p.x + half, p.y + half},
        Point{p.x - half, p.y + half},
    };
    Polygon{verts, true, color, false, 0, color, LineType{}}.draw(target);
}

// 组重心标记：较大的红色十字 + 圆圈感（用加粗十字表示）
void Canvas::drawGroupCentroidMark(Point p) { target.drawCross(p, 6, 10); }

void Canvas::applyTransform(Transform t)
{
    if (selected.empty()) {
        return;
    }
    for (auto i : selected) {
        shapes[i]->transform(t);
    }
    dirty = true;
}

void Canvas::removeShapes()
{
    if (selected.empty()) {
        return;
    }

    size_t current_index = 0;
    auto it = std::remove_if(
        shapes.begin(), shapes.end(), [&](const std::unique_ptr<Drawable> &) {
            bool should_remove = selected.contains(current_index);
            current_index++;
            return should_remove;
        });

    shapes.erase(it, shapes.end());
    selected.clear();
    dirty = true;
}

void Canvas::applyClip(std::span<const Point> clipPoints)
{
    std::vector<std::unique_ptr<Drawable>> newShapes;
    for (auto i : selected) {
        auto clippedShapes = shapes[i]->clip(clipPoints);
        for (auto &s : clippedShapes) {
            newShapes.push_back(std::move(s));
        }
    }
    removeShapes();
    shapes.insert(shapes.end(), std::make_move_iterator(newShapes.begin()),
                  std::make_move_iterator(newShapes.end()));
}
