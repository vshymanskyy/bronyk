#ifndef LIFT_H_
#define LIFT_H_

#include "../ctrl/Sensors.h"
#include "../ctrl/Drives.h"

class Lift {
public:
	enum Level {
		//L0,	// For funny figure
		L1,		// Ground level
		L2,
		L3		// Highest
	};

public:
	Lift() {}
	virtual ~Lift() {}

	void SetLevel(const Level l) { targetLevel = l; }
	Level GetLevel() const { return currentLevel; }

private:
	// State (actual info)
	Level currentLevel;

	// Additional info
	Level targetLevel;

	// Hardware
	IDriveServo* _lifter;
	IEventSensor* _level0reached;
	IEventSensor* _level1reached;
	IEventSensor* _level2reached;
	IEventSensor* _level3reached;
};

#endif /* LIFT_H_ */
