#pragma once
#include <iostream>
#include <romhack_b3313_cartography/uis/Textures.h>
#include <romhack_b3313_cartography/utils/qt_includes.hpp>
#include <romhack_b3313_cartography/utils/rom_utils.h>

#include <vector>

struct StarData {
    QString courseName;
    int numStars;
    bool collected = false;
    int offset;
    int mask;

    StarData() : numStars(0), collected(false), offset(0), mask(0) {}

    StarData(const QString &courseName, int numStars, bool collected, int offset, int mask)
        : courseName(courseName), numStars(numStars), collected(collected), offset(offset), mask(mask) {}
};
class StarDisplay {
  public:
    void displayStars(const QJsonObject &jsonData);

  private:
    QRectF groupTextRect;
};