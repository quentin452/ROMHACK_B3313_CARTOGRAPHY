#pragma once
#include "qt_includes.hpp"
enum NodeShapes {
    Circle,
    Square,
    Triangle
};
NodeShapes stringToShape(const QString &shapeString);