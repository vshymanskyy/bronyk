#pragma once

#include "../mstd/Thread.h"

/// Concept of controller
template <class T>
class IController
	: private Thread
{
public:
	/// Destructor
	virtual ~IController() {}

	virtual void DoSomething() = 0;

private:
	virtual void Run() {
		for(;;) {
			if (currentState != targetState) {
				DoSomething();
			} else {
				// Suspend;
			}
		}
	}
private:
	T currentState;
	T targetState;
};
