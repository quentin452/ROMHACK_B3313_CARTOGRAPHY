#include "StarModel.h"
#include "../windows/MainWindow.h"

QVariant StarModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= starData.size())
        return QVariant();
    const StarData &star = starData[index.row()];
    if (role == Qt::DisplayRole) {
        return star.courseName;
    } else if (role == Qt::DecorationRole) {
        return star.collected ? Textures::starCollectedImage : Textures::starMissingImage;
    } else if (role == Qt::UserRole) {
        return QVariant::fromValue(scaledLogoTexture); // THIS IS NEVER REACHED
    }
    return QVariant();
}

void StarModel::setStarData(const QVector<StarData> &data) {
    beginResetModel();
    starData = data;
    endResetModel();
}

void StarModel::setScaledLogoTexture(const QImage &texture) {
    scaledLogoTexture = texture;
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}

void StarModel::handleItemClicked(const QModelIndex &index) {
    if (index.isValid()) {
        const StarData &star = starData[index.row()];
        Node *associatedNode = MainWindow::findAssociatedNode(star.courseName);
        if (associatedNode) {
            MainWindow::force_toggle_star_display = true;
            MainWindow::graphicsView->centerOn(associatedNode->pos());
        }
    }
}
