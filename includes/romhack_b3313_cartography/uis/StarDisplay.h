#pragma once
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

#include <iostream>
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
    void afficherEtoilesGroupeFusionne(const QString &groupName, const QMap<QString, QVector<StarData>> &courseStarsMap, QPainter &painter, const QFont &font, int &yOffset, float reservedHeight, const QRectF &windowRect);
};