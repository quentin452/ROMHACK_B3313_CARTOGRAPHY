#pragma once

#include <romhack_b3313_cartography/utils/qt_includes.hpp>

#include <vector>

class Node : public QGraphicsEllipseItem {
  public:
    Node(float x, float y, const QString &text, const QFont &font);
    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;
    QJsonObject toJson() const;
    static Node *fromJson(const QJsonObject &json, const QFont &defaultFont);
    void setColor(const QColor &color);
    void setModified(bool modified) { this->modified = modified; }
    bool isModified() { return modified; }
    void setMovable(bool movable);
    void addConnection(int nodeIndex);
    void removeConnection(int nodeIndex);
    const QVector<int> &getConnections() const { return connections; }
    QVector<int> connections;
    void updateStar();
    void setPosition(float x, float y);

  private:
    QString label;
    QFont font;
    QColor color;
    QColor starColor;
    bool modified;

    float radius;
    QGraphicsTextItem *labelItem;
    QGraphicsPixmapItem *starItem;
};