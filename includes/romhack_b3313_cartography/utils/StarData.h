
#pragma once
#include <romhack_b3313_cartography/utils/qt_includes.hpp>
struct StarData {
    QString courseName;
    int numStars;
    bool collected = false;
    int offset;
    int mask;

    StarData() : numStars(0), collected(false), offset(0), mask(0) {}

    StarData(const QString &courseName, int numStars, bool collected, int offset, int mask)
        : courseName(courseName), numStars(numStars), collected(collected), offset(offset), mask(mask) {}
    bool operator==(const StarData &other) const {
        return courseName == other.courseName; 
    }
};
