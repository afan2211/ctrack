#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

using namespace cv;
using namespace std;

void plotPoint(Mat& mat, Point pt) {
	rectangle(mat, pt, pt, Scalar(0, 0, 255), 30);
}

int main(int argc, char** argv) {
	VideoCapture stream(0);

	Mat lastFrame;
	Mat twoFrame;
	Mat threeFrame;

	stream.read(lastFrame); // fixes a race condition
	stream.read(twoFrame);
	stream.read(threeFrame);

	double upperRightBound = 0, upperLeftBound = lastFrame.cols;
	Point upperLeftBoundP, upperRightBoundP;

	Point centerOfGravity = Point(lastFrame.cols / 2, lastFrame.rows / 2);

	for(;;) {
		Mat flipped_frame;
		stream.read(flipped_frame);

		Mat frame;
		flip(flipped_frame, frame, 1);
		
		Mat visualization = frame.clone();
		cvtColor(frame, visualization, CV_BGR2GRAY);

		// find pixels in motion
		Mat out1, out2, delta;
		absdiff(twoFrame, frame, out1);
		absdiff(lastFrame, frame, out2);
		bitwise_and(out1, out2, delta);

		// extract user
		threshold(delta, delta, 10, 255, THRESH_BINARY);
		blur(delta, delta, Size(17, 17), Point(-1, -1));
		cvtColor(delta, delta, CV_BGR2GRAY);
		adaptiveThreshold(delta, delta, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 25, -7);	
		blur(delta, delta, Size(105, 105), Point(-1, -1));
		threshold(delta, delta, 15, 255, THRESH_BINARY);
		blur(delta, delta, Size(25, 25), Point(-1, -1));
		threshold(delta, delta, 1, 255, THRESH_BINARY);

		// find contours
		Canny(delta, delta, 40, 40 * 3, 3);
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		vector<Point> approxShape;

		findContours(delta.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		Mat contourVisualization = Mat::zeros(delta.size(), CV_8UC3);

		for(int i = 0; i < contours.size(); ++i) {
			double t_arcLength = arcLength(Mat(contours[i]), true);

			if(t_arcLength > 1000) { // remove tiny contours.. don't waste your time
				drawContours(contourVisualization, contours, i, Scalar(255, 0, 0), 3);
				drawContours(visualization, contours, i, Scalar(255, 0, 0), 10);
			}
		}

//		imshow("Relevant", delta);
		imshow("contourVisualization", contourVisualization);
		imshow("Visualization", visualization);

		/*

		cvtColor(delta, delta, CV_BGR2GRAY);
		blur(delta, delta, Size(75, 75), Point(-1, -1));
		threshold(delta, delta, 1, 255, THRESH_BINARY);
		adaptiveThreshold(delta, delta, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 25, -1);	

		uint64_t motion = countNonZero(delta);

		Canny(delta, delta, 40, 40 * 3, 3);

		// obtain contours
		std::vector<std::vector<Point> > contours;
		std::vector<Vec4i> hierarchy;
		std::vector<Point> approxShape;

		findContours(delta.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		Mat contourDrawing = Mat::zeros(delta.size(), CV_8UC3);

		double totalY = 0, totalX = 0, count = 0;

		for(int i = 0; i < contours.size(); ++i) {
			double my_arcLength = arcLength(Mat(contours[i]), true);
			approxPolyDP(contours[i], contours[i], my_arcLength * 0.02, true);

			if(my_arcLength > 1000) {
				drawContours(contourDrawing, contours, i, Scalar(255, 0, 0), 3);

				for(int j = 0; j < contours[i].size(); ++j) {
					totalX += contours[i][j].x;
					totalY += contours[i][j].y;

					count++;
				}
			}
		}

		Point local_centerOfGravity(totalX / count, totalY / count);
		double cog_fluctuation = norm(local_centerOfGravity - centerOfGravity);

		if(local_centerOfGravity.x > 0)
			//if(cog_fluctuation < 100) // silences large fluctuations
				if(motion > 22500 || cog_fluctuation > 200) // single limb movements should be ignored for our purposes
					centerOfGravity = local_centerOfGravity;

		double maxDistance = 0;

		for (int i = 0; i < contours.size(); ++i) {
			if(arcLength(Mat(contours[i]), true) > 1000) {
				for(int j = 0; j < contours[i].size(); ++j) {
					if(norm(contours[i][j] - centerOfGravity) > maxDistance)
						maxDistance = norm(contours[i][j] - centerOfGravity);
				}
			}
		}

		// create limb threshold
		double limbThreshold = maxDistance * 0.4;
		if(limbThreshold < 100) limbThreshold = 100; // lower minimum threshold

		Mat limbDrawing = Mat::zeros(delta.size(), CV_8UC3);

		std::vector<Point> interestingPoints;

		for(int i = 0; i < contours.size(); ++i) {
			if(arcLength(Mat(contours[i]), true) > 1000) {
				for(int j = 0; j < contours[i].size(); ++j) {
					double distance = norm(centerOfGravity - contours[i][j]);
					
					if(distance > limbThreshold) {
						interestingPoints.push_back(contours[i][j]);
						//plotPoint(limbDrawing, contours[i][j]);
					}
				}
			}
		}

		//imshow("pts", limbDrawing);

		double local_upperRightBound = 0, local_upperLeftBound = lastFrame.cols;
		Point local_upperRightBoundP, local_upperLeftBoundP;

		for(int i = 0; i < interestingPoints.size(); ++i) {
			if(interestingPoints[i].x > local_upperRightBound && interestingPoints[i].x > centerOfGravity.x) {
				local_upperRightBound = interestingPoints[i].x; // right hand
				local_upperRightBoundP = interestingPoints[i];
			}

			if(interestingPoints[i].x < local_upperLeftBound && interestingPoints[i].x < centerOfGravity.x) {
				local_upperLeftBound = interestingPoints[i].x;
				local_upperLeftBoundP = interestingPoints[i];
			}

		}

		if(local_upperRightBound > 0)
			if(norm(upperRightBoundP - local_upperRightBoundP) < 400)
				upperRightBoundP = local_upperRightBoundP;

		if(local_upperLeftBound > 0)
			if(norm(upperLeftBoundP - local_upperLeftBoundP) < 400)
				upperLeftBoundP = local_upperLeftBoundP;

		Mat copyFrame = frame.clone();
		plotPoint(copyFrame, centerOfGravity);
		//plotPoint(copyFrame, upperLeftBoundP);
		//plotPoint(copyFrame, upperRightBoundP);

		//imshow("copyFrame", copyFrame);


		//imshow("limbDrawing", limbDrawing);

		*/

		if(waitKey(10) == 27) {
			break;
		}

		threeFrame = twoFrame;
		twoFrame = lastFrame;
		lastFrame = frame;
	}

	return 0;
}