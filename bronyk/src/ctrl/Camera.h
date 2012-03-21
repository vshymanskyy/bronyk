#pragma once
#include <stdint.h>
#include <stddef.h>

#include <opencv/cv.h>

struct ICamera {
	/// Destructor
	virtual ~ICamera() {}

	/// Grabs frame from device
	virtual bool GrabFrame() = 0;
	/// Gets grabbed frame
	virtual IplImage* GetFrame() = 0;
	/// Releases frame if needed
	virtual void ReleaseFrame(IplImage** frame) = 0;

	virtual CvSize GetFrameSize() = 0;
};
