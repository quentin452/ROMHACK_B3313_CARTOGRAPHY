#pragma once
#include <romhack_b3313_cartography/utils/Caches.h>
#include <romhack_b3313_cartography/utils/StarData.h>
#include <romhack_b3313_cartography/utils/qt_includes.hpp>


class StarModel : public QAbstractListModel {
    Q_OBJECT
  public:
    StarModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return starData.size();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= starData.size())
            return QVariant();

        const StarData &star = starData[index.row()];
        if (role == Qt::DisplayRole)
            return star.courseName;
        else if (role == Qt::DecorationRole)
            return star.collected ? starCollectedTexture : starMissingTexture;
        return QVariant();
    }

    void setStarData(const QVector<StarData> &data) {
        beginResetModel();
        starData = data;
        endResetModel();
    }

  private:
    QVector<StarData> starData;
    QImage starCollectedTexture = ImageCache::getImage("resources/textures/star-collected.png");
    QImage starMissingTexture = ImageCache::getImage("resources/textures/star-missing.png");
};
