#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "canvaswidget.h"
#include "linetype.h"

class MainWindow : public QMainWindow
{
    CanvasWidget canvasWidget;

    LineType lineTypePresets[4];

  public:
    explicit MainWindow(QWidget *parent = nullptr);

  private:
    void setupToolBar();
    void setupStatusBar();
};

#endif // MAINWINDOW_H
