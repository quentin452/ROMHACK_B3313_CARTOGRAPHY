#include "../windows/MainWindow.h"
#include <romhack_b3313_cartography/uis/StarDisplay.h>

void StarDisplay::afficherEtoilesGroupeFusionne(const QString &groupName, const QMap<QString, QVector<StarData>> &courseStarsMap, QPainter &painter, const QFont &font, int &yOffset, int &reservedHeight, const QRectF &windowRect) {
    // Définir les dimensions et la position du rectangle englobant
    const float rectWidth = 600;
    const float rectLeft = 50;
    const float rectTop = 50;
    const float rectHeight = windowRect.height() - rectTop; // Ajuste la hauteur du rectangle

    // Dessiner le rectangle englobant
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

    // Dessiner les éléments pour chaque cours à l'intérieur du rectangle englobant
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

                // Choisir la texture selon que l'étoile est collectée ou non
                const QImage &starTexture = star.collected ? starCollectedTexture : starMissingTexture;
                painter.drawImage(starRect, starTexture);
            }
            startX += starSpacing;
        }

        yOffset += (30 > static_cast<int>(starTextureHeight)) ? 30 : static_cast<int>(starTextureHeight);
    }

    yOffset += 30;
}

void StarDisplay::displayStars(const QJsonObject &jsonData) {
    if (isRomHackLoaded(MainWindow::global_detected_emulator)) {
        MainWindow::tabWidget->show();
        MainWindow::emulatorText->hide();
        MainWindow::b3313Text->hide();
        std::string saveLocation = GetParallelLauncherSaveLocation();
        QJsonObject format = jsonData["format"].toObject();
        SaveParams params;
        params.saveFormat = parseSaveFormat(format["save_type"].toString().toStdString());
        params.slotsStart = format["slots_start"].toInt();
        params.slotSize = format["slot_size"].toInt();
        params.activeBit = format["active_bit"].toInt();
        params.numSlots = format["num_slots"].toInt();
        params.checksumOffset = format["checksum_offset"].toInt();
        auto saveData = ReadSrmFile(saveLocation, params);
        if (saveData.empty()) {
            return;
        }
        int numSlots = params.numSlots;
        if (numSlots <= 0) {
            return;
        }
        MainWindow::tabNames.clear();
        for (int i = 1; i <= numSlots; ++i) {
            MainWindow::tabNames.append("Mario " + QString::number(i));
        }

        for (int i = 0; i < numSlots; ++i) {
            QString tabName = MainWindow::tabNames[i];

            // Vérifiez si l'onglet existe déjà
            QWidget *existingTab = nullptr;
            for (int j = 0; j < MainWindow::tabWidget->count(); ++j) {
                if (MainWindow::tabWidget->tabText(j) == tabName) {
                    existingTab = MainWindow::tabWidget->widget(j);
                    break;
                }
            }

            if (existingTab) {
                // Mettre à jour le contenu de l'onglet existant
                QScrollArea *scrollArea = qobject_cast<QScrollArea *>(existingTab);
                QWidget *contentWidget = scrollArea->widget();
                QVBoxLayout *contentLayout = qobject_cast<QVBoxLayout *>(contentWidget->layout());

                // Supprimez les anciens widgets de contenu
                while (QLayoutItem *item = contentLayout->takeAt(0)) {
                    delete item->widget();
                    delete item;
                }

                // Ajoutez le nouveau contenu
                QLabel *tabLabel = new QLabel(tabName, contentWidget);
                tabLabel->setFont(MainWindow::qfont);
                tabLabel->setStyleSheet("color: black;");
                contentLayout->addWidget(tabLabel);

                // Créer un QPixmap de la taille de la vue graphique
                QPixmap pixmap(MainWindow::graphicsView->size());
                pixmap.fill(Qt::transparent);
                QPainter painter(&pixmap);
                painter.setRenderHint(QPainter::Antialiasing);

                int yOffset = 0;
                int reservedHeight = 0;
                int maxHeight = 0;

                // Accumuler les dessins pour chaque groupe
                for (const auto &groupValue : jsonData["groups"].toArray()) {
                    QJsonObject group = groupValue.toObject();
                    if (!group.contains("name") || !group.contains("courses")) {
                        continue;
                    }
                    QString groupName = group["name"].toString();
                    QMap<QString, QVector<StarData>> courseStarsMap;
                    for (const auto &courseValue : group["courses"].toArray()) {
                        QJsonObject course = courseValue.toObject();
                        if (!course.contains("name") || !course.contains("data")) {
                            continue;
                        }
                        QString courseName = course["name"].toString();
                        QVector<StarData> &courseStarList = courseStarsMap[courseName];
                        for (const auto &dataValue : course["data"].toArray()) {
                            QJsonObject data = dataValue.toObject();
                            int offset = data["offset"].toInt();
                            int mask = data["mask"].toInt();
                            int numStars = 1;
                            for (int bit = 0; bit < 32; ++bit) {
                                if (mask & (1 << bit)) {
                                    bool star_collected = isStarCollected(saveData, offset, bit, i, params.slotSize);
                                    courseStarList.append({courseName, numStars, star_collected, offset, mask});
                                }
                            }
                        }
                    }

                    afficherEtoilesGroupeFusionne(groupName, courseStarsMap, painter, MainWindow::qfont, yOffset, reservedHeight, scrollArea->viewport()->rect());
                    maxHeight = std::max(maxHeight, yOffset + reservedHeight);
                }

                painter.end();

                QLabel *pixmapLabel = new QLabel(contentWidget);
                pixmapLabel->setPixmap(pixmap);
                contentLayout->addWidget(pixmapLabel);

                // Assurez-vous que les marges sont bien définies
                contentLayout->setContentsMargins(0, 0, 0, 0); // Pas de marges pour éviter de pousser le contenu vers le bas
                contentLayout->setSpacing(0);                  // Pas d'espace entre les widgets

                // Définir la hauteur minimale de contentWidget
                contentWidget->setMinimumHeight(maxHeight + 50); // Ajustez cette valeur selon vos besoins
            } else {
                // Créez un nouvel onglet
                QWidget *tabContent = new QWidget();
                QVBoxLayout *layout = new QVBoxLayout(tabContent);

                QWidget *contentWidget = new QWidget();
                QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
                contentWidget->setLayout(contentLayout);

                QScrollArea *scrollArea = new QScrollArea(MainWindow::tabWidget);
                scrollArea->setWidgetResizable(true);
                scrollArea->setWidget(contentWidget);

                MainWindow::tabWidget->addTab(scrollArea, tabName);

                QLabel *tabLabel = new QLabel(tabName, tabContent);
                tabLabel->setFont(MainWindow::qfont);
                tabLabel->setStyleSheet("color: black;");
                contentLayout->addWidget(tabLabel);

                QPixmap pixmap(MainWindow::graphicsView->size());
                pixmap.fill(Qt::transparent);
                QPainter painter(&pixmap);
                painter.setRenderHint(QPainter::Antialiasing);

                int yOffset = 0;
                int reservedHeight = 0;
                int maxHeight = 0;

                for (const auto &groupValue : jsonData["groups"].toArray()) {
                    QJsonObject group = groupValue.toObject();
                    if (!group.contains("name") || !group.contains("courses")) {
                        continue;
                    }
                    QString groupName = group["name"].toString();
                    QMap<QString, QVector<StarData>> courseStarsMap;
                    for (const auto &courseValue : group["courses"].toArray()) {
                        QJsonObject course = courseValue.toObject();
                        if (!course.contains("name") || !course.contains("data")) {
                            continue;
                        }
                        QString courseName = course["name"].toString();
                        QVector<StarData> &courseStarList = courseStarsMap[courseName];
                        for (const auto &dataValue : course["data"].toArray()) {
                            QJsonObject data = dataValue.toObject();
                            int offset = data["offset"].toInt();
                            int mask = data["mask"].toInt();
                            int numStars = 1;
                            for (int bit = 0; bit < 32; ++bit) {
                                if (mask & (1 << bit)) {
                                    bool star_collected = isStarCollected(saveData, offset, bit, i, params.slotSize);
                                    courseStarList.append({courseName, numStars, star_collected, offset, mask});
                                }
                            }
                        }
                    }

                    afficherEtoilesGroupeFusionne(groupName, courseStarsMap, painter, MainWindow::qfont, yOffset, reservedHeight, scrollArea->viewport()->rect());
                    maxHeight = std::max(maxHeight, yOffset + reservedHeight);
                }

                painter.end();

                QLabel *pixmapLabel = new QLabel(contentWidget);
                pixmapLabel->setPixmap(pixmap);
                contentLayout->addWidget(pixmapLabel);

                contentLayout->setContentsMargins(0, 0, 0, 0);
                contentLayout->setSpacing(0);

                contentWidget->setMinimumHeight(maxHeight + 50);
            }
        }
    } else {
        MainWindow::tabWidget->hide();
        MainWindow::emulatorText->show();
        MainWindow::b3313Text->show();
    }
    MainWindow::switchViewButton->show();
}