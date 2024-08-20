#include <romhack_b3313_cartography/uis/Button.h>

Button::Button(float x, float y, const QPixmap &pixmap)
    : pixmap(pixmap), boundingBox(x, y, pixmap.width(), pixmap.height()) {}

void Button::draw(QPainter &painter) {
    painter.drawPixmap(boundingBox.topLeft(), pixmap);
}

bool Button::isClicked(const QPoint &mousePos) {
    return boundingBox.contains(mousePos);
}
