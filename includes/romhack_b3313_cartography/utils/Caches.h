#pragma once
#include "qt_includes.hpp"

class TextureCache {
  public:
    static QPixmap getTexture(const QString &path);

  private:
    static QHash<QString, QPixmap> cache;
};
class ImageCache {
  public:
    static QImage getImage(const QString &path);

  private:
    static QHash<QString, QImage> cache;
};
