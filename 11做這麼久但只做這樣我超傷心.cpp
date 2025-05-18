#include <opencv2/opencv.hpp>
using namespace cv;

bool freeze = false;
Rect buttonRect;
Mat baseFrame;

int brightness = 50;
int contrast = 50;
int thresh = 127;
int mode = 0;
int prev_mode = -1;

bool initialized = false;
int setColorTarget = 0;

Vec3b blackColor1(0, 0, 0), blackColor2(0, 0, 0);
Vec3b whiteColor1(255, 255, 255), whiteColor2(255, 255, 255);
bool blackColor1Set = false, blackColor2Set = false;
bool whiteColor1Set = false, whiteColor2Set = false;

int r = 0, g = 0, b = 0;

void mouseCallback(int event, int x, int y, int flags, void*) {
    if (event == EVENT_LBUTTONDOWN && !freeze) {
        if (buttonRect.contains(Point(x, y))) {
            freeze = true;
        }
    }
}

void on_trackbar(int, void*) {
    Mat color(300, 300, CV_8UC3, Scalar(b, g, r));
    imshow("Color Mixer", color);
}

Vec3b getCurrentColor() {
    return Vec3b(b, g, r);
}

Mat adjustBrightness(const Mat& src, int brightness) {
    int bright = brightness - 50;
    Mat result;
    src.convertTo(result, -1, 1.0, bright);
    return result;
}

Mat adjustContrast(const Mat& src, int contrast) {
    double con = contrast / 50.0;
    Mat result;
    src.convertTo(result, -1, con, 0);
    return result;
}

Mat NOT(const Mat& src) {
    Mat result;
    bitwise_not(src, result);
    return result;
}

Mat THRESH(const Mat& src, int thresh) {
    Mat result, gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);
    threshold(gray, result, thresh, 255, 0);
    return result;
}

Mat GRAD(const Mat& src, int thresh) {
    Mat gray, dst;
    cvtColor(src, gray, COLOR_BGR2GRAY);
    threshold(gray, dst, thresh, 255, 0);

    if (!(blackColor1Set && blackColor2Set) && !(whiteColor1Set && whiteColor2Set)) {
        Mat binBGR;
        cvtColor(dst, binBGR, COLOR_GRAY2BGR);
        return binBGR;
    }

    Mat result = src.clone();
    int height = dst.rows;

    for (int row = 0; row < height; row++) {
        float alpha = static_cast<float>(row) / (height - 1);
        Vec3b gradBlack, gradWhite;

        for (int ch = 0; ch < 3; ch++) {
            gradBlack[ch] = static_cast<uchar>((1 - alpha) * blackColor1[ch] + alpha * blackColor2[ch]);
            gradWhite[ch] = static_cast<uchar>((1 - alpha) * whiteColor1[ch] + alpha * whiteColor2[ch]);
        }

        for (int col = 0; col < dst.cols; col++) {
            if (dst.at<uchar>(row, col) == 0 && blackColor1Set && blackColor2Set) {
                result.at<Vec3b>(row, col) = gradBlack;
            }
            else if (dst.at<uchar>(row, col) != 0 && whiteColor1Set && whiteColor2Set) {
                result.at<Vec3b>(row, col) = gradWhite;
            }
            else {
                result.at<Vec3b>(row, col) = Vec3b(dst.at<uchar>(row, col)); // fallback to gray
            }
        }
    }

    return result;
}

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) return -1;

    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    namedWindow("Portrait 9:16 Crop");
    setMouseCallback("Portrait 9:16 Crop", mouseCallback);

    createTrackbar("Brightness", "Portrait 9:16 Crop", &brightness, 100, nullptr);
    createTrackbar("Contrast", "Portrait 9:16 Crop", &contrast, 100, nullptr);
    createTrackbar("Thresh", "Portrait 9:16 Crop", &thresh, 255, nullptr);

    while (true) {
        Mat frame;
        if (!freeze) {
            cap >> frame;
            if (frame.empty()) break;

            int w = frame.cols, h = frame.rows;
            int crop_h = h;
            int crop_w = crop_h * 9 / 16;
            if (crop_w > w) {
                crop_w = w;
                crop_h = crop_w * 16 / 9;
            }
            int x = (w - crop_w) / 2;
            int y = (h - crop_h) / 2;

            baseFrame = frame(Rect(x, y, crop_w, crop_h)).clone();
        }

        Mat display = baseFrame.clone();

        if (mode == 1) {
            display = adjustBrightness(display, brightness);
        }
        else if (mode == 2) {
            display = adjustContrast(display, contrast);
        }
        else if (mode == 3) {
            display = NOT(display);
        }
        else if (mode == 4) {
            display = THRESH(display, thresh);
        }
        else if (mode == 5) {
            if (!initialized) {
                namedWindow("Color Mixer", WINDOW_AUTOSIZE);
                createTrackbar("Red", "Color Mixer", &r, 255, on_trackbar);
                createTrackbar("Green", "Color Mixer", &g, 255, on_trackbar);
                createTrackbar("Blue", "Color Mixer", &b, 255, on_trackbar);
                on_trackbar(0, 0);
                initialized = true;
                setColorTarget = 0;
                std::cout << "按b填入黑色區塊的第一種顏色\n";
                std::cout << "按n填入黑色區塊的第二種顏色\n";
                std::cout << "按w填入白色區塊的第一種顏色\n";
                std::cout << "按m填入白色區塊的第二種顏色\n";
            }

            if (setColorTarget == 1) {
                blackColor1 = getCurrentColor();
                blackColor1Set = true;
                std::cout << "黑色漸層起始色已設定\n";
            }
            else if (setColorTarget == 2) {
                blackColor2 = getCurrentColor();
                blackColor2Set = true;
                std::cout << "黑色漸層結束色已設定\n";
            }
            else if (setColorTarget == 3) {
                whiteColor1 = getCurrentColor();
                whiteColor1Set = true;
                std::cout << "白色漸層起始色已設定\n";
            }
            else if (setColorTarget == 4) {
                whiteColor2 = getCurrentColor();
                whiteColor2Set = true;
                std::cout << "白色漸層結束色已設定\n";
            }

            setColorTarget = 0;
            display = GRAD(display, thresh);
        }

        if (prev_mode == 5 && mode != 5) {
            destroyWindow("Color Mixer");
            initialized = false;
        }
        prev_mode = mode;

        if (!freeze) {
            int r = 30;
            Point center(display.cols / 2, display.rows - r - 10);
            circle(display, center, r, Scalar(255, 255, 255), FILLED);
            buttonRect = Rect(center.x - r, center.y - r, r * 2, r * 2);
        }

        imshow("Portrait 9:16 Crop", display);

        char key = waitKey(30);
        if (key == 27) break;
        else if (key >= '0' && key <= '5') {
            mode = key - '0';
        }
        else if (mode == 5 && key == 'b') {
            setColorTarget = 1; // 黑起始
        }
        else if (mode == 5 && key == 'n') {
            setColorTarget = 2; // 黑結束
        }
        else if (mode == 5 && key == 'w') {
            setColorTarget = 3; // 白起始
        }
        else if (mode == 5 && key == 'm') {
            setColorTarget = 4; // 白結束
        }
    }

    return 0;
}
