#include "../windows/MainWindow.h"
#include <romhack_b3313_cartography/uis/StarDisplay.h>
void drawCourseStars(QPainter &painter, const QMap<QString, QVector<StarData>> &courseStarsMap, float startX, float starTextureHeight, float rectLeft, float rectTop, int &yOffset, int reservedHeight, const QImage &starCollectedTexture, const QImage &starMissingTexture) {
    float starSpacing = 20.0f;
    for (auto it = courseStarsMap.cbegin(); it != courseStarsMap.cend(); ++it) {
        const QString &courseName = it.key();
        const QVector<StarData> &stars = it.value();
        QFont font = painter.font();
        QRectF courseTextRect(rectLeft + 10, rectTop + 40 + yOffset + reservedHeight, 600, 30);
        painter.setFont(font);
        painter.drawText(courseTextRect, courseName);
        float currentX = courseTextRect.right() + 10;
        for (const auto &star : stars) {
            for (int i = 0; i < star.numStars; ++i) {
                QRectF starRect(
                    currentX + i * starSpacing,
                    courseTextRect.top() + (30 - starTextureHeight) / 2,
                    starCollectedTexture.width(),
                    starCollectedTexture.height());
                const QImage &starTexture = star.collected ? starCollectedTexture : starMissingTexture;
                painter.drawImage(starRect, starTexture);
            }
            currentX += starSpacing * star.numStars;
        }
        yOffset += std::max(static_cast<int>(starTextureHeight), 30);
    }
}
void generateTabContent(const QString &tabName, const QPixmap &pixmap, QWidget *contentWidget, QVBoxLayout *contentLayout) {
    while (QLayoutItem *item = contentLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    // Ajoutez le nouveau contenu
    QLabel *tabLabel = new QLabel(tabName, contentWidget);
    tabLabel->setFont(MainWindow::qfont);
    tabLabel->setStyleSheet("color: black;");
    contentLayout->addWidget(tabLabel);
    QLabel *pixmapLabel = new QLabel(contentWidget);
    pixmapLabel->setPixmap(pixmap);
    contentLayout->addWidget(pixmapLabel);
    // Assurez-vous que les marges sont bien définies
    contentLayout->setContentsMargins(0, 0, 0, 0); // Pas de marges pour éviter de pousser le contenu vers le bas
    contentLayout->setSpacing(0);                  // Pas d'espace entre les widgets
}

// Fonction principale pour afficher les étoiles
void StarDisplay::displayStars(const QJsonObject &jsonData) {
    if (!isRomHackLoaded(MainWindow::global_detected_emulator)) {
        MainWindow::tabWidget->hide();
        MainWindow::emulatorText->show();
        MainWindow::b3313Text->show();
        MainWindow::switchViewButton->show();
        return;
    }
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
    if (saveData.empty() || params.numSlots <= 0)
        return;
    MainWindow::tabNames.clear();
    for (int i = 0; i < static_cast<int>(params.numSlots); ++i) {
        MainWindow::tabNames.append("Mario " + QString::number(i));
    }
    // Charger les textures des étoiles
    QImage starCollectedTexture("resources/textures/star-collected.png");
    QImage starMissingTexture("resources/textures/star-missing.png");
    if (starCollectedTexture.isNull() || starMissingTexture.isNull()) {
        qWarning() << "One or both star textures failed to load.";
        return;
    }
    float collectedHeight = static_cast<float>(starCollectedTexture.height());
    float missingHeight = static_cast<float>(starMissingTexture.height());
    float starTextureHeight = std::max(collectedHeight, missingHeight);
    for (int i = 0; i < static_cast<int>(params.numSlots); ++i) {
        QString tabName = MainWindow::tabNames[i];
        // Vérifiez si l'onglet existe déjà
        QWidget *existingTab = nullptr;
        for (int j = 0; j < MainWindow::tabWidget->count(); ++j) {
            if (MainWindow::tabWidget->tabText(j) == tabName) {
                existingTab = MainWindow::tabWidget->widget(j);
                break;
            }
        }
        QScrollArea *scrollArea;
        QWidget *contentWidget;
        QVBoxLayout *contentLayout;
        bool isNewTab = !existingTab;
        if (isNewTab) {
            // Créez un nouvel onglet
            QWidget *tabContent = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout(tabContent);
            contentWidget = new QWidget();
            contentLayout = new QVBoxLayout(contentWidget);
            contentWidget->setLayout(contentLayout);
            scrollArea = new QScrollArea(MainWindow::tabWidget);
            scrollArea->setWidgetResizable(true);
            scrollArea->setWidget(contentWidget);
            MainWindow::tabWidget->addTab(scrollArea, tabName);
        } else {
            // Mettre à jour le contenu de l'onglet existant
            scrollArea = qobject_cast<QScrollArea *>(existingTab);
            contentWidget = scrollArea->widget();
            contentLayout = qobject_cast<QVBoxLayout *>(contentWidget->layout());
        }
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
            if (!group.contains("name") || !group.contains("courses"))
                continue;
            QString groupName = group["name"].toString();
            QMap<QString, QVector<StarData>> courseStarsMap;
            for (const auto &courseValue : group["courses"].toArray()) {
                QJsonObject course = courseValue.toObject();
                if (!course.contains("name") || !course.contains("data"))
                    continue;
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
            drawCourseStars(painter, courseStarsMap, 50 + 10, starTextureHeight, 50, 50, yOffset, reservedHeight, starCollectedTexture, starMissingTexture);
            maxHeight = std::max(maxHeight, yOffset + reservedHeight);
        }
        painter.end();
        if (isNewTab)
            generateTabContent(tabName, pixmap, contentWidget, contentLayout);
        else
            contentWidget->setMinimumHeight(maxHeight + 50);
    }
    MainWindow::switchViewButton->show();
}