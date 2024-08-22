#include <romhack_b3313_cartography/utils/enums.h>
NodeShapes stringToShape(const QString &shapeString) {
    if (shapeString == "Circle")
        return Circle;
    if (shapeString == "Square")
        return Square;
    if (shapeString == "Triangle")
        return Triangle;
    return Circle;
}