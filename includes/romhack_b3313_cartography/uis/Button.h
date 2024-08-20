#pragma once

#include <QPixmap>
#include <QPainter>
#include <QRectF>
#include <QPoint>

class Button {
public:
    Button(float x, float y, const QPixmap &pixmap);

    void draw(QPainter &painter);

    bool isClicked(const QPoint &mousePos);

private:
    QPixmap pixmap;
    QRectF boundingBox;
};