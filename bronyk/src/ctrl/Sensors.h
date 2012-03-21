#pragma once

#include <XDelegate.h>
#include <XQueue.h>
#include <time.h>

/*
 * Event sensor
 * It signals (calls a Delegate) on specific trigger
 * Examples: pushbutton, pressure exceeds some level, etc
 */
class IEventSensor {
public:
	typedef XDelegate<void (void)> Handler;

	// Constructor
	IEventSensor(Handler eh) : _handler(eh) {};
	/// Destructor
	virtual ~IEventSensor() {}

protected:
	void Notify() {
		_handler();
	}

private:
	Handler _handler;
};

/*
 * Value sensor
 * Examples: infrared, temperature, pressure
 */
template <typename T>
class IValueSensor {
	/// Destructor
	virtual ~IValueSensor() {}

	/// Gets value from sensor
	/// returns true if value is available
	virtual bool GetValue(T* value) = 0;
};

template <typename T>
struct Entry {
	time_t	time;
	T		data;
};

template <typename T>
class SensorBuffer : public XQueue < Entry<T>, 32 > {};
