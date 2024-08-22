#pragma once
#include "qt_includes.hpp"

class TextureCache {
  public:
    static QPixmap getTexture(const QString &path);

  private:
    static QHash<QString, QPixmap> cache;
};
