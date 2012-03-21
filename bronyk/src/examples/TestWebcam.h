#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>


void TestWebcam() {

	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, 1.0, 1.0, 0, 2);

	cvNamedWindow("Video", 0);
	if (CvCapture* cap = cvCaptureFromCAM(0)) {
		cvResizeWindow("Video",
				(int) cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH),
				(int) cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT));

		for (int key = -1; key != 27; key = cvWaitKey(10)) {

			if (IplImage* src = cvQueryFrame(cap)) {


				cvSmooth(src, src, CV_GAUSSIAN, 3, 3);
				IplImage* srcSmall = cvCreateImage(cvSize(src->width/3, src->height/3), src->depth, src->nChannels);

				cvResize(src, srcSmall, CV_INTER_LINEAR);

				/*IplImage* hsv = cvCreateImage(cvGetSize(src), 8, 3);
				IplImage* img_h = cvCreateImage(cvGetSize(src), 8, 1);
				IplImage* img_s = cvCreateImage(cvGetSize(src), 8, 1);
				IplImage* img_v = cvCreateImage(cvGetSize(src), 8, 1);
*/
				//cvCvtColor(src, hsv, CV_BGR2HSV);
				//cvSplit(hsv, img_h, img_s, img_v, 0);

				if (IplImage* img2 = cvCreateImage(cvGetSize(srcSmall), IPL_DEPTH_8U, 1)) {
					//cvSetImageCOI(src, 2); // Select red channel
					//cvCopy(src, img2, NULL); // img2 now holds red channel

					cvSplit(srcSmall, NULL, NULL, img2, NULL);

					//cvSmooth(img2, img2, CV_GAUSSIAN, 3, 3);
					//cvAdaptiveThreshold(img2, img2, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 225, 2);


					CvPoint2D32f corners[512];
					int cornerQty = 512;

					if (cvFindChessboardCorners(srcSmall, cvSize(7, 7), corners, &cornerQty,
							CV_CALIB_CB_FAST_CHECK |
							CV_CALIB_CB_NORMALIZE_IMAGE | CV_CALIB_CB_FILTER_QUADS))
					{
						cvFindCornerSubPix(img2, corners, cornerQty, cvSize(11, 11), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
						cvDrawChessboardCorners(img2, cvSize(7, 7), corners, cornerQty, true);
					}

					cvShowImage("Detect", img2);
					cvReleaseImage(&img2);
				}
				cvShowImage("Video", src);
			}
		}
		cvReleaseCapture(&cap);
	}
	cvDestroyWindow("Video");
}
