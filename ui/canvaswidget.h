#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QMouseEvent>
#include <QPainter>
#include <QWidget>

#include "canvas.h"
#include "point.h"
#include "rgba.h"
#include "tool.h"

class CanvasWidget : public QWidget
{
    Q_OBJECT
    Canvas canvas;
    Tool tool;
    Point lastPos;
    // Store data of shapes which is being drawed
    std::vector<Point> operations;

  public:
    double lineWidth;
    bool stroke;
    bool fill;
    RGBA strokeColor;
    RGBA fillColor;

  public:
    CanvasWidget();

    inline void setTool(Tool tool)
    {
        this->tool = tool;
        operations.clear();
        update();
    }

  protected:
    inline QSize sizeHint() const override { return QSize(800, 600); }
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    Point toPoint(QPointF pos) const;

  signals:
    void mouseWorldPos(Point pos);
    void scaleChanged(double scale);
};

#endif // CANVASWIDGET_H
