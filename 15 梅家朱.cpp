#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <sstream>

using namespace cv;
using namespace std;

// �����ܼ�
bool freeze = false;        // �O�_����e��
Rect buttonRect;            // ��Ϋ��s�ϰ� (����/��_)
Rect importButtonRect;      // �פJ�Ϥ����s�ϰ�
Mat importedImage;          // �פJ���Ϥ�
Mat capturedFrame;          // �x�s��Ӫ��e��
bool showButtons = true;    // �O�_��ܫ��s
VideoCapture cap(0);        // �N VideoCapture �]�������ܼ�
int saveCounter = 1;        // �x�s�p�ƾ�
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

// �ƹ��^�I���
void mouseCallback(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN) {
        Point clickPoint(x, y);

        // ����/��_���s
        if (showButtons && buttonRect.contains(clickPoint) && importedImage.empty()) {
            freeze = !freeze; // �����ᵲ���A
            cout << (freeze ? "�e���w�ᵲ�C" : "�e���w��_�C") << endl;
            showButtons = !freeze; // �P�B��s showButtons �����A
            if (freeze && capturedFrame.empty()) {
                // �b�ᵲ�ɫO�s��e�e��
                Mat tempFrame;
                cap >> tempFrame;
                if (!tempFrame.empty()) {
                    int frame_width = tempFrame.cols;
                    int frame_height = tempFrame.rows;
                    int crop_height = frame_height;
                    int crop_width = crop_height * 9 / 16;
                    if (crop_width > frame_width) {
                        crop_width = frame_width;
                        crop_height = crop_width * 16 / 9;
                    }
                    int x_offset = (frame_width - crop_width) / 2;
                    int y_offset = (frame_height - crop_height) / 2;
                    Rect roi(x_offset, y_offset, crop_width, crop_height);
                    capturedFrame = tempFrame(roi).clone();
                }
                else {
                    cout << "ĵ�i�G�ᵲ�ɵL�kŪ���e���C" << endl;
                    freeze = false; // �p�G�L�kŪ���A�h�����ᵲ
                    showButtons = true; // �����ᵲ�ɭ��s��ܫ��s
                }
            }
        }

        // �פJ�Ϥ����s
        if (showButtons && importButtonRect.contains(clickPoint) && importedImage.empty()) {
            string path;
            cout << "�п�J�Ϥ����|�G";
            cin >> path;
            Mat img = imread(path);
            if (!img.empty()) {
                cout << "Ū���Ϥ����\�A�ؤo���G" << img.cols << "x" << img.rows << endl;

                Size targetSize(360, 640); // �w�]�e���j�p
                Mat processed;

                if (img.cols > img.rows) {
                    // ��Ӥ��G������Y��e�׬� 360�A���צ۰ʽվ�
                    float scale = (float)targetSize.width / img.cols;
                    int newHeight = img.rows * scale;
                    resize(img, processed, Size(targetSize.width, newHeight));
                    Mat background = Mat::zeros(targetSize, img.type());
                    int y_offset = (targetSize.height - newHeight) / 2;
                    processed.copyTo(background(Rect(0, y_offset, targetSize.width, newHeight)));
                    importedImage = background;
                    cout << "�o�O��Ӥ��A�w�m����ܡC" << endl;
                }
                else {
                    // �����Υ���ηӤ��G������Y�񰪫׬� 640�A�e�ץi��W�L
                    float scale = (float)targetSize.height / img.rows;
                    int newWidth = img.cols * scale;
                    resize(img, processed, Size(newWidth, targetSize.height));
                    int x_offset = (newWidth - targetSize.width) / 2;
                    Rect cropRegion(x_offset, 0, targetSize.width, targetSize.height);
                    importedImage = processed(cropRegion).clone();
                    cout << "�o�O�����Υ���ηӤ��A�w�����m����ܡC" << endl;
                }
                freeze = true;
                showButtons = false; // �פJ�Ϥ���]���ë��s
                cout << "�Ϥ��פJ���\�I" << endl;
            }
            else {
                cout << "�L�kŪ���Ϥ��A�нT�{���|�O�_���T�C" << endl;
            }
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
    if (!cap.isOpened()) return -1;

    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    namedWindow("Portrait 9:16 Crop", WINDOW_AUTOSIZE);
    resizeWindow("Portrait 9:16 Crop", 360, 640);        // �]�w�����ؤo
    moveWindow("Portrait 9:16 Crop", 100, 100);         // �N�������ʨ�e����m�A�קK�Q�B��
    setMouseCallback("Portrait 9:16 Crop", mouseCallback); // �]�w�ƹ��^�I

    createTrackbar("Brightness", "Portrait 9:16 Crop", &brightness, 100, nullptr);
    createTrackbar("Contrast", "Portrait 9:16 Crop", &contrast, 100, nullptr);
    createTrackbar("Thresh", "Portrait 9:16 Crop", &thresh, 255, nullptr);

    Mat frame;
    Mat lastFrame;

    while (true) {
        if (!freeze && importedImage.empty()) { // �u���b���ᵲ�B�S���פJ�Ϥ��ɤ~Ū����v��
            cap >> frame;
            if (frame.empty()) break;

            // --- 9:16 ���� ---
            /*����v�������e*/
            int frame_width = frame.cols;
            int frame_height = frame.rows;

            /*��ܪ����e*/
            int crop_height = frame_height;
            int crop_width = crop_height * 9 / 16;
            /*�ˬd�s���e�׬O�_�W�L����v�������e�A�p�G�W�L�N��e���A�A���s�p��*/
            if (crop_width > frame_width) {
                crop_width = frame_width;
                crop_height = crop_width * 16 / 9;
            }
            /*�p��X����v����ܵe�����W�����y��*/
            int x = (frame_width - crop_width) / 2;
            int y = (frame_height - crop_height) / 2;

            /*�w�q�X�n�������d��*/
            Rect roi(x, y, crop_width, crop_height);
            frame = frame(roi).clone();

            lastFrame = frame.clone(); // �O�s��e�e��
        }

        Mat display;
        if (!importedImage.empty()) {
            display = importedImage.clone(); // ��ܶפJ���Ϥ�
        }
        else {
            display = lastFrame.clone();        // �����v���e���έᵲ���e��
        }
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
                std::cout << "��b��J�¦�϶����Ĥ@���C��\n";
                std::cout << "��n��J�¦�϶����ĤG���C��\n";
                std::cout << "��w��J�զ�϶����Ĥ@���C��\n";
                std::cout << "��m��J�զ�϶����ĤG���C��\n";
            }

            if (setColorTarget == 1) {
                blackColor1 = getCurrentColor();
                blackColor1Set = true;
                std::cout << "�¦⺥�h�_�l��w�]�w\n";
            }
            else if (setColorTarget == 2) {
                blackColor2 = getCurrentColor();
                blackColor2Set = true;
                std::cout << "�¦⺥�h������w�]�w\n";
            }
            else if (setColorTarget == 3) {
                whiteColor1 = getCurrentColor();
                whiteColor1Set = true;
                std::cout << "�զ⺥�h�_�l��w�]�w\n";
            }
            else if (setColorTarget == 4) {
                whiteColor2 = getCurrentColor();
                whiteColor2Set = true;
                std::cout << "�զ⺥�h������w�]�w\n";
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
    

        // �u���b showButtons �� true �B�S���פJ�Ϥ��ɤ~ø�s���s
        if (showButtons && importedImage.empty()) {
            int button_radius = 30;
            Point button_center(display.cols / 2, display.rows - button_radius - 10);
            circle(display, button_center, button_radius, Scalar(255, 255, 255), FILLED);
            //�����ƹ��I���d��(�������d��O����]�t����Ϊ��̤p��ΡA�ҥH�I������񪺪ťճB�٬O�|�ᵲ�e��)
            //rect(�̥��W����x�y��,�̥��W����y�y��,�x�μe��,�x�ΰ���)
            buttonRect = Rect(button_center.x - button_radius, button_center.y - button_radius, button_radius * 2, button_radius * 2);

            int square_size = 25;
            //�̥��W���Z�������10�A�������W���+10
            Point import_top_left(10, display.rows - square_size - 10);
            rectangle(display, Rect(import_top_left.x, import_top_left.y, square_size, square_size), Scalar(255, 255, 255), 2);
            importButtonRect = Rect(import_top_left.x, import_top_left.y, square_size, square_size);
        }
        else {
            // �p�G���פJ�Ϥ��A�h��ø�s���s�A�ñN���s�ϰ�]�w���L��
            buttonRect = Rect();
            importButtonRect = Rect();
        }

        imshow("Portrait 9:16 Crop", display);

        char key = waitKey(30);
        if (key == 27) { // ESC
            cout << "�h�X�{��" << endl;
            break;
        }
        else if (key >= '0' && key <= '5') {
            mode = key - '0';
        }
        else if (mode == 5 && key == 'b') {
            setColorTarget = 1; // �°_�l
        }
        else if (mode == 5 && key == 'n') {
            setColorTarget = 2; // �µ���
        }
        else if (mode == 5 && key == 'w') {
            setColorTarget = 3; // �հ_�l
        }
        else if (mode == 5 && key == 'm') {
            setColorTarget = 4; // �յ���
        }
        else if (key == 82 || key == 114) { // R or r
            importedImage.release();
            capturedFrame.release();
            freeze = false;
            showButtons = true; // ���]����ܫ��s
            cout << "�w���]�Ҧ��Ϥ��A��_�Y����v���e���C" << endl;
        }
        else if (key == 83 || key == 115) { // S or s
            Mat imageToSave;
            string filename;
            if (!importedImage.empty()) {
                imageToSave = importedImage;
                stringstream filenameSS;
                filenameSS << "image_" << saveCounter << ".jpg"; // �ϥάۦP���ɦW�榡�M�p�ƾ�
                filename = filenameSS.str();
                cout << "�N�פJ���Ϥ��x�s���G" << filename << endl;
                saveCounter++; // ���W�p�ƾ�
            }
            else if (freeze && !capturedFrame.empty()) {
                imageToSave = capturedFrame;
                stringstream filenameSS;
                filenameSS << "image_" << saveCounter << ".jpg"; // �ϥάۦP���ɦW�榡�M�p�ƾ�
                filename = filenameSS.str();
                cout << "�w�x�s�ᵲ���e�����G" << filename << endl;
                saveCounter++; // ���W�p�ƾ�
            }
            else {
                cout << "�ثe�S���i�x�s���Ϥ� (�S���פJ�Ϥ��B�e�����ᵲ)�C" << endl;
                continue;
            }
            if (!imageToSave.empty()) {
                if (imwrite(filename, imageToSave)) {
                    cout << "�Ϥ��w�x�s���G" << filename << endl;
                }
                else {
                    cout << "�x�s�Ϥ����ѡC" << endl;
                }
            }
        }
    }
    return 0;
}