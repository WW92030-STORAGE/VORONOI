#include <set>
#include <iostream>
#include "voronoi.h"

int main() {
	srand(0);
	std::set<point> pp;
	std::vector<point> p;
	int B = 4096;
	int M = 256;

	for (int i = 0; i < 64; i++) pp.insert({M + rand() % (B - 2 * M), M + rand() % (B - 2 * M)});
	for (auto i : pp) p.push_back(i);

	// p = std::vector<point>({{0, 0}, {4, 0}, {4, 4}});

	std::cout << B << "\n";

	std::string s = generateDiagram(p);
	std::cout << s << "\n";

	return 0;
}