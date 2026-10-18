#ifndef TRANSFORMDIALOG_H
#define TRANSFORMDIALOG_H

#include <QDialog>

#include "transform.h"

class QDoubleSpinBox;

class TransformDialog : public QDialog
{
    Q_OBJECT
    QDoubleSpinBox *spinTranslateX;
    QDoubleSpinBox *spinTranslateY;
    QDoubleSpinBox *spinScale;
    QDoubleSpinBox *spinRotate;
    QDoubleSpinBox *spinPivotX;
    QDoubleSpinBox *spinPivotY;

  public:
    explicit TransformDialog(Point pivot, QWidget *parent = nullptr);

    Transform transform() const;
};

#endif // TRANSFORMDIALOG_H
