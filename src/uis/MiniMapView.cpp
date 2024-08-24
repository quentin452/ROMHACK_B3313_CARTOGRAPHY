#include "MiniMapView.h"
void MiniMapView::mousePressEvent(QMouseEvent *event) {
    emit minimapClicked(event);
    QGraphicsView::mousePressEvent(event);
}
#include "MiniMapView.moc"