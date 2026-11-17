#include <math.h>
#include "collision.h"

double calculateDistance(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}

const char *getRiskLevel(double distance) {
    if (distance <= ASTRA_CRITICAL_DISTANCE) return "CRITICAL";
    if (distance <= ASTRA_WARNING_DISTANCE)  return "WARNING";
    return "SAFE";
}
