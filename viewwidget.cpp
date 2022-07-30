#include "viewwidget.h"
#include <QPainter>
#include <iostream>

ViewWidget::ViewWidget(QWidget *parent, std::vector<uint16_t> image)
    : QOpenGLWidget(parent), image_(image)
{
    setFixedSize(512, 512);
    setAutoFillBackground(false);
}

void ViewWidget::render()
{
    update();
}

void ViewWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    for (int i = 0; i < 512; i++)
    {
        for (int j = 0; j < 512; j++)
        {
            unsigned int value = (unsigned int)((float)image_[i + (j * 512)] / 2048 * 255);
            painter.setPen(QPen(QColor(value > 255 ? 255 : value, value > 255 ? 255 : value, value > 255 ? 255 : value)));
            painter.drawPoint(i, j);
        }
    }

    painter.end();
}
