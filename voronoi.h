#include <vector>
#include <algorithm>
#include <climits>
#include <cfloat>
#include <queue>
#include <string>
#include <map>
#include <utility>
#include <cmath>

typedef double numeric; // any numeric type
typedef std::pair<numeric, numeric> point;
typedef std::pair<int, int> indexPair;

constexpr point INF = {FLT_MAX, FLT_MAX};

// 3x3 minor matrix det
inline numeric det4min(std::vector<std::vector<numeric>>& mat, int col_skipped) {
	int indices[6] = {0, 1, 2};
	for (int i = 0; i < 3; i++) {
		if (indices[i] >= col_skipped) indices[i]++;
	}
	for (int i = 0; i < 3; i++) indices[i + 3] = indices[i];

	numeric res = 0;
	for (int i = 0; i < 3; i++) {
		int i0 = indices[i];
		int i1 = indices[i + 1];
		int i2 = indices[i + 2];
		res += mat[1][i0] * mat[2][i1] * mat[3][i2] - mat[1][i2] * mat[2][i1] * mat[3][i0];
	}
	return res;
}

inline numeric det4(std::vector<std::vector<numeric>>& mat) {
	return mat[0][0] * det4min(mat, 0) - mat[0][1] * det4min(mat, 1) + mat[0][2] * det4min(mat, 2) - mat[0][3] * det4min(mat, 3);
}

// is d in circle(a, b, c)?
inline bool inCircle(point a, point b, point c, point d) {
	std::vector<std::vector<numeric>> mat(4);
	mat[0] = std::vector<numeric>({a.first, a.second, a.first * a.first + a.second * a.second, 1});
	mat[1] = std::vector<numeric>({b.first, b.second, b.first * b.first + b.second * b.second, 1});
	mat[2] = std::vector<numeric>({c.first, c.second, c.first * c.first + c.second * c.second, 1});
	mat[3] = std::vector<numeric>({d.first, d.second, d.first * d.first + d.second * d.second, 1});
	return det4(mat) > 0;
}

// positive if a, b, c are in counterclockwise order, negative if clockwise, 0 if collinear
inline int ccw(point a, point b, point c) {
	numeric x = (b.second - a.second) * (c.first - b.first) - (b.first - a.first) * (c.second - b.second);
	if (x > 0) return -1;
	if (x < 0) return 1;
	return 0;
}

inline std::string p2str(point p) {
	return "(" + std::to_string(p.first) + ", " + std::to_string(p.second) + ")";
}

inline point sort(point p) {
	if (p.first > p.second) return {p.second, p.first};
	return p;
}

// quadedge. A quad edge stores information about itself, the dual edge 90 degrees counterclockwise from it (rot), and the first edge counterclockwise of it and having the same origin (onext).
// Each edge has two adjoining faces, one on the left and one on the right.
/*

Terminology:
origin = the start point of this edge
sym = the reverse edge = rot->rot
onext = next counterclockwise from same origin (can be same edge)
oprev = next clockwise from same origin
lnext = next edge on left face (can be sym)
rprev = next edge on right face (can be sym, called prev cuz this edge and rprev points clockwise on the right face)
dest = sym->origin

*/

struct quadEdge {
	point origin;
	int originIndex = -1;
	quadEdge* rot = 0;
	quadEdge* onext = 0;
	bool vis = false;

	inline quadEdge* sym() { return rot->rot; }
	inline quadEdge* lnext() { return rot->sym()->onext->rot; }
	inline quadEdge* oprev() { return rot->onext->rot; }
	inline quadEdge* rprev() { return sym()->onext; }
	inline point dest() { return sym()->origin; }
	inline int destIndex() { return sym()->originIndex; }
	inline indexPair indices() {
		indexPair p = {originIndex, destIndex()};
		return sort(p);
	}

	std::string to_string(bool recursive = true) {
		std::string res = "quadEdge[" + std::to_string(originIndex) + ", " + std::to_string(destIndex()) + "]";
		if (!recursive) return res;
		if (onext) res += "onext=" + onext->to_string(false);
		if (sym()) res += "sym=" + sym()->to_string(false);
		if (lnext()) res += "lnext=" + lnext()->to_string(false);
		if (oprev()) res += "oprev=" + oprev()->to_string(false);
		return res;
	}

	quadEdge() {}
	quadEdge(point s, point d, int si = 0, int di = 0) {
		quadEdge* rot_ = new quadEdge;
		quadEdge* sym = new quadEdge;
		quadEdge* rrot = new quadEdge;

		origin = s;
		originIndex = si;
		sym->origin = d;
		sym->originIndex = di;
		rot_->origin = rrot->origin = INF;

		rot = rot_;
		sym->rot = rrot;
		rot_->rot = sym;
		rrot->rot = this;
		onext = this;
		rot_->onext = rrot;
		sym->onext = sym;
		rrot->onext = rot_;
	}
};

// somehow this does a lot of different things depending on the relationship between the two edges
inline void splice(quadEdge* a, quadEdge* b) {
	std::swap(a->onext->rot->onext, b->onext->rot->onext);
	std::swap(a->onext, b->onext);
}

// remove edge, closing any gaps if needed
inline void delEdge(quadEdge* e) {
	splice(e, e->oprev());
	splice(e->sym(), e->sym()->oprev());

	// destroy the edge's group of 4 quadEdges
	delete e->sym()->rot;
	delete e->sym();
	delete e->rot;
	delete e;
}

// connect two quadEdges together
inline quadEdge* connect(quadEdge* a, quadEdge* b) {
	quadEdge* e = new quadEdge(a->dest(), b->origin, a->destIndex(), b->originIndex);
	splice(e, a->lnext());
	splice(e->sym(), b);
	return e;
}

inline bool rightOf(point x, quadEdge* e) {
	return ccw(x, e->dest(), e->origin) > 0;
}

inline bool leftOf(point x, quadEdge* e) {
	return ccw(x, e->origin, e->dest()) > 0;
}

std::pair<quadEdge*, quadEdge*> triangulateUtil(std::vector<point>& points, int L = 0, int H = -1) {
	int n = points.size();
	if (L < 0) L = 0;
	if (H >= n || H < 0) H = n - 1;
	if (H == L + 1) {
		quadEdge* res = new quadEdge(points[L], points[H], L, H);
		return {res, res->sym()};
	}
	if (H == L + 2) {
		quadEdge* a = new quadEdge(points[L], points[L + 1], L, L + 1);
		quadEdge* b = new quadEdge(points[L + 1], points[H], L + 1, H);
		splice(a->sym(), b);
		int status = ccw(points[L], points[L + 1], points[H]);
		if (status > 0) {
			connect(b, a);
			return {a, b->sym()};
		}
		if (status < 0) {
			quadEdge* c = connect(b, a);
			return {c->sym(), c};
		}
		return {a, b->sym()};
	}

	int mid = L + ((H - L)>>1);
	std::pair<quadEdge*, quadEdge*> LL = triangulateUtil(points, L, mid);
	std::pair<quadEdge*, quadEdge*> RR = triangulateUtil(points, mid + 1, H);
	quadEdge* ldo = LL.first;
	quadEdge* ldi = LL.second;
	quadEdge* rdi = RR.first;
	quadEdge* rdo = RR.second;

	// screw phase: find lowest common tangent of the two regions by "screwing" the DI edges downward
	while (true) {
		if (leftOf(rdi->origin, ldi)) ldi = ldi->lnext();
		else if (rightOf(ldi->origin, rdi)) rdi = rdi->rprev();
		else break;
	}

	// connect an edge from the right region to the left and adjust the returned edges if needed
	quadEdge* basel = connect(rdi->sym(), ldi);
	if (ldi->origin == ldo->origin) ldo = basel->sym();
	if (rdi->origin == rdo->origin) rdo = basel;

	// merge phase
	while (true) {
		quadEdge* lcand = basel->rprev();
		if (rightOf(lcand->dest(), basel)) {
			// find a good edge with same origin as lcand
			while (inCircle(basel->dest(), basel->origin, lcand->dest(), lcand->onext->dest())) {
				quadEdge* t = lcand->onext;
				delEdge(lcand);
				lcand = t;
			}
		}
		quadEdge* rcand = basel->oprev();
		if (rightOf(rcand->dest(), basel)) {
			while (inCircle(basel->dest(), basel->origin, rcand->dest(), rcand->oprev()->dest())) {
				quadEdge* t = rcand->oprev();
				delEdge(rcand);
				rcand = t;
			}
		}
		// reached the end
		if (!rightOf(rcand->dest(), basel) && !(rightOf(lcand->dest(), basel))) break;

		// otherwise find a new base edge
		if (!rightOf(lcand->dest(), basel) || (rightOf(rcand->dest(), basel) && inCircle(lcand->dest(), lcand->origin, rcand->origin, rcand->dest()))) {
			basel = connect(rcand, basel->sym());
		}
		else basel = connect(basel->sym(), lcand->sym());
	}
	return {ldo, rdo};
}

std::pair<std::vector<std::vector<int>>, std::vector<indexPair>> triangulate(std::vector<point>& points) {
	std::sort(points.begin(), points.end());
	std::vector<std::vector<int>> res;
	std::vector<indexPair> finalEdges;
	if (points.size() < 2) return {res, finalEdges};

	std::pair<quadEdge*, quadEdge*> qe = triangulateUtil(points);
	quadEdge* e = qe.first;
	std::queue<quadEdge*> q;
	q.push(e);

	std::map<indexPair, quadEdge*> usedEdges;
	while (ccw(e->onext->dest(), e->dest(), e->origin) < 0) e = e->onext;

	quadEdge* ee = e;
	while (true) {
		ee->vis = true;
		usedEdges[ee->indices()] = ee;
		q.push(ee->sym());
		ee = ee->lnext();
		if (ee == e) break;
	}
	std::vector<int> p;
	while (q.size()) {
		quadEdge* e = q.front();
		q.pop();
		if (e->vis) continue;
		quadEdge* ee = e;
		while (true) {
			ee->vis = true;
			usedEdges[ee->indices()] = ee;
			p.push_back(ee->originIndex);
			q.push(ee->sym());
			ee = ee->lnext();
			if (ee == e) break;
		}
	}

	for (int i = 0; i < p.size(); i += 3) res.push_back(std::vector<int>({p[i], p[i + 1], p[i + 2]}));

	for (auto x : usedEdges) {
		finalEdges.push_back(x.first);
		delEdge(x.second);
	}

	return {res, finalEdges};
}

std::pair<double, double> circumcenter(point a, point b, point c) {
	numeric arsq = a.first * a.first + a.second * a.second;
	numeric brsq = b.first * b.first + b.second * b.second;
	numeric crsq = c.first * c.first + c.second * c.second;

	point bc = {b.first - c.first, b.second - c.second};
	point ca = {c.first - a.first, c.second - a.second};
	point ab = {a.first - b.first, a.second - b.second};
	double D = 2 * (a.first * bc.second + b.first * ca.second + c.first * ab.second);
	if (D == 0) return {(a.first + b.first + c.first) / 3.0, (a.second + b.second + c.second) / 3.0};
	float invD = 1.0 / D;
	return {invD * (arsq * bc.second + brsq * ca.second + crsq * ab.second), -invD * (arsq * bc.first + brsq * ca.first + crsq * ab.first)};
}

std::pair<std::vector<indexPair>, std::vector<std::pair<int, indexPair>>> voronoi(std::vector<std::vector<int>> tri) {
	std::map<indexPair, std::vector<int>> edgemap;
	for (int i = 0; i < tri.size(); i++) {
		auto t = tri[i];
		for (int j = 0; j < 3; j++) {
			auto p = sort({t[j], t[(j + 1) % 3]});
			if (edgemap.find(p) == edgemap.end()) edgemap[p] = std::vector<int>();
			edgemap[p].push_back(i);
		}
	}
	std::vector<indexPair> res;
	std::vector<std::pair<int, indexPair>> infs;
	for (auto i : edgemap) {
		if (i.second.size() < 1) continue;
		else if (i.second.size() == 1) {
			infs.push_back({i.second[0], i.first});
		}
		else res.push_back({i.second[0], i.second[1]});
	}

	
	return {res, infs};
}

std::string generateDiagram(std::vector<point> p, bool doV = true) {
	std::string ret = "";
	auto res = triangulate(p);
	auto v = res.first;
	auto e = res.second;

	ret += std::to_string(p.size()) + "\n";

	for (auto i : p) ret += std::to_string(i.first) + " " + std::to_string(i.second) + "\n";
	ret += std::to_string(v.size()) + "\n";

	for (auto i : v) {
		for (auto j : i) ret += std::to_string(j) + " ";
		ret.push_back('\n');
	}
	std::vector<point> circums;
	for (auto i : v) {
		auto cc = circumcenter(p[i[0]], p[i[1]], p[i[2]]);
		ret += std::to_string(cc.first) + " " + std::to_string(cc.second) + "\n";
		circums.push_back(cc);
	}
	if (!doV) return ret;

	auto vor = voronoi(v);
	auto interior = vor.first;
	auto inf = vor.second;
	ret += std::to_string(interior.size()) + "\n";
	for (auto i : interior) ret += std::to_string(i.first) + " " + std::to_string(i.second) + "\n";
	ret += std::to_string(inf.size()) + "\n";
	for (auto i : inf) {
		point location = circums[i.first];
		point a = p[i.second.first];
		point b = p[i.second.second];
		int ci = v[i.first][0];
		for (int j = 0; j < 3; j++) {
			if (v[i.first][j] != i.second.first && v[i.first][j] != i.second.second) ci = v[i.first][j];
		}
		point c = p[ci];

		point dir = {b.first - a.first, b.second - a.second};
		numeric norm = sqrt(dir.first * dir.first + dir.second * dir.second);
		if (norm > 0) {
			numeric inv = 1.0 / norm;
			dir.first *= inv;
			dir.second *= inv;
		}
		if (ccw(a, b, c) < 0) dir = {-dir.second, dir.first};
		else dir = {dir.second, -dir.first};

		ret += std::to_string(i.first) + " " + std::to_string(dir.first) + " " + std::to_string(dir.second) + "\n";
	}
	return ret;
}