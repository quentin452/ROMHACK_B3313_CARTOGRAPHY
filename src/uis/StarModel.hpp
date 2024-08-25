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
    QVariant StarModel::data(const QModelIndex &index, int role) const {
        if (!index.isValid() || index.row() >= starData.size())
            return QVariant();

        const StarData &star = starData[index.row()];

        if (role == Qt::DisplayRole) {
            // Retourne le nom du cours pour l'affichage
            return star.courseName;
        } else if (role == Qt::DecorationRole) {
            // Retourne l'image de l'étoile en fonction de son état (collecté ou manquant)
            return star.collected ? starCollectedTexture : starMissingTexture;
        } else if (role == Qt::UserRole) {
            // Retourne l'image du logo pour les éléments associés, si nécessaire
            return QVariant::fromValue(scaledLogoTexture); // THIS IS NEVER REACHED
        }

        return QVariant();
    }
    void setStarData(const QVector<StarData> &data) {
        beginResetModel();
        starData = data;
        endResetModel();
    }

    void setScaledLogoTexture(const QImage &texture) {
        scaledLogoTexture = texture;
        // Optionnel: Émettre un signal pour indiquer que l'image a changé
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    }

  private:
    QImage scaledLogoTexture;
    QVector<StarData> starData;
    QImage starCollectedTexture = ImageCache::getImage("resources/textures/star-collected.png");
    QImage starMissingTexture = ImageCache::getImage("resources/textures/star-missing.png");
};
