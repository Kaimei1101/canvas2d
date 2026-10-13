#ifndef COLORSELECTION_H
#define COLORSELECTION_H

#include <QPushButton>

#include "rgba.h"

class ColorSelection : public QPushButton
{
    Q_OBJECT

    QColor color;

  public:
    explicit ColorSelection(QWidget *parent = nullptr);
    inline QColor getColor() { return color; }

  signals:
    void colorChanged(RGBA color);

  private:
    void updateIcon();
};

#endif // COLORSELECTION_H
