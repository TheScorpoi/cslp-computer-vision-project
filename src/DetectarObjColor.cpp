#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <list>
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
const int MAX_NUM_OBJECTS = 20;
const int MIN_OBJECT_AREA = 28 * 28;
const int MAX_OBJECT_AREA = FRAME_HEIGHT * FRAME_WIDTH / 1.5;
double PIXEL_SIZE_WIDTH = 0.0;
double PIXEL_SIZE_HEIGHT = 0.0;
double W = 2.0;
double H = 1.0;
bool IS_CAMERA_CALIBRATED = false;

void setPixelSize(Rect c) {
    // saves the pixel size
    if (W > H && c.width > c.height) {
        PIXEL_SIZE_WIDTH = round(c.width / W);
        PIXEL_SIZE_HEIGHT = round(c.height / H);
    } else {
        PIXEL_SIZE_WIDTH = round(c.width / H);
        PIXEL_SIZE_HEIGHT = round(c.height / W);
    }
}

tuple<int, int> get_size_to_detection_fuction(Rect c) {
    int w, h;
    if (PIXEL_SIZE_HEIGHT == 0.0 && PIXEL_SIZE_WIDTH == 0.0) {
        setPixelSize(c);
    }
    if (PIXEL_SIZE_WIDTH > PIXEL_SIZE_HEIGHT) {
        w = c.width / PIXEL_SIZE_WIDTH;
        h = c.height / PIXEL_SIZE_HEIGHT;
    } else {
        w = c.width / PIXEL_SIZE_HEIGHT;
        h = c.height / PIXEL_SIZE_WIDTH;
    }

    return {w, h};
}


bool calibrate(Rect c, Mat &cameraFeed){
    auto [value1, value2] = get_size_to_detection_fuction(c);
    if (value1 != W && value2 != H) {
        cout << "\rCalibration failed\r" << std::flush;
        //putText(cameraFeed, "Calibration is not correct", Point(10, 30), 1,2, Scalar(0, 0, 250), 2);
        IS_CAMERA_CALIBRATED = false;
        usleep(600);
    } else if  (value1 == W && value2 == H) {
        IS_CAMERA_CALIBRATED = true;
        cout << "Camera Calibrated" << endl;
        
    }
    return IS_CAMERA_CALIBRATED;
}


string get_size(Rect c) {
    int w, h;
    if (PIXEL_SIZE_HEIGHT == 0.0 && PIXEL_SIZE_WIDTH == 0.0) {
        setPixelSize(c);
    }
    if (PIXEL_SIZE_WIDTH > PIXEL_SIZE_HEIGHT) {
        w = c.width / PIXEL_SIZE_WIDTH;
        h = c.height / PIXEL_SIZE_HEIGHT;
    } else {
        w = c.width / PIXEL_SIZE_HEIGHT;
        h = c.height / PIXEL_SIZE_WIDTH;
    }

    string shape = "";
    if (w == h) {
        shape = "square";
    } else {
        shape = "rectangle";
    }

    return to_string(w) + "x" + to_string(h) + " - " + shape;
}

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
void drawObject(vector<Object> theObjects, Mat &frame, Mat &temp, vector<vector<Point>> contours, vector<Vec4i> hierarchy, Rect c) {
    for (int i = 0; i < theObjects.size(); i++) {
        cv::drawContours(frame, contours, i, theObjects.at(i).getColor(), 3, 8, hierarchy);
        cv::putText(frame, theObjects.at(i).getType() + " > " + get_size(c), cv::Point(theObjects.at(i).getXPos(), theObjects.at(i).getYPos() - 20), 1, 2, theObjects.at(i).getColor());
    }
}

/**
 * 
 * @deprecated - Instead of this, it's better calculating the shape on fucntion get_size each time that a object is drawned (this word exists? idk).
 * @brief This function is used to return the object's shape
 *
 * @param c
 * @return string
 */
/*string returnShapeName(Rect c) {
    cout << c.width << " " << c.height << endl;
    cout << endl;
    if (lround(c.width) > lround(c.height)) {
        return "Rectangle";
    } else if (lround(c.width) == lround(c.height)) {
        return "Square";
    } else {
        return "Circle";
    }
}
*/

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
                drawObject(objects, cameraFeed, temp, contours, hierarchy, c);
                if (IS_CAMERA_CALIBRATED == false) {
                    calibrate(c, cameraFeed);
                }
                if (IS_CAMERA_CALIBRATED) {
                    putText(cameraFeed, "......", Point(60, 50), 1, 2, Scalar(0, 128, 0), 6);

                } else {
                    putText(cameraFeed, "......" , Point(60, 50), 1, 2, Scalar(0, 0, 255), 6);
                }
            }
        } else
            putText(cameraFeed, "A LOT OF OBJECTS ON IMAGE", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
    }
}

void updateCalibrationVariables() {
    string filename = "../../output/piece_size.txt";
    ifstream read(filename.c_str());
    read >> W >> H;
    read.close();
}

void detectInRealTime(vector<Object> objects) {
    vector<Object> new_objects = objects;
    Mat cameraFeed, threshold, HSV, blur;
    VideoCapture capture;
    
    updateCalibrationVariables(); // update the calibration variables
    
    capture.open(0);
    // infinite loop, searching for the objects and displaying the result in the window named "Image"
    while (1) {
        // store image on cameraFeed
        capture.read(cameraFeed);

        GaussianBlur(cameraFeed, blur, Size(5, 5), 0, 0);

        // convert BGR to HSV colorspace
        cvtColor(blur, HSV, COLOR_BGR2HSV);

        for (Object obj : new_objects) {
            cout << obj.getType() << " -> "<<obj.getHSVmin() << " " << obj.getHSVmax() << obj.getColor() << endl;
            inRange(HSV, obj.getHSVmin(), obj.getHSVmax(), threshold);
            morphologicalOperations(threshold);
            trackFilteredObject(obj, threshold, HSV, cameraFeed);
        }
        imshow("Image", cameraFeed);

        waitKey(80);  // taxa de refresh da camara, > menor taxa, < maior taxa
    }
}

void detectInRealTime() {
    Object blue("blue"), yellow("yellow"), red("red"), green("green");
    Object new_objects[] = {Object("blue"), Object("yellow"), Object("red"), Object("green")};
    Mat cameraFeed, threshold, HSV, blur;
    VideoCapture capture;

    updateCalibrationVariables(); // update the calibration variables

    capture.open(0);
    // infinite loop, searching for the objects and displaying the result in the window named "Image"
    while (1) {
        // store image on cameraFeed
        capture.read(cameraFeed);

        GaussianBlur(cameraFeed, blur, Size(5, 5), 0, 0);

        // convert BGR to HSV colorspace
        cvtColor(blur, HSV, COLOR_BGR2HSV);

        for (Object obj : new_objects) {
            cout << obj.getType() << " -> "<<obj.getHSVmin() << " " << obj.getHSVmax() << obj.getColor() << endl;

            inRange(HSV, obj.getHSVmin(), obj.getHSVmax(), threshold);
            morphologicalOperations(threshold);
            trackFilteredObject(obj, threshold, HSV, cameraFeed);
        }
        imshow("Image", cameraFeed);
        waitKey(80);  // taxa de refresh da camara, > menor taxa, < maior taxa
    }
}

void on_mouse_click(int event, int x, int y, int flags, void *ptr) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        cv::Mat *snapshot = (cv::Mat *)ptr;
        cv::Mat &img = *((cv::Mat *)(ptr));
        cv::Vec3b pixel = snapshot->at<cv::Vec3b>(x, y);
        cv::Vec3b bgrPixel(snapshot->at<cv::Vec3b>(y, x));
        circle(*snapshot, cv::Point(x, y), 4, cv::Scalar(0, 0, 255), cv::FILLED, 8, 0);
        cv::imshow("Capture", *snapshot);
        int b, g, r;
        b = pixel[0];
        g = pixel[1];
        r = pixel[2];

        cv::Mat3b bgr(bgrPixel);
        cv::Mat3b hsv;
        cv::Mat3b rgb;
        cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
        cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
        cout << "AQUIIIII" << rgb << endl;

        cv::Vec3b hsvPixel(hsv.at<cv::Vec3b>(0, 0));
        cv::Vec3b rgbPixel(rgb.at<cv::Vec3b>(0, 0));

        std::string rgbText = "[" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + "]";
        cout << "rgb " << rgbPixel << std::endl;
        cout << "hsv " << hsvPixel << endl;

        int thresh = 20;
        cv::Scalar minHSV = cv::Scalar(hsvPixel.val[0] - thresh, hsvPixel.val[1] - thresh, hsvPixel.val[2] - thresh);
        cv::Scalar maxHSV = cv::Scalar(hsvPixel.val[0] + thresh, hsvPixel.val[1] + thresh, hsvPixel.val[2] + thresh);

        cv::Scalar rgbColor = cv::Scalar(rgbPixel.val[0], rgbPixel.val[1], rgbPixel.val[2]);

        string colour_name;
        cout << "Colour Name? ";
        cin >> colour_name;
        string filename("../../output/tmp2.txt");
        ofstream out(filename.c_str(), fstream::app);  // append mode on
        Object ob1(colour_name, minHSV, maxHSV, rgbColor);
        out << ob1;
    }
}

vector<string> split(const string &s, char delim) {
    vector<string> result;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

vector<Object> readFileCreateColous() {
    vector<Object> objects;
    ifstream file;
    file.open("../../output/tmp2.txt");
    string line;
    vector<string> result;
    cout << "Reading file..." << endl;
    string colour_name = "";
    while (getline(file, line)) {
        boost::split(result, line, boost::is_any_of(" - "));

        cv::Scalar hsvMin;
        cv::Scalar hsvMax;
        cv::Scalar rgbColor;
        int r, g, b;

        for (int i = 0; i < result.size(); i++) {
            boost::replace_all(result[i], "[", "");
            boost::replace_all(result[i], "]", "");
            boost::replace_all(result[i], ",", "");

            colour_name = result[0];
            stringstream ss;
            int hmin, smin, vmin;
            int hmax, smax, vmax;
            
            ss << result[3];
            //cout << "result 4: hmin -- " << result[3] << endl;
            ss >> hmin;
            ss.clear();
            ss << result[4];
            //cout << "result 5: smin -- " << result[4] << endl;
            ss >> smin;
            ss.clear();
            ss << result[5];
            //cout << "result 6: vmin -- " << result[5] << endl;
            ss >> vmin;
            ss.clear();
            ss << result[9];
            ss >> hmax;
            ss.clear();
            ss << result[10];
            ss >> smax;
            ss.clear();
            ss << result[11];
            ss >> vmax;
            ss.clear();
            boost::replace_all(result[15], "[", "");
            boost::replace_all(result[15], "]", "");
            boost::replace_all(result[15], ",", "");
            ss << result[15];
            cout << "result 15: r -- " << result[15] << endl;
            ss >> r;
            ss.clear();
            boost::replace_all(result[16], "[", "");
            boost::replace_all(result[16], "]", "");
            boost::replace_all(result[16], ",", "");
            ss << result[16];
            cout << "result 16: g -- " << result[16] << endl;
            ss >> g;
            ss.clear();
            boost::replace_all(result[17], "[", "");
            boost::replace_all(result[17], "]", "");
            boost::replace_all(result[17], ",", "");
            ss << result[17];
            cout << "result 17: b -- " << result[17] << endl;
            ss >> b;
            ss.clear();

            hsvMin = cv::Scalar(hmin, smin, vmin);
            hsvMax = cv::Scalar(hmax, smax, vmax);
            rgbColor = cv::Scalar(b, g, r);
        }
        cout << "rgb AQUIIIIII QUE JA NS ONDE ESTA ESTA MERDA FDS" << rgbColor << endl;
        Object ob(colour_name, hsvMin, hsvMax, rgbColor);
        objects.push_back(ob);
    }
    file.close();
    return objects;
}

void selectColoursWithMouse() {
    cv::VideoCapture capture(0);

    if (!capture.isOpened()) {
        std::cout << "Error opening VideoCapture." << std::endl;
    }

    cv::Mat frame, snapshot;
    capture.read(frame);

    snapshot = cv::Mat(frame.size(), CV_8UC3, cv::Scalar(23, 32, 32));
    cv::imshow("Capture", snapshot);

    cv::setMouseCallback("Capture", on_mouse_click, &snapshot);

    int keyVal;
    while (1) {
        if (!capture.read(frame)) {
            break;
        }
        cv::imshow("Video", frame);

        keyVal = cv::waitKey(1) & 0xFF;
        if (keyVal == 113 || keyVal == 81) {  // q
            destroyAllWindows();
            capture.release();
            vector<Object> new_objects = readFileCreateColous();
            detectInRealTime(new_objects);
            break;
        } else if (keyVal == 115 || keyVal == 83) {  // S ou s
            snapshot = frame.clone();
            cv::imshow("Capture", snapshot);
        }
    }
}

void saveCalibrationOnFile() {
    cout << "Calibrating Camera..." << endl;
    cout << "Please, place a piece bellow the camera!" << endl;
    cout << "Type piece's width: ";
    cin >> W;
    cout << "Type piece's height: ";
    cin >> H;
    string filename = "../../output/piece_size.txt";
    ofstream out(filename.c_str());
    out << W << " " << H;
    out.close();
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
                //add the width and height of a "master" piece to calibrate the camera
                saveCalibrationOnFile();
                break;
            case 'a':
                cout << "Add colours..." << endl;
                selectColoursWithMouse();
                break;
            case 'e':
                detectInRealTime(readFileCreateColous());
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
}
