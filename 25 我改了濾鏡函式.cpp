#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <sstream>

using namespace cv;
using namespace std;

// 全局變數
bool freeze = false;        // 是否停止畫面
Rect buttonRect;            // 圓形按鈕區域 (停止/恢復)
Rect importButtonRect;      // 匯入圖片按鈕區域
Mat importedImage;          // 匯入的圖片
Mat capturedFrame;          // 儲存拍照的畫面
bool showButtons = true;    // 是否顯示按鈕
VideoCapture cap(0);        // 將 VideoCapture 設為全局變數
int saveCounter = 1;        // 儲存計數器
Mat baseFrame;
int brightness = 50;
int contrast = 50;
int thresh = 127;
int mode = 0;
int prev_mode = -1;
Mat display;
Mat lastFrame;

bool initialized = false;
int setColorTarget = 0;

Vec3b blackColor1(0, 0, 0), blackColor2(0, 0, 0);
Vec3b whiteColor1(255, 255, 255), whiteColor2(255, 255, 255);
bool blackColor1Set = false, blackColor2Set = false;
bool whiteColor1Set = false, whiteColor2Set = false;

int r = 0, g = 0, b = 0;

//WORD
bool Aa = false;
Rect buttonAa;
int a = 1;
string sentence;
vector <string> words;
string word;
bool textEntered = false;  // 加一個旗標來控制只輸入一次
int AaStage = 0;
Point textPosition(5, 75);  // 初始文字位置
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
int R_color[7] = { 0, 255, 255, 0, 255, 0, 0 };
int G_color[7] = { 0, 0, 255, 0, 255, 0, 0 };
int B_color[7] = { 0, 0, 0, 255, 255, 0, 0 };
bool pickColorMode = false;
int penThickness = 2;
bool paintWindowCreated = false;
int brush = 1;
int wait = 0;
Rect paintMode1, paintMode2, paintMode3;

// 空的 MouseCallback（初始使用）
void emptyCallback(int event, int x, int y, int flags, void* userdata) {
    // 不做任何事情
}
// 滑鼠回呼函數
void mouseCallback(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN) {
        Point clickPoint(x, y);

        // 停止/恢復按鈕
        if (showButtons && buttonRect.contains(clickPoint) && importedImage.empty()) {
            freeze = !freeze; // 切換凍結狀態
            cout << (freeze ? "畫面已凍結。" : "畫面已恢復。") << endl;
            showButtons = !freeze; // 同步更新 showButtons 的狀態
            if (freeze && capturedFrame.empty()) {
                // 在凍結時保存當前畫面
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
                    cout << "警告：凍結時無法讀取畫面。" << endl;
                    freeze = false; // 如果無法讀取，則取消凍結
                    showButtons = true; // 取消凍結時重新顯示按鈕
                }
            }
        }

        // 匯入圖片按鈕
        if (showButtons && importButtonRect.contains(clickPoint) && importedImage.empty()) {
            string path;
            cout << "請輸入圖片路徑：";
            cin >> path;
            Mat img = imread(path);
            if (!img.empty()) {
                cout << "讀取圖片成功，尺寸為：" << img.cols << "x" << img.rows << endl;

                Size targetSize(360, 640); // 預設畫面大小
                Mat processed;

                if (img.cols > img.rows) {
                    // 橫式照片：等比例縮放寬度為 360，高度自動調整
                    float scale = (float)targetSize.width / img.cols;
                    int newHeight = img.rows * scale;
                    resize(img, processed, Size(targetSize.width, newHeight));
                    Mat background = Mat::zeros(targetSize, img.type());
                    int y_offset = (targetSize.height - newHeight) / 2;
                    processed.copyTo(background(Rect(0, y_offset, targetSize.width, newHeight)));
                    importedImage = background;
                    cout << "這是橫式照片，已置中顯示。" << endl;
                }
                else {
                    // 直式或正方形照片：等比例縮放高度為 640，寬度可能超過
                    float scale = (float)targetSize.height / img.rows;
                    int newWidth = img.cols * scale;
                    resize(img, processed, Size(newWidth, targetSize.height));
                    int x_offset = (newWidth - targetSize.width) / 2;
                    Rect cropRegion(x_offset, 0, targetSize.width, targetSize.height);
                    importedImage = processed(cropRegion).clone();
                    cout << "這是直式或正方形照片，已裁切置中顯示。" << endl;
                }
                freeze = true;
                showButtons = false; // 匯入圖片後也隱藏按鈕
                cout << "圖片匯入成功！" << endl;
            }
            else {
                cout << "無法讀取圖片，請確認路徑是否正確。" << endl;
            }
        }
    }

    if (freeze) {
        Point mousePt(x, y);
        if (Aa && textEntered) {
            // 用 lines 對每一行文字框出一個大範圍
            int totalHeight = lines.size() * 30;  // 粗估一行高
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
            // 畫圖模式
            if (event == EVENT_LBUTTONDOWN) {
                drawing = true;
                drawing_point = Point(x, y);
            }
            else if (event == EVENT_MOUSEMOVE && drawing) {
                if (brush == 1) {
                    if (!importedImage.empty()) {
                        line(importedImage, drawing_point, Point(x, y), Scalar(B_color[0], G_color[0], R_color[0]), penThickness);
                    }
                    else {
                        line(lastFrame, drawing_point, Point(x, y), Scalar(B_color[0], G_color[0], R_color[0]), penThickness);
                    }
                }
                else if (brush == 2) {
                    if (!importedImage.empty()) {
                        rectangle(importedImage, Point(x - (penThickness / 2), y - (penThickness / 2)), Point(x + (penThickness / 2), y + (penThickness / 2)), Scalar(B_color[0], G_color[0], R_color[0]), FILLED);
                    }
                    else {
                        rectangle(lastFrame, Point(x - (penThickness / 2), y - (penThickness / 2)), Point(x + (penThickness / 2), y + (penThickness / 2)), Scalar(B_color[0], G_color[0], R_color[0]), FILLED);
                    }
                }
                else if (brush == 3) {
                    wait = (wait + 1) % (penThickness);
                    if (wait < (penThickness / 2)) {
                        if (!importedImage.empty()) {
                            circle(importedImage, drawing_point, (penThickness / 2), Scalar(B_color[0], G_color[0], R_color[0]), FILLED);
                        }
                        else {
                            circle(lastFrame, drawing_point, (penThickness / 2), Scalar(B_color[0], G_color[0], R_color[0]), FILLED);
                        }
                    }
                }
                drawing_point = Point(x, y);
            }
            else if (event == EVENT_LBUTTONUP) {
                drawing = false;
            }
            //取得點選位置的顏色
            if (pickColorMode && event == EVENT_LBUTTONDOWN) {

                if (draw) {
                    if (x >= 0 && x < display.cols && y >= 0 && y < display.rows) {
                        Vec3b color = display.at<Vec3b>(y, x);
                        R_color[0] = color[2];
                        G_color[0] = color[1];
                        B_color[0] = color[0];
                        pickColorMode = false;
                    }
                }
                else if (Aa) {
                    if (x >= 0 && x < display.cols && y >= 0 && y < display.rows) {
                        Vec3b color = display.at<Vec3b>(y, x);
                        R_color[6] = color[2];
                        G_color[6] = color[1];
                        B_color[6] = color[0];
                        pickColorMode = false;
                    }
                }
                return;
            }
            if (buttonColor.contains(Point(x, y))) {
                pickColorMode = true;
                return;
            }
        }
        // 功能選單模式
        if (event == EVENT_LBUTTONDOWN) {
            if (buttonAa.contains(Point(x, y))) {
                AaStage = (AaStage + 1) % 4;
                if (AaStage == 0) {
                    Aa = false;
                }
                else {
                    Aa = true;
                }
            }
            if (buttonPaint.contains(Point(x, y))) {
                draw = !draw;
                drawing = false;
                // 不要再呼叫 setMouseCallback，這樣會一直覆蓋
            }
            if (buttonColor1.contains(Point(x, y))) {
                if (draw) {
                    R_color[0] = R_color[1], G_color[0] = G_color[1], B_color[0] = B_color[1];
                }
                else if (Aa) {
                    R_color[6] = R_color[1], G_color[6] = G_color[1], B_color[6] = B_color[1];
                }

            }
            if (buttonColor2.contains(Point(x, y))) {
                if (draw) {
                    R_color[0] = R_color[2], G_color[0] = G_color[2], B_color[0] = B_color[2];
                }
                else if (Aa) {
                    R_color[6] = R_color[2], G_color[6] = G_color[2], B_color[6] = B_color[2];
                }
            }
            if (buttonColor3.contains(Point(x, y))) {
                if (draw) {
                    R_color[0] = R_color[3], G_color[0] = G_color[3], B_color[0] = B_color[3];
                }
                else if (Aa) {
                    R_color[6] = R_color[3], G_color[6] = G_color[3], B_color[6] = B_color[3];
                }
            }
            if (buttonColor4.contains(Point(x, y))) {
                if (draw) {
                    R_color[0] = R_color[4], G_color[0] = G_color[4], B_color[0] = B_color[4];
                }
                else if (Aa) {
                    R_color[6] = R_color[4], G_color[6] = G_color[4], B_color[6] = B_color[4];
                }
            }
            if (buttonColor5.contains(Point(x, y))) {
                if (draw) {
                    R_color[0] = R_color[5], G_color[0] = G_color[5], B_color[0] = B_color[5];
                }
                else if (Aa) {
                    R_color[6] = R_color[5], G_color[6] = G_color[5], B_color[6] = B_color[5];
                }
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
}

void putAa(int, void*) {
    double AaScale = a / 2.0;
    int font = FONT_HERSHEY_SIMPLEX;
    int margin = 5;
    int maxLineWidth = display.cols - 2 * margin;

    int baseline = 0;
    int y = textPosition.y;
    lines.clear();  // 清空重排行文字
    string currentLine;

    for (size_t i = 0; i < words.size(); ++i) {
        string testLine = currentLine.empty() ? words[i] : currentLine + " " + words[i];
        Size testSize = getTextSize(testLine, font, AaScale, AaScale, &baseline);
        if (testSize.width > maxLineWidth && !currentLine.empty()) {
            lines.push_back(currentLine);
            currentLine = words[i]; // 換新行
        }
        else {
            currentLine = testLine;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);

    // 畫每一行，根據 AaStage 對齊，但 y 從 textPosition.y 開始
    for (const string& line : lines) {
        Size textSize = getTextSize(line, font, AaScale, AaScale, &baseline);
        int x;
        if (AaStage == 3) {
            x = textPosition.x + (maxLineWidth - textSize.width);  // 右對齊
        }
        else if (AaStage == 1) {
            x = textPosition.x;  // 左對齊
        }
        else if (AaStage == 2) {
            x = textPosition.x + (maxLineWidth - textSize.width) / 2;  // 置中
        }

        putText(display, line, Point(x, y), font, AaScale, Scalar(B_color[6], G_color[6], R_color[6]), AaScale);
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
    Mat result, gray, dst;
    cvtColor(src, gray, COLOR_BGR2GRAY);
    threshold(gray, dst, thresh, 255, 0);
    cvtColor(dst, result, COLOR_GRAY2BGR);
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
        gradBlack[0] = static_cast<uchar>((1 - alpha) * blackColor1[0] + alpha * blackColor2[0]);
        gradBlack[1] = static_cast<uchar>((1 - alpha) * blackColor1[1] + alpha * blackColor2[1]);
        gradBlack[2] = static_cast<uchar>((1 - alpha) * blackColor1[2] + alpha * blackColor2[2]);

        gradWhite[0] = static_cast<uchar>((1 - alpha) * whiteColor1[0] + alpha * whiteColor2[0]);
        gradWhite[1] = static_cast<uchar>((1 - alpha) * whiteColor1[1] + alpha * whiteColor2[1]);
        gradWhite[2] = static_cast<uchar>((1 - alpha) * whiteColor1[2] + alpha * whiteColor2[2]);

        for (int col = 0; col < dst.cols; col++) {
            if (dst.at<uchar>(row, col) == 0 && blackColor1Set && blackColor2Set) {
                result.at<Vec3b>(row, col) = gradBlack;
            }
            else if (dst.at<uchar>(row, col) != 0 && whiteColor1Set && whiteColor2Set) {
                result.at<Vec3b>(row, col) = gradWhite;
            }
            else {
                uchar grayVal = dst.at<uchar>(row, col);
                result.at<Vec3b>(row, col) = Vec3b(grayVal, grayVal, grayVal);
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
    resizeWindow("Portrait 9:16 Crop", 360, 640);        // 設定視窗尺寸
    moveWindow("Portrait 9:16 Crop", 100, 100);         // 將視窗移動到畫面位置，避免被遮住
    setMouseCallback("Portrait 9:16 Crop", mouseCallback); // 設定滑鼠回呼

    createTrackbar("Brightness", "Portrait 9:16 Crop", &brightness, 100, nullptr);
    createTrackbar("Contrast", "Portrait 9:16 Crop", &contrast, 100, nullptr);
    createTrackbar("Thresh", "Portrait 9:16 Crop", &thresh, 255, nullptr);

    Mat frame;


    while (true) {
        if (!freeze && importedImage.empty()) { // 只有在未凍結且沒有匯入圖片時才讀取攝影機
            cap >> frame;
            if (frame.empty()) break;

            // --- 9:16 裁切 ---
            /*原攝影機的長寬*/
            int frame_width = frame.cols;
            int frame_height = frame.rows;

            /*顯示的長寬*/
            int crop_height = frame_height;
            int crop_width = crop_height * 9 / 16;
            /*檢查新的寬度是否超過原攝影機的長寬，如果超過就把寬當高，再重新計算*/
            if (crop_width > frame_width) {
                crop_width = frame_width;
                crop_height = crop_width * 16 / 9;
            }
            /*計算出原攝影機顯示畫面左上角的座標*/
            int x = (frame_width - crop_width) / 2;
            int y = (frame_height - crop_height) / 2;

            /*定義出要裁切的範圍*/
            Rect roi(x, y, crop_width, crop_height);
            frame = frame(roi).clone();

            lastFrame = frame.clone(); // 保存當前畫面
        }


        if (!importedImage.empty()) {
            display = importedImage.clone(); // 顯示匯入的圖片
        }
        else {
            display = lastFrame.clone();        // 顯示攝影機畫面或凍結的畫面
        }
        Mat buttom = Mat(display.rows, display.cols, CV_8UC3, Scalar(150, 202, 142));

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
                blackColor1 = Vec3b(b, g, r);
                blackColor1Set = true;
                std::cout << "黑色漸層起始色已設定\n";
            }
            else if (setColorTarget == 2) {
                blackColor2 = Vec3b(b, g, r);
                blackColor2Set = true;
                std::cout << "黑色漸層結束色已設定\n";
            }
            else if (setColorTarget == 3) {
                whiteColor1 = Vec3b(b, g, r);
                whiteColor1Set = true;
                std::cout << "白色漸層起始色已設定\n";
            }
            else if (setColorTarget == 4) {
                whiteColor2 = Vec3b(b, g, r);
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
            // 畫一個白色圓形按鈕
            int button_radius1 = 26;
            Point button_center(display.cols / 2, display.rows - button_radius1 - 14); // 下方置中
            circle(display, button_center, button_radius1, Scalar(255, 255, 255), FILLED);
            int button_radius2 = 30;
            circle(display, button_center, button_radius2, Scalar(255, 255, 255), 2);

            // 記錄按鈕範圍 (給滑鼠偵測用)
            buttonRect = Rect(button_center.x - button_radius2, button_center.y - button_radius2, button_radius2 * 2, button_radius2 * 2);
        }


        // 只有在 showButtons 為 true 且沒有匯入圖片時才繪製按鈕
        if (showButtons && importedImage.empty()) {
            // 畫一個白色圓形按鈕
            int button_radius1 = 26;
            Point button_center(display.cols / 2, display.rows - button_radius1 - 14); // 下方置中
            circle(display, button_center, button_radius1, Scalar(255, 255, 255), FILLED);
            int button_radius2 = 30;
            circle(display, button_center, button_radius2, Scalar(255, 255, 255), 2);
            buttonRect = Rect(button_center.x - button_radius2, button_center.y - button_radius2, button_radius2 * 2, button_radius2 * 2);

            int square_size = 25;
            //最左上角距離左邊界10，底部往上邊長+10
            Point import_top_left(10, display.rows - square_size - 10);
            rectangle(display, Rect(import_top_left.x, import_top_left.y, square_size, square_size), Scalar(255, 255, 255), 2);
            importButtonRect = Rect(import_top_left.x, import_top_left.y, square_size, square_size);
        }
        else {
            // 如果有匯入圖片，則不繪製按鈕，並將按鈕區域設定為無效
            buttonRect = Rect();
            importButtonRect = Rect();
        }
        //按下白色圓按鍵後的畫面
        if (freeze) {
            //Aa
            Point Aa_center(display.cols - 65, 30);
            circle(buttom, Aa_center, buttom_radius, Scalar(0, 0, 0), FILLED);
            putText(buttom, "Aa", Point(Aa_center.x - 9, Aa_center.y + 6), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1.2);
            // 記錄按鈕範圍 (給滑鼠偵測用)
            buttonAa = Rect(Aa_center.x - buttom_radius, Aa_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);
            //Paint
            Point Paint_center(display.cols - 30, 30);
            circle(buttom, Paint_center, buttom_radius, Scalar(0, 0, 0), FILLED);
            line(buttom, Point(Paint_center.x - 8, Paint_center.y + 8), Point(Paint_center.x + 8, Paint_center.y - 8), Scalar(255, 255, 255), 1.8);
            // 記錄按鈕範圍 (給滑鼠偵測用)
            buttonPaint = Rect(Paint_center.x - buttom_radius, Paint_center.y - buttom_radius, buttom_radius * 2, buttom_radius * 2);

        }
        if (Aa) {
            circle(buttom, Point(display.cols - 65, 30), buttom_radius + 1, Scalar(255, 255, 255), 1);
        }

        //輸入文字
        if (Aa && !textEntered) {
            getline(cin, sentence);
            istringstream iss(sentence);
            while (iss >> word) {
                words.push_back(word);
            }
            // 等待使用者輸入一次
            namedWindow("Aasize", WINDOW_AUTOSIZE);
            createTrackbar("AaSize", "Aasize", &a, 10, putAa);
            textEntered = true;             // 記錄已輸入，避免一直卡住
        }
        if (Aa && textEntered) {
            putAa(0, 0);  // 每一幀都把文字畫上去
        }

        if (Aa || draw) {
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
            //取色器
            Point color_center(20, display.rows - 20);
            circle(buttom, color_center, buttom_radius, Scalar(B_color[0], G_color[0], R_color[0]), FILLED);
            circle(buttom, Point(color_center.x, color_center.y - 8), 2, Scalar(255, 255, 255), FILLED);
            line(buttom, Point(color_center.x - 4, color_center.y - 4), Point(color_center.x + 4, color_center.y - 4), Scalar(255, 255, 255), 2);
            line(buttom, Point(color_center.x, color_center.y - 2), Point(color_center.x, color_center.y + 10), Scalar(255, 255, 255), 4);
            circle(buttom, color_center, buttom_radius + 1, Scalar(255, 255, 255), 1);
            buttonColor = Rect(color_center.x - buttom_radius - 1, color_center.y - buttom_radius - 1, (buttom_radius - 1) * 2, (buttom_radius - 1) * 2);
        }
        //按下繪圖按鈕後的畫面
        if (draw) {
            circle(buttom, Point(display.cols - 30, 30), buttom_radius + 1, Scalar(255, 255, 255), 1);


            //粗細滑桿
            if (!paintWindowCreated) {
                namedWindow("Paint Settings", WINDOW_AUTOSIZE);
                createTrackbar("Thickness", "Paint Settings", &penThickness, 20);  // max value 可調整
                paintWindowCreated = true;
            }
            //筆刷種類
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
        }
        imshow("Portrait 9:16 Crop", show);



        char key = waitKey(30);
        if (key == 27) { // ESC
            cout << "退出程式" << endl;
            break;
        }
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
        else if (key == 82 || key == 114) { // R or r
            importedImage.release();
            capturedFrame.release();
            freeze = false;
            showButtons = true; // 重設後顯示按鈕
            Aa = false;
            draw = false;
            cout << "已重設所有圖片，恢復即時攝影機畫面。" << endl;
        }
        else if (key == 83 || key == 115) { // S or s
            Mat imageToSave;
            string filename;
            if (!importedImage.empty()) {
                imageToSave = display;
                stringstream filenameSS;
                filenameSS << "image_" << saveCounter << ".jpg"; // 使用相同的檔名格式和計數器
                filename = filenameSS.str();
                cout << "將匯入的圖片儲存為：" << filename << endl;
                saveCounter++; // 遞增計數器
            }
            else if (freeze && !capturedFrame.empty()) {
                imageToSave = display;
                stringstream filenameSS;
                filenameSS << "image_" << saveCounter << ".jpg"; // 使用相同的檔名格式和計數器
                filename = filenameSS.str();
                cout << "已儲存凍結的畫面為：" << filename << endl;
                saveCounter++; // 遞增計數器
            }
            else {
                cout << "目前沒有可儲存的圖片 (沒有匯入圖片且畫面未凍結)。" << endl;
                continue;
            }
            if (!imageToSave.empty()) {
                if (imwrite(filename, imageToSave)) {
                    cout << "圖片已儲存為：" << filename << endl;
                }
                else {
                    cout << "儲存圖片失敗。" << endl;
                }
            }
        }
    }
    return 0;
}