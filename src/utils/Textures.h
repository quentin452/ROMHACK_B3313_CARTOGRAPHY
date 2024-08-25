#pragma once
#include <romhack_b3313_cartography/utils/Caches.h>
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

class Textures {
  public:
    static QImage starCollectedImage, starMissingImage;
    static void InitTexturesAndImage();
};
