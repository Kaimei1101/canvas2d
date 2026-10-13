#include "canvaswidget.h"

#include <memory>

#include "drawable/circle.h"
#include "drawable/line.h"
#include "drawable/polygon.h"

CanvasWidget::CanvasWidget()
    : canvas{0, 0}, tool{Tool::Select}, lastPos{0, 0}, lineWidth{1},
      stroke{true}, fill{false}, strokeColor{0, 0, 0}, fillColor{0, 0, 0}
{
    canvas.setScale(devicePixelRatio());
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void CanvasWidget::paintEvent(QPaintEvent *e)
{
    canvas.redraw();
    switch (tool) {
    case Tool::Line:
        if (!operations.empty()) {
            canvas.drawShape(Line{operations.back(),
                                  canvas.getCamera().screenToWorld(lastPos),
                                  lineWidth, strokeColor});
        }
        break;
    case Tool::Circle:
        if (!operations.empty()) {
            const auto &center = operations.back();
            canvas.drawShape(
                Circle{center, canvas.getCamera().screenToWorld(lastPos), fill,
                       fillColor, stroke, lineWidth, strokeColor});
        }
        break;
    case Tool::Polygon:
        auto verts = operations;
        verts.push_back(canvas.getCamera().screenToWorld(lastPos));
        canvas.drawShape(Polygon{std::move(verts), fill, fillColor, stroke,
                                 lineWidth, strokeColor});
        break;
    }
    QImage img{canvas.getRenderTarget().getPixels().as_raw(), canvas.getwidth(),
               canvas.getheight(), QImage::Format_RGBA8888};
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
        canvas.move_camera(lastPos - position);
        update();
    }
    switch (tool) {
    case Tool::Select:
    case Tool::Line:
    case Tool::Circle:
    case Tool::Polygon:
        if (!operations.empty()) {
            update();
        }
        break;
    }
    lastPos = position;
    emit mouseWorldPos(canvas.getCamera().screenToWorld(position));
    e->accept();
}

void CanvasWidget::mousePressEvent(QMouseEvent *e)
{
    auto worldPosition =
        canvas.getCamera().screenToWorld(toPoint(e->position()));
    if (e->button() == Qt::LeftButton) {
        switch (tool) {
        case Tool::Line:
            if (!operations.empty()) {
                canvas.addShape(std::make_unique<Line>(
                    operations.back(), worldPosition, lineWidth, strokeColor));
                operations.pop_back();
                update();
            }
            operations.push_back(worldPosition);
            break;
        case Tool::Circle:
            if (!operations.empty()) {
                const auto &center = operations.back();
                canvas.addShape(std::make_unique<Circle>(
                    center, worldPosition, fill, fillColor, stroke, lineWidth,
                    strokeColor));
                operations.pop_back();
                update();
            } else {
                operations.push_back(worldPosition);
            }
            break;
        case Tool::Polygon:
            operations.push_back(worldPosition);
            break;
        }
    } else if (e->button() == Qt::RightButton) {
        switch (tool) {
        case Tool::Line:
            if (!operations.empty()) {
                canvas.addShape(std::make_unique<Line>(
                    operations.back(), worldPosition, lineWidth, strokeColor));
                operations.pop_back();
                update();
            }
            break;
        case Tool::Circle:
            if (!operations.empty()) {
                const auto &center = operations.back();
                canvas.addShape(std::make_unique<Circle>(
                    center, worldPosition, fill, fillColor, stroke, lineWidth,
                    strokeColor));
                operations.pop_back();
                update();
            } else {
                operations.push_back(worldPosition);
            }
            break;
        case Tool::Polygon:
            operations.push_back(worldPosition);
            if (operations.size() > 2) {
                canvas.addShape(std::make_unique<Polygon>(
                    std::move(operations), fill, fillColor, stroke, lineWidth,
                    strokeColor));
            }
            operations.clear();
            break;
        }
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
    if (e->key() == Qt::Key_Escape && !operations.empty()) {
        operations.clear();
        update();
    }
}

Point CanvasWidget::toPoint(QPointF pos) const
{
    return Point{
        pos.x() * devicePixelRatio(),
        pos.y() * devicePixelRatio(),
    };
}