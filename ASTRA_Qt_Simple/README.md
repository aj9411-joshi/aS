# ASTRA - Simple Qt Widgets UI

A small, single-window Qt 6 interface for demonstrating ASTRA.

## Included
- 10 sample space objects (8 satellites and 2 debris objects)
- 2D object visualization and catalogue table
- Collision screening using the existing `collision.c` module
- Space-network creation using the existing `graph.c` module
- BFS, DFS and fewest-hops route search using the graph module
- Weighted safest-route search (Dijkstra) using the `routing.c` module

## Open in Qt Creator
1. Extract this ZIP.
2. Open Qt Creator.
3. Choose **File > Open File or Project**.
4. Select this folder's `CMakeLists.txt`.
5. Select a Qt 6 Desktop kit and click Configure.
6. Select the `ASTRA_GUI` target and press Run.

## How to demonstrate
1. **Generate Demo Data** - loads ten sample objects.
2. **Detect Collision** - checks every unique pair using Euclidean distance.
   - SAFE: more than 20 units
   - WARNING: 10 to 20 units
   - CRITICAL: 10 units or less
3. **Build Space Network** - each object is a graph node; objects within 100 units are connected.
4. **Run BFS / Run DFS** - shows the traversal order from SAT-01.
5. **Find Route** - finds a route between the selected objects using the graph path function.
6. **Safest Route** - runs Dijkstra on the same network. Each link costs `distance x (1 + risk) + fuel`;
   risk is 1.0 for links touching debris, 0.2 near debris, otherwise 0; fuel = 5% of distance.

## Important project scope
The coordinates are synthetic demo units, not real orbital coordinates. The drawing is illustrative.
Find Route uses breadth-first search (fewest edges); Safest Route uses Dijkstra with the demo cost model above.
Neither is a scientific orbital maneuver optimizer. This UI is intentionally simple for a clear faculty demonstration.
