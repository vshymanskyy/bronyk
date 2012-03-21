#pragma once

void ImageFiltering() {
	if (IplImage* boardImage = cvLoadImage("5.jpg", CV_LOAD_IMAGE_COLOR)) {
		//cvShowImage("Raw", boardImage);
		if (IplImage* Iimg = cvCreateImage(cvSize(boardImage->width,boardImage->height), IPL_DEPTH_8U, 1)) {
			cvSetImageCOI(boardImage, 3); // Select red channel
			cvCopy( boardImage, Iimg, NULL ); // Iat now holds red channel

			cvShowImage("Red", Iimg);

			cvAdaptiveThreshold(Iimg, Iimg, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 91, 2);

			//cvShowImage("Threshold", Iimg);

			cvWaitKey(0);

			cvReleaseImage(&Iimg);
			cvDestroyWindow("Threshold");
		}
		cvReleaseImage(&boardImage);
		cvDestroyWindow("Raw");
	}
}
