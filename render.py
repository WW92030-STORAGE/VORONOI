import PIL
from PIL import Image, ImageDraw

INPUT = "voronoi.out"

# The input format is the same as generateDiagram() except with a single number at the top before everything else, representing the image size.

def run(WIDTH = 4, R = 16, POINTS = 0xFFFFFF, DELAUNAY = 0x0000FF, VORONOI = 0xFFFF00, CIRCUMS = None):
	with open(INPUT, 'r') as F:
		B = int(F.readline())
		image = Image.new('RGB', (B, B))
		draw = ImageDraw.Draw(image)

		p = []
		tris = []
		cc = []
		ve = []
		infs = []
		N = int(F.readline())
		for i in range(N):
			a, b = [float(x) for x in F.readline().split()]
			p.append((a, b))
		
		M = int(F.readline())
		for i in range(M):
			a, b, c = [int(x) for x in F.readline().split()]
			tris.append([p[a], p[b], p[c]])

		for i in range(M):
			a, b = [float(x) for x in F.readline().split()]
			cc.append((a, b))
		
		E = int(F.readline())
		for i in range(E):
			i1, i2 = [int(x) for x in F.readline().split()]
			ve.append((i1, i2))
		
		X = int(F.readline())
		for i in range(X):
			sp = F.readline().split()
			infs.append((int(sp[0]), float(sp[1]), float(sp[2])))

		# draw stuff

		for tri in tris:
			draw.polygon(tri, outline = DELAUNAY, width=WIDTH)

		for (i1, i2) in ve:
			draw.line((cc[i1][0], cc[i1][1], cc[i2][0], cc[i2][1]), fill=VORONOI, width = 2 * WIDTH)

		# draw lines to infinity
		for pack in infs:
			t, a, b = pack
			scale = B
			if a > 0:
				scale = B / a
			if b > 0:
				scale = max(scale, B / b)
			scale = 2 + max(scale, 1)
			a *= scale
			b *= scale
			cen = cc[t]
			draw.line((cen[0], cen[1], cen[0] + a, cen[1] + b), fill = VORONOI, width = 2*WIDTH)

		# draw points

		if CIRCUMS is not None:
			for (a, b) in cc:
				draw.ellipse((a - R, b - R, a + R, b + R), fill = CIRCUMS)

		for (a, b) in p:
			draw.ellipse((a - R, b - R, a + R, b + R), fill = POINTS)
		
		
		
		image.save('voronoi.png')

if __name__ == "__main__":
	run()