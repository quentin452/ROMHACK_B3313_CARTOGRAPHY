#pragma once
#include <romhack_b3313_cartography/utils/Caches.h>
#include <romhack_b3313_cartography/utils/StarData.h>
#include <romhack_b3313_cartography/utils/qt_includes.hpp>
#include "../utils/Textures.h"

class StarModel : public QAbstractListModel {
    Q_OBJECT
  public:
    StarModel(QObject *parent = nullptr) : QAbstractListModel(parent) {
        if (QListView *view = qobject_cast<QListView *>(parent))
            connect(view, &QListView::clicked, this, &StarModel::handleItemClicked);
    }

    int StarModel::rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return starData.size();
    }
    QVariant data(const QModelIndex &index, int role) const;

    void setStarData(const QVector<StarData> &data);

    void setScaledLogoTexture(const QImage &texture);
  signals:
    void starClicked(const QString &courseName);

  private slots:
    void handleItemClicked(const QModelIndex &index);

  private:
    QImage scaledLogoTexture;
    QVector<StarData> starData;
};
