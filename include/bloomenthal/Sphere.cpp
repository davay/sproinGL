// Sphere.cpp - make unit sphere

#include "Sphere.h"
#define _PI 3.141592f

class PtUv {
public:
	vec3 pt;
	vec2 uv;
	PtUv() { }
	PtUv(vec3 p, vec2 t) : pt(p), uv(t) { }
};

void UnitSphere(int res, vector<SphereVertex> *vertices) {
	// to avoid missing/degenerate triangles at the poles,
	// each pole is represented as a circle with u varying (0,1)
	class Helper {
	public:
		int res, ptCount, vrtCount;
		vector<SphereVertex> *vrts;
		vector<PtUv> ptuvs;
		void AddTriangle(int i1, int i2, int i3) {
			for (int k = 0; k < 3; k++) {
				PtUv pu = ptuvs[k>1? i3 : k? i2 : i1];
				// printf("vrtCount = %i\n", vrtCount);
				(*vrts)[vrtCount++] = SphereVertex(pu.pt, pu.uv);
			}
		}
		vec3 PtFromUV(float u, float v) {
			float elevation = _PI/2-v*_PI; // elevation PI/2 = N. pole, 0 = equator, -PI/2 = S. pole
			float eFactor = cos(elevation);
			float y = sin(elevation);
			float angle = 2*_PI*u;
			float x = eFactor*cos(angle), z = eFactor*sin(angle);
			return vec3(x, y, z);
		}
		Helper(int res, vector<SphereVertex> *vrts) : res(res), vrts(vrts) {
			ptCount = vrtCount = 0;
			// res is # latitudinal circles north to south, excluding poles, including equator if res odd
			int nCircles = res, nCirclePoints = 2*res+2;
				// number of latitudinal, non-pole, circles
			int nPtUvs = (nCirclePoints+1)*(nCircles+2);
				// allocate for nCircles plus circle for each pole
				// within each latitudinal circle the first point (u=0) is repeated at the end (u=1)
			int nTriangles = 2*(nCircles+1)*nCirclePoints;
			int nVertices = 3*nTriangles;
			ptuvs.resize(nPtUvs);
			vrts->resize(nVertices);
			// points for circles
			for (int i = 0; i <= nCircles+1; i++) {
				float v = (float) i / (nCircles+1);
				for (int j = 0; j <= nCirclePoints; j++) {
					float u = (float) j / nCirclePoints; // 0 through 1
					ptuvs[ptCount++] = PtUv(PtFromUV(u, v), vec2(u, v));
				}
			}
			// between circles
			for (int k = 0; k <= nCircles; k++) {
				int vidCircle1 = k*(nCirclePoints+1);
				int vidCircle2 = vidCircle1+nCirclePoints+1;
				for (int j = 0; j < nCirclePoints; j++) {
					int vid1A = vidCircle1+j, vid1B = vid1A+1;
					int vid2A = vidCircle2+j, vid2B = vid2A+1;
					AddTriangle(vid1A, vid1B, vid2B);
					AddTriangle(vid1A, vid2B, vid2A);
				}
			}
		}
	} h(res, vertices);
}

void UnitSphere(int res, vector<vec3> &points, vector<vec2> &uvs, vector<int3> &triangles) {
	// res is # latitudinal circles north to south, excluding poles, including equator if res odd
	int tCount = 0, vCount = 0;
	int nCircles = res, nCirclePoints = 2*res+2, nVertices = 2+nCirclePoints*nCircles;
	points.resize(nVertices);
	uvs.resize(nVertices);
	triangles.resize(nCircles*2*nCirclePoints);
	// points for unit sphere, start at north pole, then circles, then south pole
	uvs[vCount] = vec2(0, 0);
	points[vCount++] = vec3(0, 1, 0);
	float dAngle = _PI/(res+1);
	for (int i = 0; i < res; i++) {
		float v = (float) i / res;
		float elevation = (_PI/2)-(i+1)*dAngle; // elevation PI/2 = N. pole, 0 = equator, -PI/2 = S. pole
		float eFactor = cos(elevation);
		float y = sin(elevation);
		for (int j = 0; j < nCirclePoints; j++) {
			float u = (float) j / nCirclePoints;
			float angle = 2*_PI*u;
			float x = eFactor*cos(angle), z = eFactor*sin(angle);
			uvs[vCount] = vec2(u, v);
			points[vCount++] = vec3(x, y, z);
		}
	}
	uvs[vCount] = vec2(0, 1);
	points[vCount++] = vec3(0, -1, 0);
	// triangles: north pole to first circle
	for (int i = 0; i < nCirclePoints; i++)
		triangles[tCount++] = int3(0, i+1, (i+1)%nCirclePoints+1);
	// between circles
	for (int k = 1; k < nCircles; k++) {
		int vidCircle1 = 1+(k-1)*nCirclePoints, vidCircle2 = 1+k*nCirclePoints;
		for (int j = 0; j < nCirclePoints; j++) {
			int jj = (j+1)%nCirclePoints;
			int vid1A = vidCircle1+j, vid1B = vidCircle1+jj;
			int vid2A = vidCircle2+j, vid2B = vidCircle2+jj;
			triangles[tCount++] = int3(vid1A, vid1B, vid2B);
			triangles[tCount++] = int3(vid1A, vid2B, vid2A);
		}
		if (k == nCircles-1)
			// last circle to south pole
			for (int i = 0; i < nCirclePoints; i++)
				triangles[tCount++] = int3(nVertices-1, i+vidCircle2, (i+1)%nCirclePoints+vidCircle2);
	}
}
