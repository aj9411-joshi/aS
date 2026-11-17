#include "routing.h"

#define INF 999999999.0

/* Set every connection cost to infinity.
   A cost of infinity means there is no direct connection. */
void initializeRouteGraph(RouteGraph *graph, int nodes)
{
    int i, j;

    if (nodes < 0 || nodes > MAX_NODES)
        nodes = 0;

    graph->nodeCount = nodes;

    for (i = 0; i < nodes; i++) {
        for (j = 0; j < nodes; j++) {
            graph->cost[i][j] = INF;
        }
    }
}

/* Add a pathway in both directions because the graph is undirected. */
int addPathway(RouteGraph *graph, int a, int b,
               double distance, double risk, double fuel)
{
    double totalCost;

    if (a < 0 || b < 0 ||
        a >= graph->nodeCount || b >= graph->nodeCount)
        return 0;

    /* Combine distance, risk and fuel into one cost. */
    totalCost = distance * (1 + risk) + fuel;

    graph->cost[a][b] = totalCost;
    graph->cost[b][a] = totalCost;

    return 1;
}

/* Find the unvisited node having the smallest known cost. */
static int findMinimumNode(double cost[], int visited[], int nodes)
{
    int i;
    int minimumNode = -1;
    double minimumCost = INF;

    for (i = 0; i < nodes; i++) {
        if (visited[i] == 0 && cost[i] < minimumCost) {
            minimumCost = cost[i];
            minimumNode = i;
        }
    }

    return minimumNode;
}

/* Dijkstra's algorithm finds the minimum-cost route. */
RouteResult runDijkstra(RouteGraph *graph, int start, int target)
{
    RouteResult result = {0, 0.0, {0}, 0};

    double cost[MAX_NODES];
    int parent[MAX_NODES];
    int visited[MAX_NODES] = {0};

    int i, j, current;
    int reversePath[MAX_NODES];
    int pathLength = 0;

    if (start < 0 || target < 0 ||
        start >= graph->nodeCount || target >= graph->nodeCount)
        return result;

    /* Initially, all nodes are unreachable. */
    for (i = 0; i < graph->nodeCount; i++) {
        cost[i] = INF;
        parent[i] = -1;
    }

    /* Starting node has cost zero. */
    cost[start] = 0;

    /* Continue until all reachable nodes have been checked. */
    for (i = 0; i < graph->nodeCount; i++) {

        current = findMinimumNode(cost, visited, graph->nodeCount);

        if (current == -1)
            break;

        visited[current] = 1;

        /* Check every possible neighbour of the current node. */
        for (j = 0; j < graph->nodeCount; j++) {

            if (graph->cost[current][j] != INF &&
                visited[j] == 0) {

                double newCost = cost[current] + graph->cost[current][j];

                /* Update the route if a cheaper path is found. */
                if (newCost < cost[j]) {
                    cost[j] = newCost;
                    parent[j] = current;
                }
            }
        }
    }

    /* If target still has infinite cost, no route exists. */
    if (cost[target] == INF)
        return result;

    /* Reconstruct the path by following parent nodes backwards. */
    current = target;

    while (current != -1 && pathLength < MAX_NODES) {
        reversePath[pathLength] = current;
        pathLength++;
        current = parent[current];
    }

    /* Store the path in start-to-target order. */
    for (i = 0; i < pathLength; i++) {
        result.path[i] = reversePath[pathLength - 1 - i];
    }

    result.pathLength = pathLength;
    result.totalCost = cost[target];
    result.reachable = 1;

    return result;
}

/* Create the same sample graph as the original code. */
void buildSampleGraph(RouteGraph *graph)
{
    initializeRouteGraph(graph, 5);

    addPathway(graph, 0, 1, 10.0, 0.1, 2.0);
    addPathway(graph, 0, 2,  5.0, 1.5, 1.0);
    addPathway(graph, 1, 3,  8.0, 0.2, 1.5);
    addPathway(graph, 2, 3,  2.0, 1.0, 0.5);
    addPathway(graph, 3, 4,  7.0, 0.0, 1.0);
}
