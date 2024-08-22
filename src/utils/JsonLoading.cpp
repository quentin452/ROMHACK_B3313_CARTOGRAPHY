#include "JsonLoading.h"

QJsonObject JsonLoading::loadJsonData2(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file:" << filePath;
        return QJsonObject(); // Return an empty QJsonObject on failure
    }
    QByteArray jsonData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isObject()) {
        qWarning() << "Invalid JSON format in file:" << filePath;
        return QJsonObject(); // Return an empty QJsonObject on failure
    }
    return jsonDoc.object(); // Return the parsed QJsonObject
}
void JsonLoading::loadNodesJsonData(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << filename;
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    if (doc.isNull() || !doc.isObject()) { // Vérifiez si c'est un QJsonObject
        qWarning() << "Failed to parse JSON or JSON is not an object.";
        return;
    }
    QJsonObject jsonObject = doc.object();
    MainWindow::lastJsonData = jsonObject; // Clear lastJsonData if not used
    parseNodesJsonData(jsonObject);
}

void JsonLoading::parseNodesJsonData(const QJsonObject &jsonObj) {
    MainWindow::nodes.clear();
    MainWindow::connections.clear();
    MainWindow::graphicsScene->clear();
    if (jsonObj.contains("nodes") && jsonObj["nodes"].isArray()) {
        QJsonArray nodesArray = jsonObj["nodes"].toArray();
        QHash<int, Node *> nodeIndexMap;
        for (int i = 0; i < nodesArray.size(); ++i) {
            QJsonObject nodeObj = nodesArray[i].toObject();
            Node *node = Node::fromJson(nodeObj, QFont("Arial", 12, QFont::Bold));
            MainWindow::graphicsScene->addItem(node);
            MainWindow::nodes.append(node);
            nodeIndexMap.insert(i, node);
        }
        if (jsonObj.contains("connections") && jsonObj["connections"].isObject()) {
            QJsonObject connectionsObj = jsonObj["connections"].toObject();
            if (connectionsObj.contains("connections") && connectionsObj["connections"].isArray()) {
                QJsonArray connectionsArray = connectionsObj["connections"].toArray();
                for (const QJsonValue &value : connectionsArray) {
                    QJsonObject connectionObj = value.toObject();
                    int startIndex = connectionObj["start"].toInt();
                    int endIndex = connectionObj["end"].toInt();
                    if (nodeIndexMap.contains(startIndex) && nodeIndexMap.contains(endIndex)) {
                        Node *startNode = nodeIndexMap[startIndex];
                        Node *endNode = nodeIndexMap[endIndex];
                        if (startNode && endNode) {
                            MainWindow::connections.push_back(QPair<int, int>(startIndex, endIndex));
                            MainWindow::nodes[startIndex]->addConnection(endIndex);
                            MainWindow::nodes[endIndex]->addConnection(startIndex);
                        }
                    } else {
                        qWarning() << "Invalid node index in connection.";
                    }
                }
            }
        }
    }
}

QMap<QString, QMap<QString, QVector<StarData>>> JsonLoading::readStarDisplayJsonData(const QJsonObject &jsonData, const std::vector<uint8_t> &saveData, const SaveParams &params, int slotIndex) {
    QMap<QString, QMap<QString, QVector<StarData>>> groupCourseMap;
    for (const auto &groupValue : jsonData["groups"].toArray()) {
        QJsonObject group = groupValue.toObject();
        if (!group.contains("name") || !group.contains("courses"))
            continue;
        QString groupName = group["name"].toString();
        QMap<QString, QVector<StarData>> &courseStarsMap = groupCourseMap[groupName];
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
                        bool star_collected = isStarCollected(saveData, offset, bit, slotIndex, params.slotSize);
                        courseStarList.append({courseName, numStars, star_collected, offset, mask});
                    }
                }
            }
        }
    }
    return groupCourseMap;
}