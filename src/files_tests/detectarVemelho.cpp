#include <iostream>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    VideoCapture cap(0);  //capture the video from webcam

    if (!cap.isOpened())  // if not success, exit program
    {
        cout << "Cannot open the web cam" << endl;
        return -1;
    }

    //valores para detectar vermelho
    int iLowH = 134;
    int iHighH = 179;

    int iLowS = 118;
    int iHighS = 255;

    int iLowV = 146;
    int iHighV = 255;

    int iLastX = -1;
    int iLastY = -1;

    //Capture a temporary image from the camera
    Mat imgTmp;
    cap.read(imgTmp);

    //Create a black image with the size as the camera output
    Mat imgLines = Mat::zeros(imgTmp.size(), CV_8UC3);
    ;

    while (true) {
        Mat imgOriginal;

        bool bSuccess = cap.read(imgOriginal);  // read a new frame from video

        if (!bSuccess)  //if not success, break loop
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }

        Mat imgHSV;

        cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);  //Convert the captured frame from BGR to HSV

        Mat imgThresholded;
        Mat imgThresholded1, imgThresholded2, imgThresholded3;

        inRange(imgHSV, Scalar(20, 100, 100), Scalar(30, 255, 255), imgThresholded1);  //Threshold the image
        inRange(imgHSV, Scalar(170, 160, 60), Scalar(180, 255, 256), imgThresholded2);  //Threshold the image
        inRange(imgHSV, Scalar(70, 160, 60), Scalar(90, 255, 256), imgThresholded3);  //Threshold the image

          imgThresholded = imgThresholded1 | imgThresholded2 | imgThresholded3;

        //morphological opening (removes small objects from the foreground)
        erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

        //morphological closing (removes small holes from the foreground)
        dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

        //Calculate the moments of the thresholded image
        Moments oMoments = moments(imgThresholded);

        double dM01 = oMoments.m01;
        double dM10 = oMoments.m10;
        double dArea = oMoments.m00;

        imshow("Thresholded Image", imgThresholded);  //show the thresholded image

        imgOriginal = imgOriginal + imgLines;
        imshow("Original", imgOriginal);  //show the original image

        if (waitKey(30) == 27)  //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            break;
        }
    }

    return 0;
}