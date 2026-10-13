#include "mainwindow.h"

#include <QActionGroup>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>

#include "colorselection.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow{parent}
{
    setCentralWidget(&canvasWidget);
    setupToolBar();
    setupStatusBar();
}

void MainWindow::setupToolBar()
{
    auto *tb = addToolBar("工具");
    tb->setMovable(false);

    auto *group = new QActionGroup(this);
    group->setExclusive(true);

    auto addTool = [&](Tool tool, bool checked = false) {
        auto *act = tb->addAction(toolName(tool));
        act->setCheckable(true);
        act->setChecked(checked);
        act->setData(static_cast<int>(tool));
        group->addAction(act);

        connect(act, &QAction::triggered, this,
                [this, tool] { canvasWidget.setTool(tool); });
    };

    addTool(Tool::Select, true);
    addTool(Tool::Line);
    addTool(Tool::Circle);
    addTool(Tool::Polygon);
    addTool(Tool::Fill);

    tb->addSeparator();

    auto *strokeBtn = new QToolButton(this);
    strokeBtn->setText("边框");
    strokeBtn->setCheckable(true);
    strokeBtn->setChecked(true);
    tb->addWidget(strokeBtn);

    auto *fillBtn = new QToolButton(this);
    fillBtn->setText("填充");
    fillBtn->setCheckable(true);
    fillBtn->setChecked(false);
    tb->addWidget(fillBtn);

    auto ensureValid = [=] {
        if (!fillBtn->isChecked() && !strokeBtn->isChecked()) {
            fillBtn->setChecked(true);
        }
    };

    connect(fillBtn, &QToolButton::toggled, this,
            [ensureValid, strokeBtn, fillBtn, this](bool) {
                ensureValid();
                canvasWidget.stroke = strokeBtn->isChecked();
                canvasWidget.fill = fillBtn->isChecked();
            });

    connect(strokeBtn, &QToolButton::toggled, this,
            [ensureValid, strokeBtn, fillBtn, this](bool) {
                ensureValid();
                canvasWidget.stroke = strokeBtn->isChecked();
                canvasWidget.fill = fillBtn->isChecked();
            });

    tb->addSeparator();

    auto *widthSpin = new QDoubleSpinBox(this);
    widthSpin->setRange(0.01, 100.0);
    widthSpin->setValue(1.0);
    widthSpin->setSingleStep(0.5);
    widthSpin->setDecimals(2);
    widthSpin->setPrefix("线宽 ");
    widthSpin->setSuffix(" unit");
    tb->addWidget(widthSpin);

    connect(widthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double w) { canvasWidget.lineWidth = w; });
    canvasWidget.lineWidth = 1;

    tb->addSeparator();

    tb->addWidget(new QLabel("边框色 "));
    auto *strokeColorSel = new ColorSelection(this);
    tb->addWidget(strokeColorSel);

    connect(strokeColorSel, &ColorSelection::colorChanged, this,
            [this](RGBA c) { canvasWidget.strokeColor = c; });

    tb->addWidget(new QLabel(" 填充色 "));
    auto *fillColorSel = new ColorSelection(this);
    tb->addWidget(fillColorSel);

    connect(fillColorSel, &ColorSelection::colorChanged, this,
            [this](RGBA c) { canvasWidget.fillColor = c; });
}

void MainWindow::setupStatusBar()
{
    auto posLabel = new QLabel("-, -");
    auto scaleLabel = new QLabel("100.0%");

    statusBar()->addPermanentWidget(posLabel);
    statusBar()->addPermanentWidget(scaleLabel);

    connect(&canvasWidget, &CanvasWidget::mouseWorldPos, posLabel,
            [=](Point p) {
                posLabel->setText(
                    QString("%1, %2").arg(p.x, 0, 'f', 2).arg(p.y, 0, 'f', 2));
            });

    connect(&canvasWidget, &CanvasWidget::scaleChanged, scaleLabel,
            [=](double scale) {
                scaleLabel->setText(
                    QString("%1%").arg(scale * 100.0, 0, 'f', 1));
            });
}