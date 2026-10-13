#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "canvaswidget.h"

class MainWindow : public QMainWindow
{
    CanvasWidget canvasWidget;

  public:
    explicit MainWindow(QWidget *parent = nullptr);

  private:
    void setupToolBar();
    void setupStatusBar();
};

#endif // MAINWINDOW_H
