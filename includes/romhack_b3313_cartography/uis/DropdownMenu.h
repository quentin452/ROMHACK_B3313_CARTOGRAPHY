#pragma once

#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QPoint>
#include <QRectF>
#include <QString>
#include <vector>

class DropdownMenu {
public:
    DropdownMenu(float x, float y, const QPixmap &pixmap, const std::vector<QString> &items, const QFont &font);
    void draw(QPainter &painter);
    
    bool isClicked(const QPoint &mousePos);
    QString getSelectedItem(const QPoint &mousePos);

private:
    QPixmap background;
    QFont font;
    std::vector<QString> items;
    std::vector<QRectF> itemBounds;  // Store the bounding boxes of each item
    QPointF position;
};