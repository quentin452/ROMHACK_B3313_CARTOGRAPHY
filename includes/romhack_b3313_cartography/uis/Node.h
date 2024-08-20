#pragma once

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QJsonObject>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <vector>


class Node : public QGraphicsEllipseItem {
  public:
    Node(float x, float y, const QString &text, const QFont &font);

    QJsonObject toJson() const;
    static Node fromJson(const QJsonObject &json, const QFont &font);
    void setColor(const QColor &color);
    void setModified(bool modified) { this->modified = modified; }
    bool isModified() { return modified; }
    void setMovable(bool movable);
    void addConnection(int nodeIndex);
    void removeConnection(int nodeIndex);
    const QVector<int> &getConnections() const { return connections; }
    QVector<int> connections;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void updateStar();

  private:
    QString label;
    QFont font;
    QColor color;
    QColor starColor;
    bool modified;

    float radius;
    QRectF boundingBox;
    QGraphicsTextItem *labelItem;
    QGraphicsPixmapItem *starItem;
};