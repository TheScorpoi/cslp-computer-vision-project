#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;

using namespace cv::gpu;


IplImage* GetThresholdedImage(IplImage* imgHSV){       
    IplImage* imgThresh=cvCreateImage(cvGetSize(imgHSV),IPL_DEPTH_8U, 1);
    cvInRangeS(imgHSV, cvScalar(170,160,60), cvScalar(180,255,256), imgThresh); 
    return imgThresh;
} 
int main(){
    //Char Firetype = type;
     CvCapture* capture = cvCaptureFromCAM(1);
     if(!capture){
        printf("Capture failure\n");
        return -1;
    }
    IplImage* frame=0;

    cvNamedWindow("Video");     
    cvNamedWindow("Fire");
    cvNamedWindow("Info");
    //iterate through each frame of the video     
    while(true){
        frame = cvQueryFrame(capture);           
        if(!frame) break;
        frame=cvCloneImage(frame); 
        cvSmooth(frame, frame, CV_GAUSSIAN,3,3); //smooth the original image using Gaussian kernel
        cv::Mat imgMat(frame);
        cv::Mat mask1;
        cv::inRange(imgMat, cv::Scalar(20, 100, 100), cv::Scalar(30, 255, 255), mask1);
        cv::Mat mask2;
        cv::inRange(imgMat, cv::Scalar(170,160,60), cv::Scalar(180,255,256), mask2);
        cv::Mat mask3;
        cv::inRange(imgMat, cv::Scalar(70,160,60), cv::Scalar(90,255,256), mask3);
        // combine them
        cv::Mat mask_combined = mask1 | mask2 | mask3;
        // now since our frame from the camera is bgr, we have to convert our mask to 3 channels:
        cv::Mat mask_rgb;
        cv::cvtColor( mask_combined, mask_rgb, CV_GRAY2BGR );
        IplImage framemask = imgMat & mask_rgb;
        IplImage* imgHSV = cvCreateImage(cvGetSize(framemask), IPL_DEPTH_8U, 3); 
        cvCvtColor(frame, imgHSV, CV_BGR2HSV); //Change the color format from BGR to HSV


        //This function threshold the HSV image and create a binary image
        // function below to get b&w image
        IplImage* imgThresh = GetThresholdedImage(imgHSV);
        cvSmooth(imgThresh, imgThresh, CV_GAUSSIAN,3,3); //smooth the binary image using Gaussian kernel
        cvShowImage("Fire", imgThresh);           
        cvShowImage("Video", frame);
        //          cvNamedWindow("Info", Firetype);           
        cvReleaseImage(&imgHSV);
        cvReleaseImage(&imgThresh);            
        cvReleaseImage(&frame);
        //Wait 50mS
        int c = cvWaitKey(10);
        //If 'ESC' is pressed, break the loop
        if((char)c==27 ) break;      
    }
    cvDestroyAllWindows() ;
    cvReleaseCapture(&capture);     
    return 0;
}