#include <opencv2/opencv.hpp>
#include<opencv2/videoio.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    
    // koefisien dari data set
    float a = 0.0077;  
    float b = -2.0341;  
    float c = 134.9859;  

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
        
        Scalar lower_yellow(20, 70, 70);  
        Scalar upper_yellow(40, 255, 255); 
        inRange(hsv_frame, lower_yellow, upper_yellow, mask);
    
        GaussianBlur(mask, mask, Size(9, 9), 2);

        Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
        morphologyEx(mask, mask, MORPH_CLOSE, kernel);

        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE); 

        if (contours.size() > 0) {
            int largest_contour_index = 0;
            double max_area = contourArea(contours[0]);
            for (size_t i = 1; i < contours.size(); i++) {
                double area = contourArea(contours[i]);
                if (area > max_area) {
                    max_area = area;
                    largest_contour_index = i;
                }
            }

            RotatedRect min_rect = minAreaRect(contours[largest_contour_index]);

            Point2f rect_points[4];
            min_rect.points(rect_points);
            for (int j = 0; j < 4; j++) {
                line(frame, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 255, 0), 2);
            }

            float object_pixel_width = min(min_rect.size.width, min_rect.size.height);

            if (object_pixel_width > 0) {
                float distance_reallife = a * pow(object_pixel_width, 2) + b * object_pixel_width + c;

                cout << "Distance Real-Life (cm): " << distance_reallife << endl;
                
                string distance_text = "Distance: " + to_string(distance_reallife) + " cm";
               
                if(object_pixel_width<=130){
                    putText(frame, distance_text, Point(min_rect.center.x - 50, min_rect.center.y - 40),
                        FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
                }else{
                    string warning_text = "Stabilo terlalu dekat dari kamera, jarak sulit terdeteksi!";
                    cout << warning_text << endl;
                    putText(frame, warning_text, Point(50, 50), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);
                }
            }
        }
        imshow("Camera", frame);
        imshow("HSV Frame", hsv_frame);
        imshow("Stabilo Kuning Terdeteksi", mask);

        if (waitKey(30) == 'q') {
            break;
        }
    }

    cap.release();
    
    return 0;
}
