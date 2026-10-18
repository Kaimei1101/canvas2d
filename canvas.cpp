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

void Canvas::drawCentroids()
{
    for (auto i : selected) {
        drawCentroidMark(shapes[i]->centroid());
    }
    // 组重心：选中图形重心的算术平均
    if (selected.size() >= 2) {
        Point sum{0, 0};
        for (size_t idx : selected) {
            sum = sum + shapes[idx]->centroid();
        }
        drawGroupCentroidMark(sum / static_cast<double>(selected.size()));
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
    drawShape(Polygon{verts, true, color, true, 1, color, LineType{}});
}

// 组重心标记：较大的红色十字 + 圆圈感（用加粗十字表示）
void Canvas::drawGroupCentroidMark(Point p)
{
    const double r = 7.0 / getScale();
    const RGBA color{220, 0, 0};
    drawShape(
        Line{Point{p.x - r, p.y}, Point{p.x + r, p.y}, 2, color, LineType{}});
    drawShape(
        Line{Point{p.x, p.y - r}, Point{p.x, p.y + r}, 2, color, LineType{}});
}

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

// 实验二-4：裁剪窗口完成后执行裁剪。
// 选择顺序（从最上层图形开始匹配，一次只裁一个对象）：
//   1) 窗口重心落在合法封闭图形内 → 多边形直接 Sutherland-Hodgman；
//      整圆采样为多边形后 Sutherland-Hodgman，结果 <3 点删除，否则替换为
//      Polygon。
//   2) 否则找与窗口相交的最上层 Line → Liang-Barsky 凸多边形线段裁剪；
//      整段裁掉删除，否则用裁剪后端点替换原 Line（继承线宽/颜色/线型）。
//   3) 否则找与窗口相交的最上层圆弧（非整圆 Circle）→ 圆与窗口边求交角切分，
//      逐段判定中点是否在窗口内，输出若干段新弧（继承描边属性）；整段裁掉删除。
// 裁剪结果是普通 Polygon/Line/Circle(弧)，再次被裁剪时自然走同一流程。
void Canvas::applyClip(std::span<const Point> clipPoints)
{
    if (clipPoints.size() < 3) {
        return;
    }
    const Point winCenter = Geometry::centroid(clipPoints);

    // ---- 1) 窗口重心命中的封闭图形 ----
    std::size_t hit = shapes.size();
    for (std::size_t i = shapes.size(); i-- > 0;) {
        if (shapes[i]->isClosed() && shapes[i]->contains(winCenter)) {
            hit = i;
            break;
        }
    }
    if (hit != shapes.size()) {
        const Drawable *orig = shapes[hit].get();
        std::vector<Point> subject;
        bool doFill = false;
        RGBA fColor{0, 0, 0};
        bool doStroke = true;
        RGBA sColor{0, 0, 0};
        double w{};
        LineType lt;

        if (const auto *poly = dynamic_cast<const Polygon *>(orig)) {
            subject = poly->getVerts();
            doFill = poly->isFilled();
            fColor = poly->getFillColor();
            doStroke = poly->isStroked();
            sColor = poly->getStrokeColor();
            w = poly->getLineWidth();
            lt = poly->getLineType();
        } else if (const auto *circ = dynamic_cast<const Circle *>(orig)) {
            subject =
                Geometry::circleToPolygon(circ->getCenter(), circ->getRadius());
            doFill = circ->isFilled();
            fColor = circ->getFillColor();
            doStroke = circ->isStroked();
            sColor = circ->getStrokeColor();
            w = circ->getLineWidth();
            lt = circ->getLineType();
        } else {
            return;
        }

        std::vector<Point> result = Geometry::clipPolygon(subject, clipPoints);
        if (result.size() < 3) {
            removeShape(hit); // 窗口外无残留：图形被完全裁掉
        } else {
            auto clipped = std::make_unique<Polygon>(
                std::move(result), doFill, fColor, doStroke, w, sColor, lt);
            removeShape(hit);
            addShape(std::move(clipped));
        }
        return;
    }

    // ---- 2) 与窗口相交的最上层 Line ----
    for (std::size_t i = shapes.size(); i-- > 0;) {
        const auto *ln = dynamic_cast<const Line *>(shapes[i].get());
        if (!ln) {
            continue;
        }
        auto seg =
            Geometry::clipLineSegment(ln->getStart(), ln->getEnd(), clipPoints);
        if (!seg) {
            continue; // 该 Line 与窗口不相交
        }
        removeShape(i);
        if (!(seg->first == seg->second)) {
            addShape(std::make_unique<Line>(seg->first, seg->second,
                                            ln->getWidth(), ln->getColor(),
                                            ln->getLineType()));
        }
        return;
    }

    // ---- 3) 与窗口相交的最上层圆弧（非整圆 Circle） ----
    for (std::size_t i = shapes.size(); i-- > 0;) {
        const auto *circ = dynamic_cast<const Circle *>(shapes[i].get());
        if (!circ || circ->isFullCircle()) {
            continue;
        }
        const Circle::ArcData &arc = circ->getArcData();
        std::vector<Geometry::ArcSeg> segs =
            Geometry::clipArc(circ->getCenter(), circ->getRadius(), arc.ep1,
                              arc.ep2, arc.side, clipPoints);
        if (segs.empty()) {
            continue;
        }
        removeShape(i);
        for (const auto &sg : segs) {
            addShape(std::make_unique<Circle>(
                circ->getCenter(), circ->getRadius(), sg.a, sg.b, sg.side,
                circ->isFilled(), circ->getFillColor(), circ->isStroked(),
                circ->getLineWidth(), circ->getStrokeColor(),
                circ->getLineType()));
        }
        return;
    }
}
