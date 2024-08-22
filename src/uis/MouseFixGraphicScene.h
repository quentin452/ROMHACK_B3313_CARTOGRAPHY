#pragma once
#include "../windows/MainWindow.h"
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

class MouseFixGraphicScene : public QGraphicsScene {
    Q_OBJECT

  public:
    MouseFixGraphicScene(QObject *parent = nullptr);

  private:
    QTimer *updateTimer = nullptr;
    QPointF lastMousePos;
    QGraphicsLineItem *previousLineItem = nullptr;
  private slots:
    void updateLineItem();

  protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

QPointF getNodeEdgePoint(const Node *node, const QPointF &endPoint);