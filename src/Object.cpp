#include "Object.h"

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"

Object::Object() {
    // set values for default constructor
    setType("Object");
    setColor(Scalar(0, 0, 0));
}

Object::Object(string name) {
    setType(name);

    if (name == "blue") {

        //setHSVmin(Scalar(110, 50, 50));
        //setHSVmax(Scalar(130, 256, 256));

        setHSVmin(Scalar(103, 70, 50));
        setHSVmax(Scalar(115, 255, 255));

        // BGR value for Blue:
        setColor(Scalar(255, 0, 0));

    }
    if (name == "green") {
        setHSVmin(Scalar(40, 100, 100));
        setHSVmax(Scalar(70, 255, 255));

        // BGR value for Green:
        setColor(Scalar(0, 255, 0));
    }
    if (name == "yellow") {
        setHSVmin(Scalar(20, 124, 123));
        setHSVmax(Scalar(30, 256, 256));

        // BGR value for Yellow:
        setColor(Scalar(0, 255, 255));
    }
    if (name == "red") {
        setHSVmin(Scalar(170, 135, 105));
        setHSVmax(Scalar(184, 255, 255));

        // BGR value for Red:
        setColor(Scalar(0, 0, 255));
    }
}

Object::~Object(void) {
}

int Object::getXPos() {
    return Object::xPos;
}

void Object::setXPos(int x) {
    Object::xPos = x;
}

int Object::getYPos() {
    return Object::yPos;
}

void Object::setYPos(int y) {
    Object::yPos = y;
}

Scalar Object::getHSVmin() {
    return Object::HSVmin;
}
Scalar Object::getHSVmax() {
    return Object::HSVmax;
}

void Object::setHSVmin(Scalar min) {
    Object::HSVmin = min;
}

void Object::setHSVmax(Scalar max) {
    Object::HSVmax = max;
}
