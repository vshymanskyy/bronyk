#pragma once
#include "../Mouse.h"
#include <XThread.h>
#include <XLog.h>

class MousePs2
	: public IMouse
	, public XThread
{
public:
	MousePs2(const char* dev)
		: mDev (dev)
		, mLog (dev)
	{
	}

	~MousePs2() {
	}

	virtual int Run();
private:
	const char* mDev;
	XLog	mLog;
};
