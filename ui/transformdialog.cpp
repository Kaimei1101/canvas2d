#include "transformdialog.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <numbers>

TransformDialog::TransformDialog(Point pivot, QWidget *parent) : QDialog(parent)
{
    setWindowTitle("应用变换");
    setMinimumWidth(300);

    spinTranslateX = new QDoubleSpinBox(this);
    spinTranslateX->setRange(-std::numeric_limits<double>::max(),
                             std::numeric_limits<double>::max());
    spinTranslateX->setDecimals(2);
    spinTranslateX->setSingleStep(50);

    spinTranslateY = new QDoubleSpinBox(this);
    spinTranslateY->setRange(-std::numeric_limits<double>::max(),
                             std::numeric_limits<double>::max());
    spinTranslateY->setDecimals(2);
    spinTranslateY->setSingleStep(50);

    spinScale = new QDoubleSpinBox(this);
    spinScale->setRange(-std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max());
    spinScale->setValue(1.0);
    spinScale->setDecimals(2);
    spinScale->setSingleStep(0.1);

    spinRotate = new QDoubleSpinBox(this);
    spinRotate->setRange(-std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max());
    spinRotate->setRange(-360.0, 360.0);
    spinRotate->setDecimals(1);
    spinRotate->setSingleStep(90.0);
    spinRotate->setSuffix(" °");

    spinPivotX = new QDoubleSpinBox(this);
    spinPivotX->setRange(-std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max());
    spinPivotX->setValue(pivot.x);
    spinPivotX->setDecimals(2);
    spinPivotX->setSingleStep(50);

    spinPivotY = new QDoubleSpinBox(this);
    spinPivotY->setRange(-std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max());
    spinPivotY->setValue(pivot.y);
    spinPivotY->setDecimals(2);
    spinPivotY->setSingleStep(50);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("X 轴平移:", spinTranslateX);
    formLayout->addRow("Y 轴平移:", spinTranslateY);
    formLayout->addRow("缩放倍数:", spinScale);
    formLayout->addRow("旋转角度:", spinRotate);
    formLayout->addRow("枢轴 X 位置:", spinPivotX);
    formLayout->addRow("枢轴 Y 位置:", spinPivotY);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
}

Transform TransformDialog::transform() const
{
    return Transform{
        Point{spinPivotX->value(), spinPivotY->value()},
        Point{spinTranslateX->value(), spinTranslateY->value()},
        spinScale->value(),
        spinRotate->value() / 180 * std::numbers::pi,
    };
}
