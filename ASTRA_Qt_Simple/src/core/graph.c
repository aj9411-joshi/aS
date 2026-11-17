#include <stdio.h>
#include <string.h>
#include "graph.h"

static int validNode(const Graph *g, int n) {
    return n >= 0 && n < g->nodeCount;
}

void initializeGraph(Graph *graph, int nodes) {
    memset(graph, 0, sizeof(*graph));
    if (nodes < 0 || nodes > MAX_NODES) {
        printf("Invalid number of nodes.\n");
        graph->nodeCount = 0;
        return;
    }
    graph->nodeCount = nodes;
}

void addNode(Graph *graph) {
    if (graph->nodeCount >= MAX_NODES) {
        printf("Maximum node capacity reached.\n");
        return;
    }
    graph->nodeCount++;
}

void addEdge(Graph *graph, int source, int destination, double weight) {
    if (!validNode(graph, source) || !validNode(graph, destination)) {
        printf("Invalid source or destination node.\n");
        return;
    }
    graph->adjacencyMatrix[source][destination] = 1;
    graph->weights[source][destination] = weight;
}

void addUndirectedEdge(Graph *graph, int a, int b, double weight) {
    addEdge(graph, a, b, weight);
    addEdge(graph, b, a, weight);
}

int hasEdge(const Graph *graph, int source, int destination) {
    if (!validNode(graph, source) || !validNode(graph, destination)) return 0;
    return graph->adjacencyMatrix[source][destination];
}

int graphLoadFromFile(Graph *graph, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[256];
    int haveNodes = 0;
    initializeGraph(graph, 0);

    while (fgets(line, sizeof line, f)) {
        int a, b, n;
        double w;
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (sscanf(line, "NODES %d", &n) == 1) {
            if (n < 0 || n > MAX_NODES) { fclose(f); return 0; }
            initializeGraph(graph, n);
            haveNodes = 1;
        } else if (sscanf(line, "EDGE %d %d %lf", &a, &b, &w) == 3) {
            if (!haveNodes || !validNode(graph, a) || !validNode(graph, b)) {
                fclose(f);
                return 0;
            }
            addUndirectedEdge(graph, a, b, w);
        }
    }
    fclose(f);
    return haveNodes;
}

int graphBFSOrder(const Graph *graph, int start, int *order) {
    if (!validNode(graph, start)) return -1;
    int visited[MAX_NODES] = {0};
    int queue[MAX_NODES];
    int front = 0, rear = 0, count = 0;

    visited[start] = 1;
    queue[rear++] = start;
    while (front < rear) {
        int cur = queue[front++];
        order[count++] = cur;
        for (int i = 0; i < graph->nodeCount; i++) {
            if (graph->adjacencyMatrix[cur][i] && !visited[i]) {
                visited[i] = 1;
                queue[rear++] = i;
            }
        }
    }
    return count;
}

static void dfsUtil(const Graph *g, int cur, int *visited, int *order, int *count) {
    visited[cur] = 1;
    order[(*count)++] = cur;
    for (int i = 0; i < g->nodeCount; i++) {
        if (g->adjacencyMatrix[cur][i] && !visited[i])
            dfsUtil(g, i, visited, order, count);
    }
}

int graphDFSOrder(const Graph *graph, int start, int *order) {
    if (!validNode(graph, start)) return -1;
    int visited[MAX_NODES] = {0};
    int count = 0;
    dfsUtil(graph, start, visited, order, &count);
    return count;
}

int graphFindPath(const Graph *graph, int src, int dst, int *path) {
    if (!validNode(graph, src) || !validNode(graph, dst)) return 0;

    int visited[MAX_NODES] = {0};
    int parent[MAX_NODES];
    int queue[MAX_NODES];
    int front = 0, rear = 0;

    for (int i = 0; i < MAX_NODES; i++) parent[i] = -1;
    visited[src] = 1;
    queue[rear++] = src;

    while (front < rear) {
        int cur = queue[front++];
        if (cur == dst) {
            int tmp[MAX_NODES], len = 0;
            for (int n = dst; n != -1; n = parent[n]) tmp[len++] = n;
            for (int i = 0; i < len; i++) path[i] = tmp[len - 1 - i];
            return len;
        }
        for (int i = 0; i < graph->nodeCount; i++) {
            if (graph->adjacencyMatrix[cur][i] && !visited[i]) {
                visited[i] = 1;
                parent[i] = cur;
                queue[rear++] = i;
            }
        }
    }
    return 0;
}

double graphPathWeight(const Graph *graph, const int *path, int length) {
    double total = 0.0;
    for (int i = 0; i + 1 < length; i++)
        total += graph->weights[path[i]][path[i + 1]];
    return total;
}

void displayGraph(const Graph *graph) {
    printf("\nAstra Space Network\n");
    printf("Number of nodes: %d\n", graph->nodeCount);
    for (int i = 0; i < graph->nodeCount; i++) {
        printf("Node %d: ", i);
        for (int j = 0; j < graph->nodeCount; j++) {
            if (graph->adjacencyMatrix[i][j])
                printf("-> Node %d (weight: %.2f)  ", j, graph->weights[i][j]);
        }
        printf("\n");
    }
}

void BFS(const Graph *graph, int startNode) {
    int order[MAX_NODES];
    int n = graphBFSOrder(graph, startNode, order);
    if (n < 0) { printf("Invalid starting node.\n"); return; }
    printf("\nBFS Traversal: ");
    for (int i = 0; i < n; i++) printf("%d ", order[i]);
    printf("\n");
}

void DFS(const Graph *graph, int startNode) {
    int order[MAX_NODES];
    int n = graphDFSOrder(graph, startNode, order);
    if (n < 0) { printf("Invalid starting node.\n"); return; }
    printf("\nDFS Traversal: ");
    for (int i = 0; i < n; i++) printf("%d ", order[i]);
    printf("\n");
}

int findRoute(const Graph *graph, int source, int destination) {
    int path[MAX_NODES];
    int len = graphFindPath(graph, source, destination, path);
    if (len == 0) {
        printf("No route exists between Node %d and Node %d.\n", source, destination);
        return 0;
    }
    printf("Route found: ");
    for (int i = 0; i < len; i++) printf(i ? "->%d" : "%d", path[i]);
    printf("\n");
    return 1;
}
