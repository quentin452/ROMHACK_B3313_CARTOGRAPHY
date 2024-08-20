#ifndef TEXTURES_H
#define TEXTURES_H

#include <QPixmap>

struct Textures {
    QPixmap saveTexture;
    QPixmap starCollectedTexture;
    QPixmap starMissingTexture;
    QPixmap dropdownTexture;

    bool loadTextures() {
        if (!saveTexture.load(":/resources/textures/save_icon.png"))
            return false;
        if (!starCollectedTexture.load(":/resources/textures/star-collected.png"))
            return false;
        if (!starMissingTexture.load(":/resources/textures/star-missing.png"))
            return false;
        if (!dropdownTexture.load(":/resources/textures/dropdown_background.png"))
            return false;
        return true;
    }
};

extern Textures textures;

#endif // TEXTURES_H