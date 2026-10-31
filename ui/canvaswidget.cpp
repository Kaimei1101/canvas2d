#include "canvaswidget.h"

#include <QApplication>
#include <algorithm>
#include <cmath>
#include <memory>

#include "drawable/beziercurve.h"
#include "drawable/bspline.h"
#include "drawable/circle.h"
#include "drawable/line.h"
#include "drawable/polygon.h"
#include "geometry.h"
#include "transform.h"
#include "transformdialog.h"

CanvasWidget::CanvasWidget()
    : canvas{0, 0}, tool{Tool::Select}, lastPos{0, 0}, arcCenter{0, 0},
      arcRadius{0}, arcSecA{0, 0}, arcSecB{0, 0}, arcEp1{0, 0}, arcEp2{0, 0},
      lineWidth{1}, stroke{true}, fill{false}, strokeColor{0, 0, 0},
      fillColor{0, 0, 0}

{
    canvas.setScale(devicePixelRatio());
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    updateTimer = new QTimer(this);
    updateTimer->setInterval(16);
    connect(updateTimer, &QTimer::timeout, this, &CanvasWidget::processInput);
    updateTimer->start();
}

// 辅助标记：以 p 为中心的红色小十字（参照 ArcTool 的交点标记）
static void drawCross(Canvas &canvas, Point p)
{
    auto radius = 5.0 / canvas.getCamera().scale;
    auto width = 3.0 / canvas.getCamera().scale;
    RGBA red{220, 0, 0};
    canvas.drawShape(Line{Point{p.x - radius, p.y}, Point{p.x + radius, p.y},
                          width, red, LineType{}});
    canvas.drawShape(Line{Point{p.x, p.y - radius}, Point{p.x, p.y + radius},
                          width, red, LineType{}});
}

void CanvasWidget::paintEvent(QPaintEvent *)
{
    canvas.redraw();
    Point worldPos = canvas.getCamera().screenToWorld(lastPos);

    switch (tool) {
    case Tool::Select:
        if (isLeftDrag && selDragtype == SelDragType::Select) {
            canvas.drawShape(Polygon{
                std::vector<Point>{leftDragPos,
                                   Point{leftDragPos.x, worldPos.y}, worldPos,
                                   Point{worldPos.x, leftDragPos.y}},
                true, RGBA{24, 144, 255, 30}, true, 0, RGBA{24, 144, 255},
                LineType{}});
        }
        break;

    case Tool::Line:
        if (!operations.empty()) {
            canvas.drawShape(Line{operations.back(), worldPos, lineWidth,
                                  strokeColor, lineType});
        }
        break;

    case Tool::Circle:
        if (!operations.empty()) {
            const auto &center = operations.back();
            canvas.drawShape(Circle{center, worldPos, fill, fillColor, stroke,
                                    lineWidth, strokeColor, lineType});
        }
        break;

    case Tool::Arc:
        // 圆弧工具预览（参照 ArcTool::rebuildPreview）：
        // 圆：用当前画笔，所见即所得；割线：灰色虚线；弧端点：红色十字
        if (arcPhase != ArcPhase::Idle && arcRadius > 0) {
            canvas.drawShape(Circle{arcCenter, arcRadius, false, fillColor,
                                    stroke, lineWidth, strokeColor, lineType});
        }
        if (arcPhase == ArcPhase::SecantDown ||
            arcPhase == ArcPhase::PickSide) {
            canvas.drawShape(Line{arcSecA, arcSecB, 0, RGBA{150, 150, 150},
                                  LineType{{4, 4, 4, 4}}});
        }
        if (arcPhase == ArcPhase::PickSide) {
            drawCross(canvas, arcEp1);
            drawCross(canvas, arcEp2);
        }
        break;

    case Tool::ThreePointsArc:
        if (operations.size() <= 2) {
            for (auto p : operations) {
                drawCross(canvas, p);
            }
        }
        if (operations.size() == 2) {
            auto c = Circle::fromThreePoints(operations[0], operations[1],
                                             worldPos, stroke, lineWidth,
                                             strokeColor, lineType);
            if (c) {
                canvas.drawShape(*c);
            }
        }
        break;

    case Tool::Polygon: {
        if (operations.empty()) {
            break;
        }
        auto verts = operations;
        verts.push_back(worldPos);
        canvas.drawShape(Polygon{std::move(verts), fill, fillColor, stroke,
                                 lineWidth, strokeColor, lineType});
        break;
    }

    case Tool::Curve: {
        if (operations.empty()) {
            break;
        }
        auto pts = operations;
        pts.push_back(worldPos);
        if (pts.size() >= 2) {
            for (auto p : pts) {
                drawCross(canvas, p);
            }
        }
        if (pts.size() == 2) {
            canvas.drawShape(
                Line{pts[0], pts[1], lineWidth, strokeColor, lineType});
        } else if (pts.size() >= 3) {
            auto degree = std::min(3, static_cast<int>(pts.size()) - 1);
            canvas.drawShape(BSpline{std::move(pts), degree, lineWidth,
                                     strokeColor, lineType});
        }
        break;
    }

    case Tool::BezierCurve: {
        if (operations.empty()) {
            break;
        }
        auto pts = operations;
        pts.push_back(worldPos);
        if (pts.size() >= 2) {
            for (auto p : pts) {
                drawCross(canvas, p);
            }
        }
        if (pts.size() == 2) {
            canvas.drawShape(
                Line{pts[0], pts[1], lineWidth, strokeColor, lineType});
        } else if (pts.size() >= 3) {
            canvas.drawShape(
                BezierCurve{std::move(pts), lineWidth, strokeColor, lineType});
        }
        break;
    }

    case Tool::Clip:
        // 裁剪窗口预览：已确认顶点构成的凸多边形轮廓 + 鼠标悬停反馈
        if (operations.empty()) {
            break;
        }
        const RGBA guide{90, 90, 90};
        const RGBA okColor{0, 130, 0};
        const LineType dash{{5, 4, 5, 4}};
        const std::size_t n = operations.size();

        // 已确认边
        for (std::size_t i = 0; i + 1 < n; ++i) {
            canvas.drawShape(
                Line{operations[i], operations[i + 1], 0, guide, LineType{}});
        }
        if (n >= 3) {
            // 闭合边
            canvas.drawShape(Line{operations.back(), operations.front(), 1,
                                  guide, LineType{}});
        }

        // 悬停的新点
        if (n == 1) {
            canvas.drawShape(Line{operations[0], worldPos, 0, guide, dash});
        } else if (n == 2) {
            std::vector<Point> tri;
            if (Geometry::makeTriangle(operations[0], operations[1], worldPos,
                                       tri)) {
                // 三角形合法：绿色虚线预览闭合轮廓
                canvas.drawShape(
                    Line{operations[0], worldPos, 0, okColor, dash});
                canvas.drawShape(
                    Line{operations[1], worldPos, 0, okColor, dash});
            } else {
                canvas.drawShape(Line{operations[1], worldPos, 0, guide, dash});
            }
        } else {
            // n>=3：判定鼠标点对哪条边异侧
            const int edge = Geometry::convexInsertEdge(operations, worldPos);
            // 橡皮线：连到最近的待插入位置
            if (edge >= 0) {
                const Point &a = operations[edge];
                const Point &b = operations[(edge + 1) % n];
                // 未落下时以“这条边 + 新增端点”绘制三角形（绿色高亮）
                canvas.drawShape(Line{a, worldPos, 0, okColor, dash});
                canvas.drawShape(Line{b, worldPos, 0, okColor, dash});
                canvas.drawShape(Line{a, b, 0, okColor, LineType{}});
            } else {
                // 不合法：只显示到重心的灰色提示线
                const Point c = Geometry::centroid(operations);
                canvas.drawShape(Line{c, worldPos, 0, guide, dash});
            }
        }
        break;
    }

    QImage img{canvas.getRenderTarget().getPixels().as_raw(), canvas.getWidth(),
               canvas.getHeight(), QImage::Format_RGBA8888};
    img.setDevicePixelRatio(devicePixelRatio());
    QPainter{this}.drawImage(0, 0, img);
}

void CanvasWidget::resizeEvent(QResizeEvent *e)
{
    canvas.resize(e->size().width() * devicePixelRatio(),
                  e->size().height() * devicePixelRatio());
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *e)
{
    auto position = toPoint(e->position());
    if (e->buttons() & Qt::MiddleButton) {
        canvas.moveCamera(lastPos - position);
        update();
    }

    if (!(e->buttons() & Qt::LeftButton)) {
        isLeftDrag = false;
    }
    auto worldPos = canvas.getCamera().screenToWorld(position);
    if (tool == Tool::Select) {
        do {
            if (!(e->buttons() & Qt::LeftButton)) {
                auto tol = 7.0 * devicePixelRatio() / canvas.getCamera().scale;
                if (!canvas.getSelected().empty() &&
                    canvas.pivot.distanceSqr(worldPos) <= tol * tol) {
                    selDragtype = SelDragType::MovePivot;
                    setCursor(Qt::CrossCursor);
                    break;
                }

                bool mod = false;
                for (auto i : canvas.getSelected()) {
                    modArg = canvas.getShapes()[i]->getModificationHandler(
                        canvas.getCamera(), worldPos);
                    if (modArg) {
                        mod = true;
                        break;
                    }
                }
                if (mod) {
                    selDragtype = SelDragType::Mod;
                    setCursor(Qt::CrossCursor);
                    break;
                }

                bool move = false;
                for (auto i : canvas.getSelected()) {
                    if (canvas.getShapes()[i]->hitTest(canvas.getCamera(),
                                                       worldPos)) {
                        move = true;
                        break;
                    }
                }
                if (move) {
                    selDragtype = SelDragType::Move;
                    setCursor(Qt::SizeAllCursor);
                    break;
                }

                selDragtype = SelDragType::Select;
                unsetCursor();
                break;
            }
            if (!isLeftDrag) {
                auto startDragDis = QApplication::startDragDistance();
                if (worldPos.distanceSqr(leftDragPos) >
                    startDragDis * startDragDis) {
                    isLeftDrag = true;
                    if (selDragtype == SelDragType::Move) {
                        canvas.applyTransform(
                            Transform{worldPos - leftDragPos});
                        update();
                    } else if (selDragtype == SelDragType::MovePivot) {
                        canvas.pivot = canvas.pivot + (worldPos - leftDragPos);
                        update();
                    } else if (selDragtype == SelDragType::Mod) {
                        if (modArg) {
                            modArg->second(worldPos -
                                           (leftDragPos - modArg->first));
                        }
                        update();
                    }
                }
            } else if (selDragtype == SelDragType::Move) {
                canvas.applyTransform(Transform{
                    worldPos - canvas.getCamera().screenToWorld(lastPos)});
                update();
            } else if (selDragtype == SelDragType::MovePivot) {
                canvas.pivot =
                    canvas.pivot +
                    (worldPos - canvas.getCamera().screenToWorld(lastPos));
                update();
            } else if (selDragtype == SelDragType::Mod) {
                if (modArg) {
                    modArg->second(worldPos - (leftDragPos - modArg->first));
                }
                update();
            } else {
                update();
            }
        } while (false);
    } else if (tool == Tool::Arc) {
        auto worldPos = canvas.getCamera().screenToWorld(position);
        switch (arcPhase) {
        case ArcPhase::CenterDown:
            // 拖动中：更新半径预览
            arcRadius = arcCenter.distance(worldPos);
            update();
            break;
        case ArcPhase::SecantDown:
            // 拖动中：更新割线端点
            arcSecB = worldPos;
            update();
            break;
        case ArcPhase::RadiusDone:
        case ArcPhase::PickSide:
            // 静态预览，无需更新
            break;
        default:
            break;
        }
    } else if (!operations.empty()) {
        update();
    }

    lastPos = position;
    emit mouseWorldPos(canvas.getCamera().screenToWorld(position));
    e->accept();
}

void CanvasWidget::handleRightClick(QMouseEvent *e)
{
    auto pos = toPoint(e->position());
    auto worldPos = canvas.getCamera().screenToWorld(pos);

    if (tool == Tool::Line) {
        if (!operations.empty()) {
            canvas.addShape(std::make_unique<Line>(
                operations.back(), worldPos, lineWidth, strokeColor, lineType));
            operations.pop_back();
            update();
        }
    } else if (tool == Tool::ThreePointsArc) {
        operations.push_back(worldPos);
        if (operations.size() == 3) {
            auto c = Circle::fromThreePoints(operations[0], operations[1],
                                             operations[2], stroke, lineWidth,
                                             strokeColor, lineType);
            if (c) {
                canvas.addShape(std::make_unique<Circle>(std::move(*c)));
            }
        }
        operations.clear();
    } else if (tool == Tool::Arc) {
        // 圆弧工具：右键随时取消
        if (arcPhase != ArcPhase::Idle) {
            arcPhase = ArcPhase::Idle;
        }
    } else if (tool == Tool::Polygon) {
        // 多边形工具：右键完成（≥3 点提交为正式多边形，不足 3 点取消）
        operations.push_back(worldPos);
        if (operations.size() >= 3) {
            canvas.addShape(std::make_unique<Polygon>(
                std::move(operations), fill, fillColor, stroke, lineWidth,
                strokeColor, lineType));
        }
        operations.clear();
    } else if (tool == Tool::Curve) {
        // 曲线工具：右键完成（=2 点退化为直线，≥3 点提交为B样条，否则取消）
        operations.push_back(worldPos);
        if (operations.size() == 2) {
            canvas.addShape(std::make_unique<Line>(operations[0], operations[1],
                                                   lineWidth, strokeColor,
                                                   lineType));
        } else if (operations.size() >= 3) {
            auto degree = std::min(3, static_cast<int>(operations.size()) - 1);
            canvas.addShape(std::make_unique<BSpline>(std::move(operations),
                                                      degree, lineWidth,
                                                      strokeColor, lineType));
        }
        operations.clear();
    } else if (tool == Tool::BezierCurve) {
        // 贝塞尔曲线工具：右键完成（=2 点退化为直线，≥3
        // 点提交为B样条，否则取消）
        operations.push_back(worldPos);
        if (operations.size() == 2) {
            canvas.addShape(std::make_unique<Line>(operations[0], operations[1],
                                                   lineWidth, strokeColor,
                                                   lineType));
        } else if (operations.size() >= 3) {
            canvas.addShape(std::make_unique<BezierCurve>(
                std::move(operations), lineWidth, strokeColor, lineType));
        }
        operations.clear();
    } else if (tool == Tool::Clip) {
        // 裁剪工具：右键完成窗口并执行裁剪（不足 3 点取消）
        if (operations.size() >= 3) {
            canvas.applyClip(operations);
        }
        operations.clear();
    }
}

void CanvasWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton) {
        handleRightClick(e);
        update();
        e->accept();
        return;
    }

    auto worldPos = canvas.getCamera().screenToWorld(toPoint(e->position()));

    if (e->button() == Qt::LeftButton) {
        isLeftDrag = false;
        leftDragPos = worldPos;

        if (tool == Tool::Arc) {
            switch (arcPhase) {
            case ArcPhase::Idle:
                // 第一次按下：确定圆心
                arcCenter = worldPos;
                arcRadius = 0;
                arcPhase = ArcPhase::CenterDown;
                update();
                break;

            case ArcPhase::RadiusDone:
                // 第二次按下：开始画割线
                arcSecA = worldPos;
                arcSecB = worldPos;
                arcPhase = ArcPhase::SecantDown;
                update();
                break;

            case ArcPhase::PickSide: {
                // 第三次点击：确定保留侧（相对 ep1→ep2 方向表达）
                const double dx = arcEp2.x - arcEp1.x;
                const double dy = arcEp2.y - arcEp1.y;
                const double cross =
                    dx * (worldPos.y - arcEp1.y) - dy * (worldPos.x - arcEp1.x);
                if (std::abs(cross) < 1e-6) {
                    // 点击恰在割线上，无效，保持选侧状态
                    break;
                }
                canvas.addShape(std::make_unique<Circle>(
                    arcCenter, arcRadius, arcEp1, arcEp2, cross > 0 ? 1 : -1,
                    stroke, lineWidth, strokeColor, lineType));
                arcPhase = ArcPhase::Idle;
                update();
                break;
            }

            default:
                break;
            }
            e->accept();
            return;
        }

        switch (tool) {
        case Tool::Select: {
            // auto tol = 7.0 * devicePixelRatio() / canvas.getCamera().scale;
            break;
        }
        case Tool::Line:
            if (!operations.empty()) {
                canvas.addShape(std::make_unique<Line>(operations.back(),
                                                       worldPos, lineWidth,
                                                       strokeColor, lineType));
                operations.pop_back();
                update();
            }
            operations.push_back(worldPos);
            break;
        case Tool::Circle:
            if (!operations.empty()) {
                const auto &center = operations.back();
                canvas.addShape(std::make_unique<Circle>(
                    center, worldPos, fill, fillColor, stroke, lineWidth,
                    strokeColor, lineType));
                operations.pop_back();
                update();
            } else {
                operations.push_back(worldPos);
            }
            break;
        case Tool::ThreePointsArc:
            operations.push_back(worldPos);
            if (operations.size() == 3) {
                auto c = Circle::fromThreePoints(
                    operations[0], operations[1], operations[2], stroke,
                    lineWidth, strokeColor, lineType);
                if (c) {
                    canvas.addShape(std::make_unique<Circle>(std::move(*c)));
                }
                operations.clear();
            }
            break;
        case Tool::Polygon:
        case Tool::Curve:
            operations.push_back(worldPos);
            break;
        case Tool::BezierCurve:
            operations.push_back(worldPos);
            // Accept unlimited control points to demonstrate the difference
            // between BSpline and BezierCurve
            // if (operations.size() == 4) {
            //     canvas.addShape(std::make_unique<BezierCurve>(
            //         std::move(operations), lineWidth, strokeColor,
            //         lineType));
            //     operations.clear();
            // }
            break;
        case Tool::Fill: {
            if (!canvas.applyFill(worldPos, fillColor)) {
                emit showStatusMessage("填充区域超出视窗");
            }
            update();
            break;
        }
        case Tool::Clip: {
            // 实验二-4：凸多边形裁剪窗口的自动排序增量构造
            const std::size_t n = operations.size();
            if (n == 0) {
                operations.push_back(worldPos);
            } else if (n == 1) {
                if (operations[0].distance(worldPos) >= 1e-6) {
                    operations.push_back(worldPos);
                }
            } else if (n == 2) {
                // 第三个点：非共线才形成三角形，自动确定绕序
                std::vector<Point> tri;
                if (Geometry::makeTriangle(operations[0], operations[1],
                                           worldPos, tri)) {
                    operations = std::move(tri);
                }
                // 共线：视为没有点击
            } else {
                // n>=3：有且仅有一条边与重心异侧才合法，
                // 新点插入该边两端点之间；否则视为没有点击
                const int edge =
                    Geometry::convexInsertEdge(operations, worldPos);
                if (edge >= 0) {
                    operations.insert(operations.begin() + edge + 1, worldPos);
                }
            }
            update();
            break;
        }
        default:
            break;
        }
    }

    e->accept();
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *e) { mousePressEvent(e); }

void CanvasWidget::mouseReleaseEvent(QMouseEvent *e)
{
    auto worldPos = canvas.getCamera().screenToWorld(toPoint(e->position()));

    if (tool == Tool::Select && e->button() == Qt::LeftButton) {
        if (selDragtype == SelDragType::Move ||
            selDragtype == SelDragType::MovePivot ||
            selDragtype == SelDragType::Mod) {
            isLeftDrag = false;
            e->accept();
            return;
        }

        // 切换选择
        const bool ctrl = e->modifiers() & Qt::ControlModifier;
        // 追加选择
        const bool shift = e->modifiers() & Qt::ShiftModifier;

        if (!shift && !ctrl) {
            canvas.clearSelection();
        }

        auto select = [=, this](std::size_t i) {
            if (i >= 0) {
                if (ctrl) {
                    canvas.toggleSelection(i);
                } else {
                    canvas.select(i);
                }
            }
        };

        if (!isLeftDrag) {
            const auto &shapes = canvas.getShapes();
            auto hitIdx = std::ssize(shapes) - 1;
            for (; hitIdx >= 0; --hitIdx) {
                if (shapes[hitIdx]->hitTest(canvas.getCamera(), worldPos)) {
                    break;
                }
            }
            select(hitIdx);
            update();
        } else if (isLeftDrag) {
            AABB range{leftDragPos, worldPos};
            for (std::size_t i = 0; i < canvas.getShapes().size(); ++i) {
                if (range.contains(canvas.getShapes()[i]->bounds())) {
                    select(i);
                }
            }
            update();
        }
        isLeftDrag = false;
        e->accept();
        return;
    }

    if (tool == Tool::Arc && e->button() == Qt::LeftButton) {
        switch (arcPhase) {
        case ArcPhase::CenterDown:
            // 松开确定半径；半径过小视为无效
            arcRadius = arcCenter.distance(worldPos);
            if (arcRadius < 1) {
                arcPhase = ArcPhase::Idle;
            } else {
                arcPhase = ArcPhase::RadiusDone;
            }
            update();
            break;

        case ArcPhase::SecantDown: {
            arcSecB = worldPos;
            if (arcSecB == arcSecA) {
                // 未形成有效割线，回到画割线前的状态重新操作
                arcPhase = ArcPhase::RadiusDone;
                update();
                break;
            }
            auto ep = Geometry::intersectCircle(arcSecA, arcSecB, arcCenter,
                                                arcRadius);
            if (!ep || ep->first.distance(ep->second) < 2.0) {
                // 无交点（含相切退化）：创建整圆
                canvas.addShape(std::make_unique<Circle>(
                    arcCenter, arcRadius, fill, fillColor, stroke, lineWidth,
                    strokeColor, lineType));
                arcPhase = ArcPhase::Idle;
            } else {
                arcEp1 = ep->first;
                arcEp2 = ep->second;
                arcPhase = ArcPhase::PickSide;
            }
            update();
            break;
        }

        default:
            break;
        }
        e->accept();
        return;
    }

    e->accept();
}

void CanvasWidget::wheelEvent(QWheelEvent *e)
{
    double delta;
    if (!e->pixelDelta().isNull()) {
        delta = e->pixelDelta().y() / 50.0;
    } else {
        delta = e->angleDelta().y() / 120.0;
    }
    if (delta == 0.0) {
        return;
    }

    double base = e->modifiers() & Qt::ControlModifier ? 1.02 : 1.1;
    double factor = std::pow(base, delta);
    auto position = toPoint(e->position());
    canvas.zoomAt(position, factor);
    update();
    emit mouseWorldPos(canvas.getCamera().screenToWorld(position));
    emit scaleChanged(canvas.getScale() / devicePixelRatio());
    e->accept();
}

void CanvasWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        operations.clear();
        arcPhase = ArcPhase::Idle;
        canvas.clearSelection();
        update();
    } else if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) {
        canvas.removeShapes();
        modArg = std::nullopt;
        update();
    } else {
        if (!e->isAutoRepeat()) {
            pressedKeys.insert(e->key());
        }
        return;
    }
    e->accept();
}

Point CanvasWidget::toPoint(QPointF pos) const
{
    return Point{
        pos.x() * devicePixelRatio(),
        pos.y() * devicePixelRatio(),
    };
}

void CanvasWidget::showTransformDialog(bool)
{
    const auto &selected = canvas.getSelected();
    if (selected.empty()) {
        emit showStatusMessage("未选中任何图形");
        return;
    }

    Point sum{0, 0};
    for (size_t idx : selected) {
        sum = sum + canvas.getShapes()[idx]->centroid();
    }
    auto initPivot = sum / static_cast<double>(selected.size());
    TransformDialog dlg(initPivot, this);
    if (dlg.exec() == QDialog::Accepted) {
        canvas.applyTransform(dlg.transform());
        update();
    }
}

void CanvasWidget::keyReleaseEvent(QKeyEvent *e)
{
    if (!e->isAutoRepeat()) {
        pressedKeys.remove(e->key());
    }
    e->accept();
}

void CanvasWidget::focusOutEvent(QFocusEvent *e)
{
    pressedKeys.clear();
    QWidget::focusOutEvent(e);
}

void CanvasWidget::processInput()
{
    if (pressedKeys.isEmpty()) {
        return;
    }

    auto &selection = canvas.getSelected();
    if (selection.empty()) {
        return;
    }

    double dt = 0.016;
    double moveSpeed = 300.0 * dt;
    double zoomSpeed = 2.0 * dt;
    double rotateSpeed = 2.0 * dt;

    double moveX = 0.0, moveY = 0.0;
    if (pressedKeys.contains(Qt::Key_W)) {
        moveY += 1.0;
    }
    if (pressedKeys.contains(Qt::Key_S)) {
        moveY -= 1.0;
    }
    if (pressedKeys.contains(Qt::Key_A)) {
        moveX += 1.0;
    }
    if (pressedKeys.contains(Qt::Key_D)) {
        moveX -= 1.0;
    }

    double worldMoveX = 0, worldMoveY = 0;
    if (moveX != 0.0 || moveY != 0.0) {
        double length = std::sqrt(moveX * moveX + moveY * moveY);
        moveX /= length;
        moveY /= length;

        worldMoveX = moveX * moveSpeed;
        worldMoveY = moveY * moveSpeed;
    }

    double zoomFactor = 1.0;
    if (pressedKeys.contains(Qt::Key_Shift)) {
        zoomFactor *= (1.0 + zoomSpeed);
    }
    if (pressedKeys.contains(Qt::Key_Space)) {
        zoomFactor /= (1.0 + zoomSpeed);
    }

    double rotationDelta = 0.0;
    if (pressedKeys.contains(Qt::Key_Q)) {
        rotationDelta += rotateSpeed;
    }
    if (pressedKeys.contains(Qt::Key_E)) {
        rotationDelta -= rotateSpeed;
    }

    if (worldMoveX != 0 || worldMoveY != 0 || zoomFactor != 1.0 ||
        rotationDelta != 0.0) {
        Transform t{canvas.pivot, Point{worldMoveX, worldMoveY}, zoomFactor,
                    rotationDelta};
        canvas.applyTransform(t);
        update();
    }
}