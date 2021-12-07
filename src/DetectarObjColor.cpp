#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "Object.cpp"
#include "Object.h"

const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
const int MAX_NUM_OBJECTS = 35;
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT * FRAME_WIDTH / 1.5;

/**
 * @brief This function is used to mark on the window the detected objects 
 * with a contours with color's object and a text with the color's name
 * 
 * @param theObjects 
 * @param frame 
 * @param temp 
 * @param contours 
 * @param hierarchy 
 */
void drawObject(vector<Object> theObjects, Mat &frame, Mat &temp, vector<vector<Point> > contours, vector<Vec4i> hierarchy) {
    for (int i = 0; i < theObjects.size(); i++) {
        cv::drawContours(frame, contours, i, theObjects.at(i).getColor(), 3, 8, hierarchy);
        cv::putText(frame, theObjects.at(i).getType(), cv::Point(theObjects.at(i).getXPos(), theObjects.at(i).getYPos() - 20), 1, 2, theObjects.at(i).getColor());
    }
}

/**
 * @brief This function is used to erode and dilate original image
 * The objetive with this two functions is to remove noise mainly, and isolate the object
 * https://docs.opencv.org/4.x/db/df6/tutorial_erosion_dilatation.html
 * 
 * @param thresh 
 */
void morphologicalOperations(Mat &thresh) {
    Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
    erode(thresh, thresh, erodeElement);
    erode(thresh, thresh, erodeElement);

    Mat dilateElement = getStructuringElement(MORPH_RECT, Size(9, 9));
    dilate(thresh, thresh, dilateElement);
    dilate(thresh, thresh, dilateElement);
}

/**
 * @brief This function is used to track the object throw the image (video capture)
 * ! ---------------------------------------------------------------------------
 * ! Some parts of it are not well implemented, we need to take a loot after...
 * ! ----------------------------------------------------------------------------
 * @param theObject 
 * @param threshold 
 * @param HSV 
 * @param cameraFeed 
 */
void trackFilteredObject(Object theObject, Mat threshold, Mat HSV, Mat &cameraFeed) {
    // TODO: see if the contours problem have something related with this function 
    vector<Object> objects;
    Mat temp;
    threshold.copyTo(temp);
    // declare the following vectors to use in findContours as output 
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(temp, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0) {
        int numObjects = hierarchy.size();
        //! if numObjects > MAX_NUM_OBJECTS probably there are much noise in the image
        //! find a way to resolve that situation
        //* https://livecodestream.dev/post/object-tracking-with-opencv/
        //* https://www.pyimagesearch.com/2018/07/30/opencv-object-tracking/
        if (numObjects < MAX_NUM_OBJECTS) {
            for (int index = 0; index >= 0; index = hierarchy[index][0]) {
                // use moments method to find our filtered object
                Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;
                if (area > MIN_OBJECT_AREA) {
                    Object object;
                    object.setXPos(moment.m10 / area);
                    object.setYPos(moment.m01 / area);
                    object.setType(theObject.getType());
                    object.setColor(theObject.getColor());
                    objects.push_back(object);
                    objectFound = true;
                } else
                    objectFound = false;
            }
            // if object found, draw the contours around it
            if (objectFound == true) {
                // draw object location on screen
                drawObject(objects, cameraFeed, temp, contours, hierarchy);
            }
        } else
            putText(cameraFeed, "A LOT OF OBJECTS ON IMAGE", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
    }
}

int main(int argc, char *argv[]) {
    Object blue("blue"), yellow("yellow"), red("red"), green("green");
    Mat cameraFeed, threshold, HSV;
    VideoCapture capture;
    capture.open(0);
    // infinite loop, searching for the objects and displaying the result in the window named "Image"
    while (1) {
        // store image on cameraFeed
        capture.read(cameraFeed);
        // convert BGR to HSV colorspace
        cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
        // BLUE
        inRange(HSV, blue.getHSVmin(), blue.getHSVmax(), threshold);
        morphologicalOperations(threshold);
        trackFilteredObject(blue, threshold, HSV, cameraFeed);
        // YELLOW
        inRange(HSV, yellow.getHSVmin(), yellow.getHSVmax(), threshold);
        morphologicalOperations(threshold);
        trackFilteredObject(yellow, threshold, HSV, cameraFeed);
        // RED
        inRange(HSV, red.getHSVmin(), red.getHSVmax(), threshold);
        morphologicalOperations(threshold);
        trackFilteredObject(red, threshold, HSV, cameraFeed);
        // GREEN
        inRange(HSV, green.getHSVmin(), green.getHSVmax(), threshold);
        morphologicalOperations(threshold);
        trackFilteredObject(green, threshold, HSV, cameraFeed);

        // imshow("windowName2",threshold);
        // imshow("windowName1", HSV);
        imshow("Image", cameraFeed);

        waitKey(80);
    }
    return 0;
}