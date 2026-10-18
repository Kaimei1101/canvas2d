#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QMouseEvent>
#include <QPainter>
#include <QWidget>

#include "canvas.h"
#include "linetype.h"
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

    // 圆弧绘制状态机（参照 VectorCanvas2d ArcTool）：
    //   第一次按下/松开：确定圆心、半径；
    //   第二次按下/松开：画出割线——无交点（含相切退化）创建整圆，
    //                    有交点则交点为弧端点，进入选侧状态；
    //   第三次点击：确定保留割线哪一侧的弧段。
    enum class ArcPhase { Idle, CenterDown, RadiusDone, SecantDown, PickSide };
    ArcPhase arcPhase;
    Point arcCenter;        // 圆心
    double arcRadius;       // 半径
    Point arcSecA, arcSecB; // 割线端点
    Point arcEp1, arcEp2;   // 割线与圆的交点（弧端点）

  public:
    double lineWidth;
    LineType lineType;
    bool stroke;
    bool fill;
    RGBA strokeColor;
    RGBA fillColor;

    CanvasWidget();

    inline void setTool(Tool tool)
    {
        this->tool = tool;
        operations.clear();
        arcPhase = ArcPhase::Idle;
        update();
    }

  protected:
    inline QSize sizeHint() const override { return QSize(800, 600); }
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    Point toPoint(QPointF pos) const;
    void applyClip();

  public slots:
    void showTransformDialog(bool = false);

  signals:
    void mouseWorldPos(Point pos);
    void scaleChanged(double scale);
};

#endif // CANVASWIDGET_H
