#ifndef WHEELS_H_
#define WHEELS_H_

#include "../ctrl/Drives.h"

class Wheels {
public:
	Wheels() {}
	virtual ~Wheels() {}

	void RotateLeft() {

	}

	void RotateRight() {

	}

	void MoveTowards() {

	}

	void MoveBackwards() {

	}

private:
	IDriveDc* _leftWheel;
	IDriveDc* _rightWheel;
};

#endif /* WHEELS_H_ */
