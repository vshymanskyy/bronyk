#pragma once

class IMouse {
public:
	IMouse() : x (0), y(0), z(0) { }
	virtual ~IMouse() { }

	long long X() { return x; };
	long long  Y() { return y; };
protected:
	long long  x;
	long long  y;
	long long  z;
};
