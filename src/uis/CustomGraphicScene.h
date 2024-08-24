#pragma once
#include "../windows/MainWindow.h"
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

class CustomGraphicScene : public QGraphicsScene {
    Q_OBJECT

  public:
    CustomGraphicScene(QObject *parent = nullptr);

  private:
    QTimer *updateTimer = nullptr;
    QPointF lastMousePos;
    QGraphicsLineItem *previousLineItem = nullptr;
    QFuture<void> updateFuture;
    void updateLineItemComputations(); // New method for computations

  private slots:
    void onUpdateLineItem();

  protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;
};

QPointF getNodeEdgePoint(const Node *node, const QPointF &endPoint);