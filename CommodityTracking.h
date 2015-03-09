#ifndef __COMMODITY_TRACKING_H_
#define __COMMODITY_TRACKING_H_

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

using namespace cv;
using namespace std;

class FrameHistory {
	public:
		FrameHistory(VideoCapture& stream);
		void append(Mat frame);
		Mat motion(Mat frame);
		Mat getLastFrame();
	private:
		Mat m_lastFrame, m_twoFrame, m_threeFrame;
};

class Skeleton {
public:
	Skeleton() {
		// initialize with an empty skeleton

		center_of_rect = Point(0, 0);
		rightMostAbove = Point(0, 0);
		rightMostBelow = Point(0, 0);
		leftMostAbove = Point(0, 0);
		leftMostBelow = Point(0, 0);
		topMost = Point(0, 0);

		fullWidth = 1; // prevent division by zero errors
		fullHeight = 1;
	}

	Point center_of_rect, rightMostAbove, rightMostBelow, leftMostAbove, leftMostBelow, topMost;
	int fullWidth, fullHeight;

	void visualize(Mat visualization) {
		line(visualization, topMost, center_of_rect, Scalar(0, 255, 0), 2);
		line(visualization, rightMostAbove, center_of_rect, Scalar(0, 255, 0), 2);
		line(visualization, leftMostAbove, center_of_rect, Scalar(0, 255, 0), 2);
		line(visualization, rightMostBelow, center_of_rect, Scalar(0, 255, 0), 2);
		line(visualization, leftMostBelow, center_of_rect, Scalar(0, 255, 0), 2);
	}

	// utility methods for getting individual skeleton points
	// returns a Point2d normalized to size of screen 

	Point2d center() { return Point2d( (double) center_of_rect.x / fullWidth, (double) center_of_rect.y / fullHeight); }
	Point2d head() { return Point2d( (double) topMost.x / fullWidth, (double) topMost.y / fullHeight); }
	Point2d rightHand() { return Point2d( (double) rightMostAbove.x / fullWidth, (double) rightMostAbove.y / fullHeight); }
	Point2d leftHand() { return Point2d( (double) leftMostAbove.x / fullWidth, (double) leftMostAbove.y / fullHeight); }

};

Mat extractUserMask(Mat& delta, double sensitivity);
Mat simplifyUserMask(Mat& mask, Mat& frame, int minimumArclength);
std::vector<Point> getEdgePoints(Mat frame, Mat simplifiedUserMask, int minimumArclength, bool draw, std::vector<std::vector<Point> >& edgePointsList);
Skeleton getSkeleton(VideoCapture& stream, FrameHistory& history, Skeleton last, bool _flip, int minimumArclength, int userSensitivity, int limbGracePeriod);
void autoCalibrateSensitivity(int* userSensitivity, VideoCapture& stream, FrameHistory& history, int minimumArclength, int interval, int limbGracePeriod);

#endif