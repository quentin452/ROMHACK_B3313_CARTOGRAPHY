#include "Textures.h"
QImage Textures::starCollectedImage, Textures::starMissingImage;
void Textures::InitTexturesAndImage() {
    starCollectedImage = ImageCache::getImage("resources/textures/star-collected.png");
    starMissingImage = ImageCache::getImage("resources/textures/star-missing.png");
}
