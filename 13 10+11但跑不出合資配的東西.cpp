#include <opencv2/opencv.hpp>
#include<iostream>
#include <string>
#include <vector>
#include <sstream>
using namespace std;
using namespace cv;

//TAKE PHOTO
Mat display;
Mat lastFrame;
bool freeze = false; // �O�_����e��
bool Aa = false;
Rect buttonRect;// ���s�ϰ� (�ΨӧP�_�I��)
//WORD
Rect buttonAa;
int a = 1;
string sentence;
vector <string> words;
string word;
bool textEntered = false;  // �[�@�ӺX�Шӱ���u��J�@��
int AaStage = 0;
Point textPosition(5, 75);  // ��l��r��m
bool draggingText = false;
Point dragOffset;
vector<string> lines;
//PAINT
Rect buttonPaint;
Rect buttonColor, buttonColor1, buttonColor2, buttonColor3, buttonColor4, buttonColor5;
Point drawing_point(-1, -1);
bool drawing = false;
bool draw = false;
int buttom_radius = 15;
int r = 255, g = 255, b = 255;
int R_color[6] = { 0, 255, 255, 0, 255, 0 };
int G_color[6] = { 0, 0, 255, 0, 255, 0 };
int B_color[6] = { 0, 0, 0, 255, 255, 0 };
bool pickColorMode = false;
int penThickness = 2;
bool paintWindowCreated = false;
int brush = 1;
int wait = 0;
Rect paintMode1, paintMode2, paintMode3;
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

// �ƹ��^�I���
// �Ū� MouseCallback�]��l�ϥΡ^
void emptyCallback(int event, int x, int y, int flags, void* userdata) {
    // ��������Ʊ�
}

void mouseCallback(int event, int x, int y, int flags, void* userdata) {
    if (!freeze) {
        if (event == EVENT_LBUTTONDOWN && buttonRect.contains(Point(x, y))) {
            freeze = true;
        }
        return;
    }
    // freeze ���A�U�G
    Point mousePt(x, y);
    if (Aa && textEntered) {
        // �� lines ��C�@���r�إX�@�Ӥj�d��
        int totalHeight = lines.size() * 30;  // �ʦ��@�氪
        Rect textBox(textPosition.x - 10, textPosition.y - 30, 400, totalHeight);

        if (event == EVENT_LBUTTONDOWN && textBox.contains(Point(x, y))) {
            draggingText = true;
            dragOffset = Point(x, y) - textPosition;
        }
        else if (event == EVENT_LBUTTONUP) {
            draggingText = false;
        }
        else if (event == EVENT_MOUSEMOVE && draggingText) {
            textPosition = Point(x, y) - dragOffset;
        }
    }
    if (draw) {
        // �e�ϼҦ�
        if (event == EVENT_LBUTTONDOWN) {
            drawing = true;
            drawing_point = Point(x, y);
        }
        else if (event == EVENT_MOUSEMOVE && drawing) {
            if (brush == 1) {
                line(lastFrame, drawing_point, Point(x, y), Scalar(B_color[0], G_color[0], R_color[0]), penThickness);
            }
            else if (brush == 2) {
                rectangle(lastFrame, Point(x - (penThickness / 2), y - (penThickness / 2)), Point(x + (penThickness / 2), y + (penThickness / 2)), Scalar(B_color[0], G_color[0], R_color[0]), FILLED);

            }
            else if (brush == 3) {
                wait = (wait + 1) % (penThickness);
                if (wait < (penThickness / 2)) {
                    circle(lastFrame, drawing_point, (penThickness / 2), Scalar(B_color[0], G_color[0], R_color[0]), FILLED);
                }
            }
            drawing_point = Point(x, y);
        }
        else if (event == EVENT_LBUTTONUP) {
            drawing = false;
        }
        //���o�I���m���C��
        if (pickColorMode && event == EVENT_LBUTTONDOWN) {
            if (x >= 0 && x < lastFrame.cols && y >= 0 && y < lastFrame.rows) {
                Vec3b color = lastFrame.at<Vec3b>(y, x);
                R_color[0] = color[2];
                G_color[0] = color[1];
                B_color[0] = color[0];
                pickColorMode = false;
            }
            return;
        }
        if (buttonColor.contains(Point(x, y))) {
            pickColorMode = true;
            return;
        }
    }
    // �\����Ҧ�
    if (event == EVENT_LBUTTONDOWN) {
        if (buttonAa.contains(Point(x, y))) {
            AaStage = (AaStage + 1) % 3;
            Aa = true;
        }
        if (buttonPaint.contains(Point(x, y))) {
            draw = !draw;
            drawing = false;
            // ���n�A�I�s setMouseCallback�A�o�˷|�@���л\
        }
        if (buttonColor1.contains(Point(x, y))) {
            R_color[0] = R_color[1], G_color[0] = G_color[1], B_color[0] = B_color[1];
        }
        if (buttonColor2.contains(Point(x, y))) {
            R_color[0] = R_color[2], G_color[0] = G_color[2], B_color[0] = B_color[2];
        }
        if (buttonColor3.contains(Point(x, y))) {
            R_color[0] = R_color[3], G_color[0] = G_color[3], B_color[0] = B_color[3];
        }
        if (buttonColor4.contains(Point(x, y))) {
            R_color[0] = R_color[4], G_color[0] = G_color[4], B_color[0] = B_color[4];
        }
        if (buttonColor5.contains(Point(x, y))) {
            R_color[0] = R_color[5], G_color[0] = G_color[5], b = B_color[5];
        }
        if (paintMode1.contains(Point(x, y))) {
            brush = 1;
        }
        if (paintMode2.contains(Point(x, y))) {
            brush = 2;
        }
        if (paintMode3.contains(Point(x, y))) {
            brush = 3;
        }
    }
}
void putAa(int, void*) {
    double AaScale = a / 2.0;
    int font = FONT_HERSHEY_SIMPLEX;
    int margin = 5;
    int maxLineWidth = display.cols - 2 * margin;

    int baseline = 0;
    int y = textPosition.y;
    lines.clear();  // �M�ŭ��Ʀ��r
    string currentLine;

    for (size_t i = 0; i < words.size(); ++i) {
        string testLine = currentLine.empty() ? words[i] : currentLine + " " + words[i];
        Size testSize = getTextSize(testLine, font, AaScale, AaScale, &baseline);
        if (testSize.width > maxLineWidth && !currentLine.empty()) {
            lines.push_back(currentLine);
            currentLine = words[i]; // ���s��
        }
        else {
            currentLine = testLine;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);

    // �e�C�@��A�ھ� AaStage ����A�� y �q textPosition.y �}�l
    for (const string& line : lines) {
        Size textSize = getTextSize(line, font, AaScale, AaScale, &baseline);
        int x;
        if (AaStage == 0) {
            x = textPosition.x + (maxLineWidth - textSize.width);  // �k���
        }
        else if (AaStage == 1) {
            x = textPosition.x;  // �����
        }
        else if (AaStage == 2) {
            x = textPosition.x + (maxLineWidth - textSize.width) / 2;  // �m��
        }

        putText(display, line, Point(x, y), font, AaScale, Scalar(255, 255, 255), AaScale);
        y += textSize.height + 10;
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
    /*int r = 0, g = 0, b = 0;*/
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

    std::cout << "\n��1�վ�G��\n��2�վ����\n��3�t���ĪG\n��4�G��ƮĪG\n��5���h�ĪG\n";
    while (true) {
        Mat frame;
        if (!freeze) {
            cap >> frame;
            if (frame.empty()) break;

            // --- 9:16 ���� ---
            int frame_width = frame.cols;
            int frame_height = frame.rows;

            int crop_height = frame_height;
            int crop_width = crop_height * 9 / 16;
            if (crop_width > frame_width) {
                crop_width = frame_width;
                crop_height = crop_width * 16 / 9;
            }

            int x = (frame_width - crop_width) / 2;
            int y = (frame_height - crop_height) / 2;

            Rect roi(x, y, crop_width, crop_height);
            frame = frame(roi).clone();

            lastFrame = frame.clone(); // �O�s��e�e��
        }

        display = lastFrame.clone();
        Mat buttom = Mat(display.rows, display.cols, CV_8UC3, Scalar(150, 202, 142));

        if (!freeze) {
            // �e�@�ӥզ��Ϋ��s
            int button_radius1 = 26;
            Point button_center(display.cols / 2, display.rows - button_radius1 - 14); // �U��m��
            circle(display, button_center, button_radius1, Scalar(255, 255, 255), FILLED);
            int button_radius2 = 30;
            circle(display, button_center, button_radius2, Scalar(255, 255, 255), 2);

            // �O�����s�d�� (���ƹ�������)
            buttonRect = Rect(button_center.x - button_radius2, button_center.y - button_radius2, button_radius2 * 2, button_radius2 * 2);
        }

        //���U�զ�����᪺�e��
        if (freeze) {
            //Aa
            Point Aa_center(display.cols - 65, 30);
            circle(buttom, Aa_center, buttom_radius, Scalar(0, 0, 0), FILLED);
            putText(buttom, "Aa", Point(Aa_center.x - 9, Aa_center.y + 6), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1.2);
            // �O�����s�d�� (���ƹ�������)
            buttonAa = Rect(Aa_center.x - buttom_radius, Aa_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);
            //Paint
            Point Paint_center(display.cols - 30, 30);
            circle(buttom, Paint_center, buttom_radius, Scalar(0, 0, 0), FILLED);
            line(buttom, Point(Paint_center.x - 8, Paint_center.y + 8), Point(Paint_center.x + 8, Paint_center.y - 8), Scalar(255, 255, 255), 1.8);
            // �O�����s�d�� (���ƹ�������)
            buttonPaint = Rect(Paint_center.x - buttom_radius, Paint_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);

        }
        if (Aa) {
            circle(buttom, Point(display.cols - 65, 30), buttom_radius + 1, Scalar(255, 255, 255), 1);
        }

        //��J��r
        if (Aa && !textEntered) {
            getline(cin, sentence);
            istringstream iss(sentence);
            while (iss >> word) {
                words.push_back(word);
            }
            // ���ݨϥΪ̿�J�@��
            namedWindow("Aasize", WINDOW_AUTOSIZE);
            createTrackbar("AaSize", "Aasize", &a, 10, putAa);
            textEntered = true;             // �O���w��J�A�קK�@���d��
        }
        if (Aa && textEntered) {
            putAa(0, 0);  // �C�@�V�����r�e�W�h
        }

        //���Uø�ϫ��s�᪺�e��
        if (draw) {
            circle(buttom, Point(display.cols - 30, 30), buttom_radius + 1, Scalar(255, 255, 255), 1);
            for (int k = 1; k <= 5; k++) {
                Point color_center(display.cols - 5 * k - 30 * (k - 1) - 15, display.rows - 20);
                circle(buttom, color_center, buttom_radius, Scalar(B_color[k], G_color[k], R_color[k]), FILLED);
                if (k == 1) {
                    buttonColor1 = Rect(color_center.x - buttom_radius, color_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);
                }if (k == 2) {
                    buttonColor2 = Rect(color_center.x - buttom_radius, color_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);
                }if (k == 3) {
                    buttonColor3 = Rect(color_center.x - buttom_radius, color_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);
                }if (k == 4) {
                    buttonColor4 = Rect(color_center.x - buttom_radius, color_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);
                }if (k == 5) {
                    buttonColor5 = Rect(color_center.x - buttom_radius, color_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);
                }
            }
            //���⾹
            Point color_center(20, display.rows - 20);
            circle(buttom, color_center, buttom_radius, Scalar(B_color[0], G_color[0], R_color[0]), FILLED);
            circle(buttom, Point(color_center.x, color_center.y - 8), 2, Scalar(255, 255, 255), FILLED);
            line(buttom, Point(color_center.x - 4, color_center.y - 4), Point(color_center.x + 4, color_center.y - 4), Scalar(255, 255, 255), 2);
            line(buttom, Point(color_center.x, color_center.y - 2), Point(color_center.x, color_center.y + 10), Scalar(255, 255, 255), 4);
            circle(buttom, color_center, buttom_radius + 1, Scalar(255, 255, 255), 1);
            buttonColor = Rect(color_center.x - buttom_radius - 1, color_center.y - buttom_radius - 1, (buttom_radius - 1) * 2, (buttom_radius - 1) * 2);
            //�ʲӷƱ�
            if (!paintWindowCreated) {
                namedWindow("Paint Settings", WINDOW_AUTOSIZE);
                createTrackbar("Thickness", "Paint Settings", &penThickness, 20);  // max value �i�վ�
                paintWindowCreated = true;
            }
            //�������
            for (int k = 1; k <= 3; k++) {
                Point brush_center(30 + 35 * (k - 1), 30);
                circle(buttom, brush_center, buttom_radius, Scalar(0, 0, 0), FILLED);
                if (k == 1) {
                    line(buttom, Point(brush_center.x - 7, brush_center.y), Point(brush_center.x + 7, brush_center.y), Scalar(255, 255, 255), 3);
                    paintMode1 = Rect(brush_center.x - buttom_radius, brush_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);
                    if (brush == 1) {
                        circle(buttom, brush_center, buttom_radius + 1, Scalar(255, 255, 255), 1);
                    }
                }if (k == 2) {
                    rectangle(buttom, Point(brush_center.x - 4, brush_center.y - 4), Point(brush_center.x + 4, brush_center.y + 4), Scalar(255, 255, 255), FILLED);
                    paintMode2 = Rect(brush_center.x - buttom_radius, brush_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);
                    if (brush == 2) {
                        circle(buttom, brush_center, buttom_radius + 1, Scalar(255, 255, 255), 1);
                    }
                }
                if (k == 3) {
                    circle(buttom, brush_center, 5, Scalar(255, 255, 255), FILLED);
                    paintMode3 = Rect(brush_center.x - buttom_radius, brush_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);
                    if (brush == 3) {
                        circle(buttom, brush_center, buttom_radius + 1, Scalar(255, 255, 255), 1);
                    }
                }
            }

        }
        if (!draw && paintWindowCreated) {
            destroyWindow("Paint Settings");
            paintWindowCreated = false;
        }
        Mat show = display.clone();
        for (int b = 0; b < display.cols; b++) {
            for (int c = 0; c < display.rows; c++) {
                if (buttom.at<Vec3b>(c, b) != cv::Vec3b(150, 202, 142)) {
                    show.at<Vec3b>(c, b) = buttom.at<Vec3b>(c, b);
                }
            }
            imshow("Portrait 9:16 Crop", show);
        }
       
        //�Ҧ�����
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

        

        imshow("Portrait 9:16 Crop", display);

        char key = waitKey(30);
        if (key == 27) break;
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
    }

    return 0;
}