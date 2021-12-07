#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat redFilter(const Mat& src)
{
    assert(src.type() == CV_8UC3);

    Mat redOnly;
    inRange(src, Scalar(0, 0, 0), Scalar(0, 255, 0), redOnly);

    Mat greenOnly;
    inRange(src, Scalar(0, 0, 0), Scalar(0, 255, 0), greenOnly);

    return redOnly;
}

int main(int argc, char** argv)
{
    Mat input = imread("../../images/123.png");

    imshow("input", input);
    waitKey();

    Mat redOnly = redFilter(input);

    imshow("redOnly", redOnly);
    waitKey();

    // detect squares after filtering...

    return 0;
}