#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    VideoCapture cap("/home/jeremymboe/cmpt130/belajarOpenCVCPP/2.video/VideoAsli.avi");

    if (!cap.isOpened()) {
        return -1;
    }

    Mat frame, hsvFrame, mask;

    double x_robot = 0.0, y_robot = 0.0;
    double prev_ballX_cm = 0.0, prev_ballY_cm = 0.0;  
    const float pixelsToCm = 0.10;  

    bool isFirstFrame = true; 
    double minBallArea = 40.0;  
    double maxBallArea = 5000.0;  

    // Define the region to exclude (center of the field)
    Point2f centerField(cap.get(CAP_PROP_FRAME_WIDTH) / 2, cap.get(CAP_PROP_FRAME_HEIGHT) / 2);
    int exclusionRadius = 100;  // Exclude objects within this radius

    while (true) {
        cap >> frame;  
        if (frame.empty()) {
            break;  
        }

        cvtColor(frame, hsvFrame, COLOR_BGR2HSV);

        Scalar lower_orange(5, 150, 150); 
        Scalar upper_orange(15, 255, 255);

        inRange(hsvFrame, lower_orange, upper_orange, mask);

        // Exclude center area (avoid detecting objects like in the image)
        circle(mask, centerField, exclusionRadius, Scalar(0), -1);

        // Find contours of the orange object (the ball)
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // If contours are found, calculate the center of the ball using minAreaRect
        if (!contours.empty()) {
            // Assume the largest contour that fits within the ball size limits is the ball
            double maxArea = 0;
            int largestContourIndex = -1;

            for (int i = 0; i < contours.size(); i++) {
                double area = contourArea(contours[i]);
                if (area > maxArea && area >= minBallArea && area <= maxBallArea) {
                    maxArea = area;
                    largestContourIndex = i;
                }
            }

            if (largestContourIndex != -1) {
                // Fit a rotated rectangle around the ball
                RotatedRect rect = minAreaRect(contours[largestContourIndex]);

                //Get the center of the rectangle
                Point2f rect_points[4];
                rect.points(rect_points);
                Point2f center = rect.center;

                // Convert ball position to centimeters
                double object_x_cm = center.x * pixelsToCm;
                double object_y_cm = center.y * pixelsToCm;

                // If it's not the first frame, update the robot's position
                if (!isFirstFrame) {
                    x_robot += object_x_cm - prev_ballX_cm;
                    y_robot += object_y_cm - prev_ballY_cm;
                }

                // Update last known ball position
                prev_ballX_cm = object_x_cm;
                prev_ballY_cm = object_y_cm;

                // Set the first frame flag to false after processing the first frame
                isFirstFrame = false;

                // Draw the detected ball's rectangle on the frame
                for (int j = 0; j < 4; j++) {
                    line(frame, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 255, 0), 2);
                }

                circle(frame, Point(center.x - 30, center.y - 30), 5, Scalar(255, 0, 0), -1);
                circle(frame, Point(center.x + 30, center.y - 30), 5, Scalar(255, 0, 0), -1);
                circle(frame, Point(center.x - 30, center.y + 30), 5, Scalar(255, 0, 0), -1);
                circle(frame, Point(center.x + 30, center.y + 30), 5, Scalar(255, 0, 0), -1);

                std::string positionText = "Posisi robot (" + std::to_string(x_robot) + "," + std::to_string(y_robot) + ") ";
                putText(frame, positionText, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
            }
        }

        imshow("Video Robot", frame);

        if (waitKey(33) == 'q') {
            break;
        }
    }

    return 0;
}
