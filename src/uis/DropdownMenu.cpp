#include <romhack_b3313_cartography/DropdownMenu.h>

DropdownMenu::DropdownMenu(float x, float y, const QPixmap &pixmap, const std::vector<QString> &items, const QFont &font)
    : background(pixmap), font(font), position(x, y), items(items) {

    // Calculer les limites des éléments
    for (size_t i = 0; i < items.size(); ++i) {
        QRectF textBounds(x, y + i * 30, background.width(), 30); // Hauteur d'un élément = 30px
        itemBounds.push_back(textBounds);
    }
}

void DropdownMenu::draw(QPainter &painter) {
    // Dessiner l'arrière-plan du menu déroulant
    painter.drawPixmap(position, background);

    // Dessiner les textes des éléments
    painter.setFont(font);
    painter.setPen(Qt::black);

    for (size_t i = 0; i < items.size(); ++i) {
        QPointF textPosition(position.x(), position.y() + i * 30); // Ajuster la position des éléments
        painter.drawText(textPosition, items[i]);
    }
}

bool DropdownMenu::isClicked(const QPoint &mousePos) {
    // Vérifier si la souris est au-dessus du menu déroulant
    QRectF backgroundBounds(position.x(), position.y(), background.width(), background.height());
    return backgroundBounds.contains(mousePos);
}

QString DropdownMenu::getSelectedItem(const QPoint &mousePos) {
    // Vérifier si la souris est au-dessus d'un des éléments
    for (size_t i = 0; i < itemBounds.size(); ++i) {
        if (itemBounds[i].contains(mousePos)) {
            return items[i];
        }
    }
    return "";
}