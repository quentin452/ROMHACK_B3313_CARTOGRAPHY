#pragma once

#include <romhack_b3313_cartography/utils/Caches.h>
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
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    QJsonObject toJson() const;
    static Node *fromJson(const QJsonObject &json, const QFont &defaultFont);
    void updateIsModified();
    bool isModified() const { return modified; }
    QString getName() const { return m_name; }
    QColor getColor() const {
        return color;
    }
    void adjustNodeSize();
    bool isStarAssociated() const { return starAssociated; }
    void setStarAssociated(bool value) { starAssociated = value; }
    void setAssociatedCourse(const QString &courseName) { associatedCourse = courseName; }
    QString getAssociatedCourse() const { return associatedCourse; }
    QList<int> connections;
    NodeShapes shape;
    QGraphicsTextItem *labelItem;

  protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

  private:
    QString associatedCourse;
    QString shapeToString(NodeShapes shape) const;

    QString label;
    QFont font;
    QColor color;
    QString m_name;
    bool modified;
    QList<QGraphicsPixmapItem *> starItems;
    bool starAssociated;
};
