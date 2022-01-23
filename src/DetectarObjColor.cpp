#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "Object.cpp"
#include "Object.h"

using namespace std;
using namespace cv;

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

string get_size(Rect c) {
    int w, h;
    w = c.width / 8;
    h = c.height / 8;
    cout << w << h << "\n";
    return w + "x" + h;
}

void drawObject(vector<Object> theObjects, Mat &frame, Mat &temp, vector<vector<Point>> contours, vector<Vec4i> hierarchy, Rect c, string shapeName) {
    for (int i = 0; i < theObjects.size(); i++) {
        cv::drawContours(frame, contours, i, theObjects.at(i).getColor(), 3, 8, hierarchy);
        cv::putText(frame, theObjects.at(i).getType() + " > " + get_size(c) + " - " + shapeName, cv::Point(theObjects.at(i).getXPos(), theObjects.at(i).getYPos() - 20), 1, 2, theObjects.at(i).getColor());
    }
}

/**
 * @brief
 *
 * @param c
 * @return string
 */
string returnShapeName(Rect c) {
    if (lround(c.width) > lround(c.height)) {
        return "Rectangle";
    } else if (lround(c.width) == lround(c.height)) {
        return "Square";
    } else {
        return "Circle";
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

    // Apply the erosion operation
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
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(temp, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
    double refArea = 0;
    bool objectFound = false;
    double area = 0;
    cv::Rect c;
    vector<vector<Point>> hull(contours.size());
    if (hierarchy.size() > 0) {
        int numObjects = hierarchy.size();
        //! if numObjects > MAX_NUM_OBJECTS probably there are much noise in the image
        //! find a way to resolve that situation
        //* https://livecodestream.dev/post/object-tracking-with-opencv/
        //* https://www.pyimagesearch.com/2018/07/30/opencv-object-tracking/
        if (numObjects < MAX_NUM_OBJECTS) {
            for (int index = 0; index >= 0; index = hierarchy[index][0]) {
                // use moments method to find our filtered object
                cv::convexHull(cv::Mat(contours[index]), hull[index], false);
                Moments moment = moments((cv::Mat)contours[index]);
                c = cv::boundingRect((cv::Mat)contours[index]);
                area = moment.m00;
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
                string shapeName = returnShapeName(c);
                // if (hull.size()==4) {
                drawObject(objects, cameraFeed, temp, contours, hierarchy, c, shapeName);
                //}
            }
        } else
            putText(cameraFeed, "A LOT OF OBJECTS ON IMAGE", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
    }
}

void detectInRealTime() {
    Object blue("blue"), yellow("yellow"), red("red"), green("green");
    Object objArray[] = {Object("blue"), Object("yellow"), Object("red"), Object("green")};
    Mat cameraFeed, threshold, HSV, blur;
    VideoCapture capture;
    capture.open(0);
    // infinite loop, searching for the objects and displaying the result in the window named "Image"
    while (1) {
        // store image on cameraFeed
        capture.read(cameraFeed);

        GaussianBlur(cameraFeed, blur, Size(5, 5), 0, 0);

        // convert BGR to HSV colorspace
        cvtColor(blur, HSV, COLOR_BGR2HSV);

        for (Object obj : objArray) {
            inRange(HSV, obj.getHSVmin(), obj.getHSVmax(), threshold);
            morphologicalOperations(threshold);
            trackFilteredObject(obj, threshold, HSV, cameraFeed);
        }
        /* maneira estática
        // BLUE
        // antes de aplicar os operadores morfologicos fazer segmentacao para (por isto a preto e branco)
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
        */
        // imshow("windowName2",threshold);
        // imshow("windowName1", HSV);
        imshow("Image", cameraFeed);

        waitKey(80);  // taxa de refresh da camara, > menor taxa, < maior taxa
    }
}
void on_mouse_click(int event, int x, int y, int flags, void *ptr) {
    ofstream file;
    file.open("../output/file.txt");
    if (event == cv::EVENT_LBUTTONDOWN) {
        cv::Mat *snapshot = (cv::Mat *)ptr;
        cv::Mat &img = *((cv::Mat *)(ptr));
        cv::Vec3b pixel = snapshot->at<cv::Vec3b>(x, y);
        cv::Vec3b bgrPixel(snapshot->at<cv::Vec3b>(y, x));
        circle(*snapshot, cv::Point(x, y), 4, cv::Scalar(0, 0, 255), cv::FILLED, 8, 0);
        cv::imshow("Snapshot", *snapshot);
        int b, g, r;
        b = pixel[0];
        g = pixel[1];
        r = pixel[2];

        cv::Mat3b bgr(bgrPixel);
        cv::Mat3b hsv;
        cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

        cv::Vec3b hsvPixel(hsv.at<cv::Vec3b>(0, 0));

        std::string rgbText = "[" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + "]";
        cout << "rgb " << rgbText << std::endl;
        cout << "hsv " << hsvPixel << endl;

        int thresh = 40;
        cv::Scalar minHSV = cv::Scalar(hsvPixel.val[0] - thresh, hsvPixel.val[1] - thresh, hsvPixel.val[2] - thresh);
        cv::Scalar maxHSV = cv::Scalar(hsvPixel.val[0] + thresh, hsvPixel.val[1] + thresh, hsvPixel.val[2] + thresh);
        cv::imshow("Snapshot", *snapshot);

        string colour_name;
        cout << "Colour Name? ";
        cin >> colour_name;
        
        file << colour_name << " - " << rgbText << " - " << hsvPixel << " - " << minHSV << " - " << maxHSV << endl;
        file << "------------------------------" << endl;
        file.close();
        // guardar num ficheiro os hsvmin hsvmax o rgb e pedir um nome para a cor
        // cout << minHSV << endl;
        // cout << maxHSV << endl;
    }
}

void readFileCreateColous() {
    ifstream file;
    file.open("../output/file.txt");
    string line;
    while (getline(file, line)) {
        cout << line << endl;
    }
    file.close();

    //objetos com as informacoes das cores que estao no ficheiro
    //po-los num array em q a main consiga aceder aos objetos
    //correr normalmente


}

void selectColoursWithMouse() {
    cv::VideoCapture capture(0);

    if (!capture.isOpened()) {
        std::cout << "Error opening VideoCapture." << std::endl;
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
}

    int main(int argc, char *argv[]) {
        int c;
        int digit_optind = 0;
        int option_index = 0;
        struct option long_options[] = {
            {"calibrate", required_argument, 0, 'c'},
            {"add_colors", no_argument, 0, 'a'},
            {"exit", no_argument, 0, 'e'}};

        while ((c = getopt_long(argc, argv, "caen", long_options, &option_index)) != -1) {
            int this_option_optind = optind ? optind : 1;
            switch (c) {
                case 'c':
                    // opcao para calibrar a camara
                    cout << "Calibrating Camera" << endl;
                    // chamar aqui a funcao que calibra a camara
                    break;
                case 'a':
                    cout << "Add colours..." << endl;
                    // opcao para adicionar cores
                    selectColoursWithMouse();
                    // chamar a funcao que adiciona novas cores
                    break;
                case 'e':
                    readFileCreateColous();
                    cout << "Exiting..." << endl;
                    exit(EXIT_SUCCESS);
                case 'n':
                    cout << "Detect LEGO pieces in Real Time" << endl;
                    detectInRealTime();
                    break;
                default:
                    cout << "USAGE: " << endl;
            }
        }

        if (optind < argc) {
            printf("non-option ARGV-elements: ");
            while (optind < argc)
                printf("%s ", argv[optind++]);
            printf("\n");
        }

        exit(EXIT_SUCCESS);
    }