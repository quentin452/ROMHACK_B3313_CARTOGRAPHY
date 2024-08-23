#include <romhack_b3313_cartography/utils/Caches.h>
QHash<QString, QPixmap> TextureCache::cache;
QPixmap TextureCache::getTexture(const QString &path) {
    if (!cache.contains(path)) {
        QPixmap pixmap(path);
        if (!pixmap.isNull()) {
            cache.insert(path, pixmap);
        } else {
            qDebug() << "Failed to load texture:" << path;
        }
    }
    return cache.value(path);
}
QHash<QString, QImage> ImageCache::cache;

QImage ImageCache::getImage(const QString &path) {
    if (!cache.contains(path)) {
        QImage image(path);
        if (!image.isNull()) {
            cache.insert(path, image);
        } else {
            qDebug() << "Failed to load image:" << path;
        }
    }
    return cache.value(path);
}