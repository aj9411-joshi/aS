#ifndef ASTRA_COLLISION_H
#define ASTRA_COLLISION_H

#ifdef __cplusplus
extern "C" {
#endif

/* Distance thresholds (demo units) used to classify proximity. */
#define ASTRA_WARNING_DISTANCE  20.0
#define ASTRA_CRITICAL_DISTANCE 10.0

/* Euclidean distance between two 2D points. */
double calculateDistance(double x1, double y1, double x2, double y2);

/* Returns "CRITICAL", "WARNING" or "SAFE" (static string, do not free). */
const char *getRiskLevel(double distance);

#ifdef __cplusplus
}
#endif

#endif
