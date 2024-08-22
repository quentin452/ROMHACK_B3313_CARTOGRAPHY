#include <romhack_b3313_cartography/utils/TextureCache.h>
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
