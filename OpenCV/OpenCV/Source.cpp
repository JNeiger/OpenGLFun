#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <windows.h>

using namespace cv;
using namespace std;

int MAX_KERNEL_LENGTH = 5;

Mat original;
Mat edited;

vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cout << " Usage: display_original originalToLoadAndDisplay" << endl;
		return -1;
	}


	original = imread(argv[1], IMREAD_COLOR); // Read the file

	if (!original.data) // Check for invalid input
	{
		cout << "Could not open or find the original" << std::endl;
		return -1;
	}

	long int before = GetTickCount();

	/*// Blur to get rid of noise
	for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
	{
		// Homogenoeous Blur, KERNEL LENGTH OF 7~, 31 ms
		//blur(original, edited, Size(i, i), Point(-1, -1));

		// Gaussian Blur ~10-13, 15 ms
		//GaussianBlur(original, edited, Size(i, i), 0, 0);

		// Median Blur ~5, 62
		medianBlur(original, edited, i);

		// Bilateral Filter (No go), 188
		bilateralFilter(original, edited, i, i * 2, i / 2);
	}
	*/
	cvtColor(original, edited, CV_BGR2GRAY);
	// Erode to get rid of noise, 16
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
	erode(edited, edited, element);
	

	threshold(edited, edited, 220, 255, THRESH_BINARY);

	// Erode to get rid of the remaining noise
	erode(edited, edited, element);

	// Find contours
	findContours(edited, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	// Computes Bounding box for each point
	vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
		boundRect[i] = boundingRect(Mat(contours_poly[i]));
	}

	// Draw nice hulls
	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(255, 255, 255);
		rectangle(edited, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);
	}
	
	printf("Contour Amour %i\n", contours.size());
	Mat singleChann;
	cvtColor(original, singleChann, CV_BGR2GRAY);
	for (int i = 0; i < contours.size(); i++)
	{
		Rect roi;
		roi = boundRect[i];
		Mat crop = singleChann(roi);
		printf("Raw %i, Area %i, Percent %f\n", countNonZero(crop), boundRect[i].area(), (float)countNonZero(crop) / (float)boundRect[i].area());
		namedWindow("ROI", WINDOW_AUTOSIZE); // Create a window for edited original.
		imshow("ROI", crop); // Show our original inside it.
	}
	

	// Get Delta T
	//cout << (GetTickCount() - before);
	
	namedWindow("Original", WINDOW_AUTOSIZE); // Create a window for original original.
	imshow("Original", original); // Show our original inside it.
	namedWindow("Edited", WINDOW_AUTOSIZE); // Create a window for edited original.
	imshow("Edited", edited); // Show our original inside it.

	waitKey(0); // Wait for a keystroke in the window
	return 0;
}