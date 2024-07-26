#ifndef UTILITY_H
#define UTILITY_H

float myasin(float x);
float myacos(float x);
float mysqrt(float x);
float mylog(float x);

#include <QDebug>
#include "vec3.h"
#include <vector>
using namespace std;

class myfloat3
{
public:
    explicit myfloat3(); 
    explicit myfloat3(float x, float y, float z);
    const float norm2() const;
    const float norm() const;
    float dot(myfloat3 another);
    myfloat3 cross(myfloat3 another);

    const myfloat3 operator +(const myfloat3 another) const;
    const myfloat3 operator -(const myfloat3 another) const;

    const myfloat3 operator *(float x) const;
    const myfloat3 operator /(float x) const;

    float getX() const;
    float getY() const;
    float getZ() const;

    friend float dot(myfloat3 x, myfloat3 y);
    friend myfloat3 cross(myfloat3 x, myfloat3 y);

private:
    float x, y, z;
};

class Triangle {
public:
	vec3 p, q, r;

	Triangle() {}
	Triangle(vec3 v1_, vec3 v2_, vec3 v3_) {
		p = v1_;
		q = v2_;
		r = v3_;
	}
};

class int3 {
public:
	int x, y, z;
	int3(int x_ = 0, int y_ = 0, int z_ = 0) {
		x = x_;
		y = y_;
		z = z_;
	}
	int operator[] (int idx) {
		if (idx == 0)
			return x;
		else if (idx == 1)
			return y;
		else if (idx == 2)
			return z;
	}
};

class Polyhedron {
public:
    vector<int3> faces;
    vector<vec3> vertices;
    vector<vec3> current_vertices;
	vector<vec3> correspondences;

	Polyhedron(){}
	Polyhedron(vector<vec3>& verts, vector<int3>& fs, vector<vec3>& corres) {
		faces = fs;
		vertices = verts;
        current_vertices = verts;
		correspondences = corres;
	}
};

class int2 {
public:
	int u, v;
	int2() { u = 0, v = 0; }
	int2(int u_, int v_) {
		u = u_;
		v = v_;
	}
    int operator[] (int idx) const {
		if (idx == 0)
			return u;
		else if (idx == 1)
			return v;
	}
};

class vec4 {
public:
	float v[4];
	vec4(){}
	vec4(float a, float b, float c, float d) {
		v[0] = a;
		v[1] = b;
		v[2] = c;
		v[3] = d;
	}
    float operator[] (int idx) const {
		return v[idx];
	}

	vec4 operator*(float mul) const {
		return vec4(v[0] * mul, v[1] * mul, v[2] * mul, v[3] * mul);
    }
    vec4 operator+(const vec4 &another) const {
        return vec4(
            v[0] + another.v[0],
            v[1] + another.v[1],
            v[2] + another.v[2],
            v[3] + another.v[3]
        );
    }
    vec4 operator-(const vec4 &another) const {
        return vec4(
            v[0] - another.v[0],
            v[1] - another.v[1],
            v[2] - another.v[2],
            v[3] - another.v[3]
        );
    }
};

#endif // UTILITY_H