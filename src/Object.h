#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>

using namespace std;
using namespace cv;

class Object {
   public:
    Object();
    ~Object(void);

    Object(string name);
    Object(string name, Scalar hsvMin, Scalar hsvMax, int r, int g, int b) {
        this->type = name;
        this->HSVmin = hsvMin;
        this->HSVmax = hsvMax;
        this->Color = Scalar(r, g, b);
    };

    int getXPos();
    void setXPos(int x);

    int getYPos();
    void setYPos(int y);

    Scalar getHSVmin();
    Scalar getHSVmax();

    void setHSVmin(Scalar min);
    void setHSVmax(Scalar max);

    string getType() { return type; }
    void setType(string t) { type = t; }

    Scalar getColor() {
        return Color;
    }
    void setColor(Scalar c) {
        Color = c;
    }
    friend ostream& operator<<(ostream& os, const Object& o);
    friend istream &operator>>(istream &input, Object &o) {
        input >> o.type;
        return input;
    }
    

   private:
    int xPos, yPos;
    string type;
    Scalar HSVmin, HSVmax;
    Scalar Color;
};
ostream &operator<<(ostream &os, const Object &o) {
        os << o.type << " - " << o.HSVmin << " - " << o.HSVmax << " - " << o.Color << endl;
        return os;
    }
