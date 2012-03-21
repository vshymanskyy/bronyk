#ifndef PAW_H_
#define PAW_H_

#include "../ctrl/Drives.h"
#include "../ctrl/Sensors.h"

class Paw {

public:
	Paw() {}
	virtual ~Paw() {}

	void SetHolding(bool h) { targetHolding = h; }
	bool GetHolding() { return currentHolding; }

private:
	void Take() {
		_leftHand->Rotate(-10);
		_rightHand->Rotate(10);
	}

	void Drop() {
		_leftHand->Rotate(10);
		_rightHand->Rotate(-10);
	}

private:
	// State (actual info)
	bool currentHolding;

	// Additional info
	bool targetHolding;

	// Hardware
	IDriveServo* _leftHand;
	IDriveServo* _rightHand;
	IValueSensor<float>* _poanSensor;
};

#endif /* PAW_H_ */
