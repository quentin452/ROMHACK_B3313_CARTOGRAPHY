#pragma once
#include <romhack_b3313_cartography/utils/qt_includes.hpp>


class Tab {
  public:
    Tab(const QString &name, float x, float y)
        : tabName(name), posX(x), posY(y), width(100), height(30) {}

    void draw(QPainter &painter, const QFont &font, bool isSelected) const {
        painter.setFont(font);

        // Draw the tab rectangle
        QRectF tabRect(posX, posY, width, height);
        if (isSelected) {
            painter.setBrush(QColor(Qt::yellow));
        } else {
            painter.setBrush(QColor(Qt::transparent));
        }
        painter.setPen(QColor(Qt::red));
        painter.drawRect(tabRect);

        // Draw the tab text
        painter.setPen(QColor(Qt::black));
        painter.drawText(tabRect.adjusted(10, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, tabName);
    }

    bool contains(const QPointF &mousePosition) const {
        QRectF bounds(posX, posY, width, height);
        return bounds.contains(mousePosition);
    }

    QString getName() const { return tabName; }

  private:
    QString tabName;
    float posX, posY;
    float width, height;
};