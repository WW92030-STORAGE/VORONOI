#include <set>
#include <iostream>
#include "voronoi.h"
#include <chrono>

double rand01() {
	return (double)(rand()) / (double)(RAND_MAX);
}

const bool PTIME = true;

int main() {
	auto start = std::chrono::high_resolution_clock::now();
	srand(0);
	std::set<point> pp;
	std::vector<point> p;
	int B = 4096;
	int M = -256;
	for (int i = 0; i < 8192; i++) pp.insert({M + rand01() * (B - 2 * M), M + rand01() * (B - 2 * M)});
	for (auto i : pp) p.push_back(i);

	// p = std::vector<point>({{1200, 1200}, {400, 1200}, {400, 400}, {400, 800}, {800, 800}, {800, 1200}, {1200, 800}, {800, 400}, {1200, 400}});
	std::cout << B << "\n"; // This line is used for the python renderer to set the image size

	std::string s = generateDiagram(p);
	std::cout << s << "\n";


	if (PTIME) {
		// BEGIN TIME PRINTOUT
		auto end = std::chrono::high_resolution_clock::now();
  		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  		int64_t count = duration.count();
		std::cout << "Time taken: " << count * 0.001 << " ms" << std::endl;
  		// END TIME PRINTOUT
	}

	return 0;
}