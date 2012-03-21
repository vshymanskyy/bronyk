#pragma once

#include "../Drives.h"
#include "../Sensors.h"
#include <XLog.h>
#include <XHelpers.h>

namespace Virtual {

	typedef float Real;
	typedef struct { Real x,y; } Point, Vector, Size;

	struct Segment { Point a; Point b; };
	struct Ray { Point p; Vector v; };
	struct Rectangle { Point center; Size size; Real rotation; };
	struct Circle { Point center; Real radius; };

	inline void SleepMs(int ms) {
		usleep(ms*1000);
	}

	class DriveServo : public IDriveServo {
	public:
		DriveServo(const char* name)
			: mLog (name)
			, mRotation (0.0)
		{}
		/// Destructor
		virtual ~DriveServo() {}

		/// Start rotation
		virtual void Rotate(float degrees) {
			LOG(mLog, "Rotating...");
			for (int i=0; i<100; i++) {
				SleepMs(20);
				mRotation += degrees/100;
			}
			LOG(mLog, "Rotation finished");
		}
		/// Return current position in degrees
		virtual float GetPosition() const {
			return mRotation;
		}
		/// ...
	private:
		XLog mLog;
		float mRotation;
	};

	class DriveDc : public IDriveDc {
	public:

		DriveDc(const char* name)
			: mLog (name)
		{}
		/// Destructor
		virtual ~DriveDc() {}

		/// Start rotation
		virtual void Start() {
			LOG(mLog, "Started");
		}
		/// Start rotation
		virtual void Stop() {
			LOG(mLog, "Stopped");
		}
	private:
		XLog mLog;
	};

	class Robot {

	private:
		Rectangle bounds;
	};

	class World {

	private:
		Rectangle bounds;
	};

}
