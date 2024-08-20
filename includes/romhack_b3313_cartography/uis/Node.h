#pragma once

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QJsonObject>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <vector>

class Node : public QGraphicsEllipseItem {
  public:
    Node(float x, float y, const QString &text, const QFont &font);

    void setPosition(float x, float y);
    QJsonObject toJson() const;
    static Node fromJson(const QJsonObject &json, const QFont &font);
    void setColor(const QColor &color);
    std::vector<int> connections; // Indices of connected nodes
    void setModified(bool modified) { this->modified = modified; }
    bool isModified() { return modified; }

  private:
    QString label;
    QFont font;
    QColor color;
    QColor starColor;
    bool modified;

    float radius;
    QRectF boundingBox;
};