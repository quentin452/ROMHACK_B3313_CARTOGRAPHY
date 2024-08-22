#pragma once
#include "../windows/MainWindow.h"
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

class JsonLoading {
  public:
    static QJsonObject loadJsonData2(const QString &filePath);
    static void loadNodesJsonData(const QString &filename);
    static QMap<QString, QMap<QString, QVector<StarData>>> readStarDisplayJsonData(const QJsonObject &jsonData, const std::vector<uint8_t> &saveData, const SaveParams &params, int slotIndex);

  private:
    static void parseNodesJsonData(const QJsonObject &jsonObj);
};