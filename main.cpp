#include <QApplication>
#include <QImage>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>

#include "ui/mainwindow.h"

class GridWidget : public QWidget
{
    std::vector<std::vector<float>> grid;
    QImage image;
    int gridW = 400, gridH = 400;

  public:
    GridWidget() : grid(400, std::vector<float>(400, 0.0f))
    {
        image = QImage(gridW, gridH, QImage::Format_RGB888);
        setMouseTracking(true);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;

    window.show();

    return QGuiApplication::exec();
}