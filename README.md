# VORONOI

Delaunay Triangulations and Voronoi diagrams. Based on https://dl.acm.org/doi/10.1145/282918.282923 and https://cp-algorithms.com/geometry/delaunay.html but with no memory leaks and with a Delaunay to Voronoi conversion scheme that automatically handles edges to infinity.

Everything runs in O(n log n).

# USAGE

```
std::vector<point> p({{0, 0}, {4, 0}, {4, 4}, {4, 8}, {8, 8}, {8, 0}, {0, 8}, {8, 4}, {0, 4}});
auto res = triangulate(p); // generates a Delaunay triangulation, containing information on both triangles and edges.
std::string s = generateDiagram(p, true); // generates a formatted string of the Voronoi diagram of the system
```