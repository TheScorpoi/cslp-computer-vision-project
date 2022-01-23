#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include <string>


using namespace std;


// https://docs.opencv.org/4.x/df/d9d/tutorial_py_colorspaces.html

void on_mouse_click(int event, int x, int y, int flags, void* ptr) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        cv::Mat* snapshot = (cv::Mat*)ptr;
        cv::Mat &img = *((cv::Mat*)(ptr));
        cv::Vec3b pixel = snapshot->at<cv::Vec3b>(x, y);
        cv::Vec3b bgrPixel(snapshot->at<cv::Vec3b>(y, x));
        circle(*snapshot,cv::Point(x,y),4, cv::Scalar(0,0,255),cv::FILLED,8,0);
        cv::imshow("Snapshot", *snapshot);
        int b, g, r;
        b = pixel[0];
        g = pixel[1];
        r = pixel[2];

        cv::Mat3b bgr(bgrPixel);
        cv::Mat3b hsv;
        cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

        cv::Vec3b hsvPixel(hsv.at<cv::Vec3b>(0,0));

        std::string rgbText = "[" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + "]";
        cout << "rgb " << rgbText << std::endl;
        cout << "hsv "<<hsvPixel << endl;
        
        int thresh = 40;
        cv::Scalar minHSV = cv::Scalar(hsvPixel.val[0] - thresh, hsvPixel.val[1] - thresh, hsvPixel.val[2] - thresh);
        cv::Scalar maxHSV = cv::Scalar(hsvPixel.val[0] + thresh, hsvPixel.val[1] + thresh, hsvPixel.val[2] + thresh);
        //cout << minHSV << endl;
        //cout << maxHSV << endl;

    }
}

int main(int argc, char** argv) {
    cv::VideoCapture capture(0);

    if (!capture.isOpened()) {
        std::cout << "Error opening VideoCapture." << std::endl;
        return -1;
    }

    cv::Mat frame, snapshot;
    capture.read(frame);

    snapshot = cv::Mat(frame.size(), CV_8UC3, cv::Scalar(23, 32, 32));
    cv::imshow("Snapshot", snapshot);

    cv::setMouseCallback("Snapshot", on_mouse_click, &snapshot);

    int keyVal;
    while (1) {
        if (!capture.read(frame)) {
            break;
        }
        cv::imshow("Video", frame);


        keyVal = cv::waitKey(1) & 0xFF;
        if (keyVal == 113 || keyVal == 81) {  // q
            break;
        } else if (keyVal == 115 || keyVal == 83) {  // S ou s
            snapshot = frame.clone();
            cv::imshow("Snapshot", snapshot);
        }
    }
    return 0;
}