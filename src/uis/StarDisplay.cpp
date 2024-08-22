#include "../utils/JsonLoading.h"
#include "../windows/MainWindow.h"
#include <memory>
#include <romhack_b3313_cartography/uis/StarDisplay.h>


void drawCourseStars(QPainter &painter, const QMap<QString, QMap<QString, QVector<StarData>>> &groupCourseMap, float startX, float starTextureHeight, float rectLeft, float rectTop, int &yOffset, int reservedHeight, const QImage &starCollectedTexture, const QImage &starMissingTexture) {
    float starSpacing = 64.0f;
    for (auto groupIt = groupCourseMap.cbegin(); groupIt != groupCourseMap.cend(); ++groupIt) {
        const QString &groupName = groupIt.key();
        const QMap<QString, QVector<StarData>> &courseStarsMap = groupIt.value();

        // Afficher le nom du groupe
        QFont groupFont = painter.font();
        groupFont.setBold(true);
        painter.setFont(groupFont);
        QRectF groupTextRect(rectLeft + 10, rectTop + yOffset, 600, 30);
        painter.setPen(Qt::blue);
        painter.drawText(groupTextRect, groupName);
        yOffset += std::max(static_cast<int>(starTextureHeight), 30);

        // Afficher les cours et les étoiles
        for (auto it = courseStarsMap.cbegin(); it != courseStarsMap.cend(); ++it) {
            const QString &courseName = it.key();
            const QVector<StarData> &stars = it.value();
            QFont font = painter.font();
            QRectF courseTextRect(rectLeft + 10, rectTop + yOffset, 600, 30);
            painter.setFont(font);
            painter.setPen(Qt::white);
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

        // Ajouter un espace après chaque groupe
        yOffset += 10;
    }
}

void generateTabContent(const QString &tabName, const QPixmap &pixmap, QWidget *contentWidget, QVBoxLayout *contentLayout) {
    while (QLayoutItem *item = contentLayout->takeAt(0)) {
        if (item->widget())
            delete item->widget();
        delete item;
    }
    auto tabLabel = std::make_unique<QLabel>(tabName, contentWidget);
    tabLabel->setFont(MainWindow::qfont);
    tabLabel->setStyleSheet("color: black;");
    contentLayout->addWidget(tabLabel.release());
    auto pixmapLabel = std::make_unique<QLabel>(contentWidget);
    pixmapLabel->setPixmap(pixmap);
    contentLayout->addWidget(pixmapLabel.release());
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
}

// Fonction principale pour afficher les étoiles
void StarDisplay::displayStars(const QJsonObject &jsonData) {
    if (!isRomHackLoaded(MainWindow::global_detected_emulator)) {
        MainWindow::tabWidget->hide();
        MainWindow::emulatorText->show();
        MainWindow::b3313Text->show();
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
            auto tabContent = std::make_unique<QWidget>();
            auto layout = std::make_unique<QVBoxLayout>(tabContent.get());
            contentWidget = new QWidget();
            contentLayout = new QVBoxLayout(contentWidget);
            contentWidget->setLayout(contentLayout);
            scrollArea = new QScrollArea(MainWindow::tabWidget);
            scrollArea->setWidgetResizable(true);
            scrollArea->setWidget(contentWidget);
            MainWindow::tabWidget->addTab(scrollArea, tabName);
        } else {
            scrollArea = qobject_cast<QScrollArea *>(existingTab);
            contentWidget = scrollArea->widget();
            contentLayout = qobject_cast<QVBoxLayout *>(contentWidget->layout());
        }
        int yOffset = 0;
        int reservedHeight = 50;
        QMap<QString, QMap<QString, QVector<StarData>>> groupCourseMap = JsonLoading::readStarDisplayJsonData(jsonData, saveData, params, i);
        for (auto groupIt = groupCourseMap.cbegin(); groupIt != groupCourseMap.cend(); ++groupIt) {
            const QMap<QString, QVector<StarData>> &courseStarsMap = groupIt.value();
            for (auto courseIt = courseStarsMap.cbegin(); courseIt != courseStarsMap.cend(); ++courseIt) {
                yOffset += std::max(static_cast<int>(starTextureHeight), 30);
            }
            yOffset += 10;
        }
        int totalHeight = yOffset + reservedHeight;
        QPixmap pixmap(MainWindow::graphicsView->width(), totalHeight);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        yOffset = 0;
        drawCourseStars(painter, groupCourseMap, 50 + 10, starTextureHeight, 50, 50, yOffset, reservedHeight, starCollectedTexture, starMissingTexture);
        painter.end();

        if (isNewTab)
            generateTabContent(tabName, pixmap, contentWidget, contentLayout);
        else
            contentWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        contentWidget->setMinimumHeight(totalHeight);
    }
    MainWindow::switchViewButton->show();
}
