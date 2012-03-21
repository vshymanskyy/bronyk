#ifndef ROBOT_H_
#define ROBOT_H_

#include "Mentality.h"
#include "Borders.h"
#include "Wheels.h"
#include "Lift.h"
#include "Paw.h"

class Robot {

public:
	Robot() {}
	virtual ~Robot() {}

private:
	Mentality	_mentality;
	Borders		_borders;
	Wheels		_wheels;
	Lift		_lift;
	Paw			_paw;
};

#endif /* ROBOT_H_ */
