#include <romhack_b3313_cartography/uis/StarDisplay.h>
#include <romhack_b3313_cartography/uis/Textures.h>
#include <romhack_b3313_cartography/utils/rom_utils.h>

void StarDisplay::afficherEtoilesGroupeFusionne(const QString &groupName, const QMap<QString, QVector<StarData>> &courseStarsMap, QPainter &painter, const QFont &font, int &yOffset, int &reservedHeight, const QRectF &windowRect) {
    // Définir les dimensions et la position du rectangle fixe
    const float rectWidth = 600;
    const float rectLeft = 50;
    const float rectTop = 50;
    const float rectHeight = windowRect.height() - rectTop; // Ajuste la hauteur du rectangle

    // Dessiner le rectangle fixe
    QRectF rectangle(rectLeft, rectTop, rectWidth, rectHeight);
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::transparent);
    painter.drawRect(rectangle);

    // Dessiner le texte du groupe
    painter.setFont(font);
    painter.setPen(Qt::black);
    QRectF groupTextRect(rectLeft + 100, rectTop + 100 + yOffset + reservedHeight, rectWidth, 30);
    painter.drawText(groupTextRect, groupName);

    yOffset += 30;

    // Préparer les noms des cours
    std::vector<QString> courseNames;
    for (auto it = courseStarsMap.cbegin(); it != courseStarsMap.cend(); ++it) {
        courseNames.push_back(it.key());
    }

    std::sort(courseNames.begin(), courseNames.end());

    // Charger les textures des étoiles
    QImage starCollectedTexture("resources/textures/star-collected.png");
    QImage starMissingTexture("resources/textures/star-missing.png");

    if (starCollectedTexture.isNull() || starMissingTexture.isNull()) {
        qWarning() << "One or both star textures failed to load.";
        return;
    }

    // Calculer la hauteur de la texture des étoiles
    float collectedHeight = static_cast<float>(starCollectedTexture.height());
    float missingHeight = static_cast<float>(starMissingTexture.height());
    float starTextureHeight = (collectedHeight > missingHeight) ? collectedHeight : missingHeight;

    // Dessiner les éléments pour chaque cours
    for (const auto &courseName : courseNames) {
        const QVector<StarData> &stars = courseStarsMap.value(courseName);
        QRectF courseTextRect(rectLeft + 100, rectTop + 130 + yOffset + reservedHeight, rectWidth, 30);
        painter.drawText(courseTextRect, courseName);

        float startX = rectLeft + 100 + painter.fontMetrics().horizontalAdvance(courseName) + 10;
        float starSpacing = 20.0f;

        for (const auto &star : stars) {
            for (int i = 0; i < star.numStars; ++i) {
                QRectF starRect(
                    startX + i * starSpacing,
                    rectTop + 130 + yOffset + (30 - starTextureHeight) / 2 + reservedHeight,
                    starCollectedTexture.width(),
                    starCollectedTexture.height());

                if (rectangle.contains(starRect)) {
                    // Choisir la texture selon que l'étoile est collectée ou non
                    const QImage &starTexture = star.collected ? starCollectedTexture : starMissingTexture;
                    painter.drawImage(starRect, starTexture);
                }
            }
            startX += starSpacing;
        }

        yOffset += (30 > static_cast<int>(starTextureHeight)) ? 30 : static_cast<int>(starTextureHeight);
    }

    yOffset += 30;
}
