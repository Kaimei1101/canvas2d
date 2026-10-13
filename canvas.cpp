#include "canvas.h"

#include "drawable/circle.h"
#include "drawable/line.h"

Canvas::Canvas(int width, int height) : target{width, height}, dirty{false}
{
    shapes.push_back(std::make_unique<Line>(Point{100, 100}, Point{0, 0}));
    shapes.push_back(std::make_unique<Line>(Point{100, 100}, Point{100, 0}));
    shapes.push_back(std::make_unique<Line>(Point{100, 100}, Point{0, 100}));
    shapes.push_back(std::make_unique<Line>(Point{100, 100}, Point{200, 300}));
    shapes.push_back(std::make_unique<Circle>(Point{100, 100}, 50));
    dirty = true;
}

int Canvas::getwidth() const { return target.getWidth(); }

int Canvas::getheight() const { return target.getHeight(); }

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
}
