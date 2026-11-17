#ifndef ASTRA_GRAPH_H
#define ASTRA_GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NODES 100

typedef struct {
    int nodeCount;
    int adjacencyMatrix[MAX_NODES][MAX_NODES];
    double weights[MAX_NODES][MAX_NODES];
} Graph;

void initializeGraph(Graph *graph, int nodes);
void addUndirectedEdge(Graph *graph, int a, int b, double weight);
int graphBFSOrder(const Graph *graph, int start, int *order);
int graphDFSOrder(const Graph *graph, int start, int *order);
int graphFindPath(const Graph *graph, int source, int destination, int *path);
double graphPathWeight(const Graph *graph, const int *path, int length);

#ifdef __cplusplus
}
#endif
#endif
