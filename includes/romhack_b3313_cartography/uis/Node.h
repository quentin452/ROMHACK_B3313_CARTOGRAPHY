#pragma once

#include <romhack_b3313_cartography/utils/enums.h>
#include <romhack_b3313_cartography/utils/qt_includes.hpp>
#include <vector>

class Node : public QGraphicsEllipseItem {
  public:
    Node(float x, float y, const QString &text, const QFont &font, NodeShapes shape = Circle);

    void setMovable(bool movable);
    void setName(const QString &name);
    void setColor(const QColor &color);
    void setShape(NodeShapes shape);
    void addConnection(int nodeIndex);
    void removeConnection(int nodeIndex);
    void setModified(bool modified);

    QJsonObject toJson() const;
    static Node *fromJson(const QJsonObject &json, const QFont &defaultFont);
    void updateIsModified();
    bool isModified() const { return modified; }
    QString getName() const { return m_name; }
    QList<int> connections;

  protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

  private:
    QString shapeToString(NodeShapes shape) const;

    QString label;
    QFont font;
    QColor color;
    QString m_name;
    NodeShapes shape;
    bool modified;
    QGraphicsTextItem *labelItem;
    QList<QGraphicsPixmapItem *> starItems;
};
