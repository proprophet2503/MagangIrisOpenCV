#include <opencv2/opencv.hpp>
#include<opencv2/videoio.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    
    // koefisien dari data set
    float a = 0.008498;  
    float b = -1.986033;  
    float c = 124.624314;  

    VideoCapture cap(0);  
    if (!cap.isOpened()){
        return -1;
    } 

    Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()){
            break;
        } 

        Mat hsv_frame, mask;
        cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

        // Adjust HSV range for yellow color based on the image
        Scalar lower_yellow(20, 70, 70);  // Lower bound for yellow (adjusted)
        Scalar upper_yellow(40, 255, 255);  // Upper bound for yellow (adjusted)
        inRange(hsv_frame, lower_yellow, upper_yellow, mask);

        // Apply Gaussian blur to reduce noise
        GaussianBlur(mask, mask, Size(9, 9), 2);

        // Use morphological closing to fill small gaps and smooth the object
        Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
        morphologyEx(mask, mask, MORPH_CLOSE, kernel);

        // Find contours of the detected yellow object
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE); // RETR_EXTERNAL to get the external contour

        if (contours.size() > 0) {
            // Find the largest contour by area
            int largest_contour_index = 0;
            double max_area = contourArea(contours[0]);
            for (size_t i = 1; i < contours.size(); i++) {
                double area = contourArea(contours[i]);
                if (area > max_area) {
                    max_area = area;
                    largest_contour_index = i;
                }
            }

            // Get the minimum area rectangle for the largest contour
            RotatedRect min_rect = minAreaRect(contours[largest_contour_index]);

            // Draw the rotated rectangle on the frame
            Point2f rect_points[4];
            min_rect.points(rect_points);
            for (int j = 0; j < 4; j++) {
                line(frame, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 255, 0), 2);
            }

            // Calculate the object pixel width from the rectangle
            float object_pixel_width = min(min_rect.size.width, min_rect.size.height);

            if (object_pixel_width > 0) {
                // Use the quadratic regression formula to calculate the real-life distance
                float calculated_distance = a * pow(object_pixel_width, 2) + b * object_pixel_width + c;

                // Calculate the contour area in pixels
                double contour_area = contourArea(contours[largest_contour_index]);

                // Output the calculated distance, pixel width, and contour area to the console
                cout << "Calculated Real-Life Distance (cm): " << calculated_distance << endl;
                cout << "Pixel Width: " << object_pixel_width << endl;
                cout << "Contour Area (pixels): " << contour_area << endl;

                // Display the distance, pixel width, and contour area on the video frame
                string distance_text = "Distance: " + to_string(calculated_distance) + " cm";
                string pixel_width_text = "Pixel Width: " + to_string(object_pixel_width) + " px";
                string contour_area_text = "Area: " + to_string(contour_area) + " px^2";

                // Position the text on the frame
                putText(frame, distance_text, Point(min_rect.center.x - 50, min_rect.center.y - 40),
                        FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
                putText(frame, pixel_width_text, Point(min_rect.center.x - 50, min_rect.center.y),
                        FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
                putText(frame, contour_area_text, Point(min_rect.center.x - 50, min_rect.center.y + 40),
                        FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
            }
        }

        imshow("Camera", frame);
        imshow("HSV Frame", hsv_frame);
        imshow("Stabilo Kuning Terdeteksi", mask);

        if (waitKey(30) == 'q') {
            break;
        }
    }

    // Release the camera
    cap.release();
    
    // Close all OpenCV windows
    destroyAllWindows();

    return 0;
}
