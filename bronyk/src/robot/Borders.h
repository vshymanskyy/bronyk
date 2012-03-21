#ifndef BORDERS_H_
#define BORDERS_H_

#include "../ctrl/Sensors.h"

class Borders {
public:
	Borders() {}
	virtual ~Borders() {}
private:
	// vec2 _directions;
	IValueSensor<float>* _sensors[7];
};

#endif /* BORDERS_H_ */
