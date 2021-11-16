#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

using std::cin;
using std::cout;
using std::endl;

using namespace cv;

int main() {
    std::string image_path = "../images/lego2.png";
    Mat img = imread(image_path, IMREAD_COLOR);

    if (img.empty()) {
        std::cout << "Could not read the image: " << image_path << std::endl;
        return 1;
    }

	Mat new_image = Mat::zeros( img.size(), img.type());

	    double alpha = 1.0; /*< Simple contrast control */
    int beta = 0;       /*< Simple brightness control */
    cout << " Basic Linear Transforms " << endl;
    cout << "-------------------------" << endl;
    cout << "* Enter the alpha value [1.0-3.0]: "; cin >> alpha;
    cout << "* Enter the beta value [0-100]: ";    cin >> beta;
    for( int y = 0; y < img.rows; y++ ) {
        for( int x = 0; x < img.cols; x++ ) {
            for( int c = 0; c < img.channels(); c++ ) {
                new_image.at<Vec3b>(y,x)[c] =
                  saturate_cast<uchar>( alpha*img.at<Vec3b>(y,x)[c] + beta );
            }
        }
    }
    imshow("Original Image", img);
    imshow("New Image", new_image);


    imshow("Display window", img);
    int k = waitKey(0);  // Wait for a keystroke in the window

    if (k == 's') {
        imwrite("starry_night.png", img);
    }

    return 0;
}