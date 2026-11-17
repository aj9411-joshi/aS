#ifndef ASTRA_ROUTING_H
#define ASTRA_ROUTING_H

#include "graph.h"   /* provides MAX_NODES (shared with the BFS/DFS graph module) */

#ifdef __cplusplus
extern "C" {
#endif

/* Stores the weighted graph: cost between every pair of nodes. */
typedef struct {
    int nodeCount;
    double cost[MAX_NODES][MAX_NODES];
} RouteGraph;

/* Stores the shortest path found by Dijkstra's algorithm. */
typedef struct {
    int reachable;
    double totalCost;
    int path[MAX_NODES];
    int pathLength;
} RouteResult;

/* Renamed from initializeGraph to avoid clashing with graph.h. */
void initializeRouteGraph(RouteGraph *graph, int nodes);

/* Cost of a pathway = distance * (1 + risk) + fuel. Returns 1 on success, 0 on bad node index. */
int addPathway(RouteGraph *graph, int a, int b,
               double distance, double risk, double fuel);

RouteResult runDijkstra(RouteGraph *graph, int start, int target);

void buildSampleGraph(RouteGraph *graph);

#ifdef __cplusplus
}
#endif

#endif
