#include "colorselection.h"

#include <QColorDialog>
#include <QFontMetrics>
#include <QPainter>

ColorSelection::ColorSelection(QWidget *parent)
    : QPushButton{parent}, color{0, 0, 0}
{
    auto fontHeight = QFontMetrics{font()}.height();
    setFixedSize(fontHeight, fontHeight);
    updateIcon();

    connect(this, &QPushButton::clicked, this, [this] {
        QColor newColor = QColorDialog::getColor(color, this, "选择颜色");
        if (newColor.isValid() && newColor != color) {
            color = newColor;
            updateIcon();
            emit colorChanged(RGBA{static_cast<uint8_t>(color.red()),
                                   static_cast<uint8_t>(color.green()),
                                   static_cast<uint8_t>(color.blue()),
                                   static_cast<uint8_t>(color.alpha())}

            );
        }
    });
}

void ColorSelection::updateIcon()
{
    QPixmap pm(size());
    pm.fill(color);
    QPainter p(&pm);
    p.setPen(Qt::darkGray);
    p.drawRect(0, 0, pm.width() - 1, pm.height() - 1);
    p.end();
    setIcon(QIcon(pm));
    setIconSize(size());
}