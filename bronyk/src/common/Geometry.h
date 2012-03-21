#pragma once

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <opencv/cv.h>

typedef cv::Point2f vec2;

inline
float Distance(const vec2& v1, const vec2& v2) {
	return sqrt(Square(v1.x - v2.x) + Square(v1.y - v2.y));
}

inline
float Length(const vec2& v) {
	return sqrt(Square(v.x) + Square(v.y));
}

inline
void Normalize(vec2& v) {
	const float len = 1.0/Length(v);
	v *= len;
}

struct Segment {
	vec2 a, b;

	Segment()
		: a ()
		, b ()
	{
	}

	Segment(const vec2& A, const vec2& B)
		: a (A)
		, b (B)
	{
	}

	float Length() const {
		return Distance(a, b);
	}

	bool Overlaps(const vec2& p) const {
		return InRange(p.x, a.x, b.x) && InRange(p.y, a.y, b.y);
	}

	void Extend(const vec2& p) {
		if (Overlaps(p)) return;
		if (Distance(a, p) < Distance(b, p)) { // TODO: squared
			a = p;
		} else {
			b = p;
		}
	}

	vec2 GetVect() const {
		vec2 u(b.x-a.x, b.y-a.y);
		Normalize(u);
		return u;
	}

	vec2 GetPerpVect() const {
		vec2 u = GetVect();
		u.y = -u.y;
		Swap(u.x, u.y);
		return u;
	}

	/*vec2 Intersect(const Segment &s) {
		const float x[4] = { a.x, b.x, s.a.x, s.b.x };
		const float y[4] = { a.y, b.y, s.a.y, s.b.y };

		const float d = (y[3] - y[2])*(x[1] - x[0]) - (x[3] - x[3])*(y[1] - y[0]);
		const float ua = ((x[3] - x[2])*(y[0] - y[2]) - (y[3] - y[2])*(x[0] - x[2])) / d;
		const float ub = ((x[1] - x[0])*(y[0] - y[2]) - (y[1] - y[0])*(x[0] - x[2])) / d;
	}*/
};

struct Line {
	// y= mx + b
	float m, b;

	Line(const Segment& s) {
		if (s.b.x == s.a.x) {
			m  = 10000;
		} else {
			m  = (s.b.y - s.a.y) / (s.b.x - s.a.x);
		}
		b = s.a.y - m * s.a.x;
	}

	bool Contains(const vec2& p, float epsilon = 0.001) {
		float  y = m * p.x + b;
		return fabs(p.y - y) < epsilon;
	}

};

#endif /* GEOMETRY_H_ */
