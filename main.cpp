#include <QApplication>
#include <QImage>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>

#include "ui/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;

    window.show();

    return QGuiApplication::exec();
}