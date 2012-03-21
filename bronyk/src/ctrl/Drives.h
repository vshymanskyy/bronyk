#pragma once

class IDriveServo {
public:
	/// Destructor
	virtual ~IDriveServo() {}

	/// Start rotation
	virtual void Rotate(float degrees) = 0;
	/// Return current position in degrees
	virtual float GetPosition() const = 0;
	/// ...
};

class IDriveDc {
public:
	/// Destructor
	virtual ~IDriveDc() {}

	/// Start rotation
	virtual void Start() = 0;
	/// Start rotation
	virtual void Stop() = 0;
};
