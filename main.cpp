#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

int main() {

    Mat frame = imread("../resources/test_image_2.jpg");
    if (frame.empty()) {
        cerr << "无法读取图像!" << endl;
        return -1;
    }

    //图像预处理
    Mat gray, blurred;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, blurred, Size(5, 5), 0);

    //亮度二值化
    Mat binary;
    threshold(blurred, binary, 220, 255, THRESH_BINARY);

    //形态学操作
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(binary, binary, MORPH_CLOSE, kernel);

    //查找并筛选轮廓
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(binary, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<RotatedRect> possibleLights;
    for (const auto& contour : contours) {
        if (contour.size() < 5) continue;

        RotatedRect rect = minAreaRect(contour);
        float width = rect.size.width;
        float height = rect.size.height;
        
        if (width < height) {
            swap(width, height);
            rect.angle = rect.angle + 90;
        }
        
        float ratio = height / width;
        
        if (ratio < 0.4f&&ratio>0.2f && rect.size.area() > 300.0f) {
            possibleLights.push_back(rect);
        }
    }

    //配对灯条
    vector<Rect> pairedRectangles;
    for (size_t i = 0; i < possibleLights.size(); i++) {
        for (size_t j = i + 1; j < possibleLights.size(); j++) {
            const RotatedRect& light1 = possibleLights[i];
            const RotatedRect& light2 = possibleLights[j];

            // 检查面积是否相似
            float areaRatio = light1.size.area() / light2.size.area();
            if (areaRatio < 0.8f || areaRatio > 1.2f) continue;
            
            // 检查宽高比是否相似
            float ratio1 = min(light1.size.width, light1.size.height) / max(light1.size.width, light1.size.height);
            float ratio2 = min(light2.size.width, light2.size.height) / max(light2.size.width, light2.size.height);
            float ratioDiff = abs(ratio1 - ratio2);
            if (ratioDiff > 0.1f) continue;
            
            // 检查是否几乎竖直
            bool isVertical1 = abs(light1.angle) > 80 || abs(light1.angle) < 10;
            bool isVertical2 = abs(light2.angle) > 80 || abs(light2.angle) < 10;
            if (!isVertical1 || !isVertical2) continue;
            
            // 计算两个灯条中心点距离
            Point2f center1 = light1.center;
            Point2f center2 = light2.center;
            float distance = norm(center1 - center2);

            // 计算两个灯条的角度差
            float angleDiff = abs(light1.angle - light2.angle);
            if (angleDiff > 90) angleDiff = 180 - angleDiff;

            // 配对
            if (angleDiff < 5.0f && distance > 20.0f && distance < 300.0f) {

                Point2f points1[4], points2[4];
                light1.points(points1);
                light2.points(points2);

                vector<Point2f> allPoints;
                for (int k = 0; k < 4; k++) {
                    allPoints.push_back(points1[k]);
                    allPoints.push_back(points2[k]);
                }
                
                Rect boundingBox = boundingRect(allPoints);
                pairedRectangles.push_back(boundingBox);
            }
        }
    }

    // 绘制方框
    for (const Rect& rect : pairedRectangles) {
        rectangle(frame, rect, Scalar(0, 0, 255), 5);
    }

    //imwrite("../result/装甲板识别.png", frame);
    imshow("装甲板识别", frame);
    waitKey(0);

    return 0;
}

/*
int main() {


    Mat img =imread("../resources/test_image.png");

    if(img.empty()){
        cout<<"无法读取图像！"<<endl;
        return -1;
    }

    //灰度图
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    //imwrite("../resources/result_test/灰度图.png", gray);
    imshow("灰度图", gray);

    //HSV图
    Mat hsv;
    cvtColor(img,hsv , COLOR_BGR2HSV);
    //imwrite("../resources/result_test/HSV.png", hsv);
    imshow("HSV图", hsv);

    //均值滤波
    Mat blurImg;
    blur(img, blurImg, Size(5, 5));
    //imwrite("../resources/result_test/均值滤波.png", blurImg);
    imshow("均值滤波", blurImg);

    //高斯滤波
    Mat gaussianImg;
    GaussianBlur(img, gaussianImg, Size(5, 5), 1.5);
    //imwrite("../resources/result_test/高斯滤波.png", gaussianImg);
    imshow("高斯滤波", gaussianImg);

    //提取红色区域
    Mat mask;
    inRange(hsv, Scalar(0, 100, 100), Scalar(10, 255, 255), mask);
    //imwrite("../resources/result_test/红色区域.png", mask);
    imshow("红色区域", mask);

    //提取红色区域外轮廓
    Mat edges;
    Canny(mask, edges, 100, 200);
    //形态学操作
    Mat morph;
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(edges, morph, MORPH_CLOSE, kernel);
    //轮廓提取和绘制
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(morph, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    Mat contourImg = Mat::zeros(img.size(), CV_8UC3);
    for (size_t i = 0; i < contours.size(); i++) {
        drawContours(contourImg, contours, (int)i, Scalar(0, 255, 0), 2);
    }
    //imwrite("../resources/result_test/红色区域外轮廓.png", contourImg);
    imshow("红色区域外轮廓", contourImg);

    //bounding box
    Mat img0=img.clone();
    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        //if (area < 500) continue;
        Rect bbox = boundingRect(contours[i]);
        rectangle(img0, bbox, Scalar(0, 0, 255), 2);
    }
    //imwrite("../resources/result_test/bounding box.png", img0);
    imshow("bounding box", img0);

    //计算轮廓面积
    double totalArea = 0.0;
    for (const auto& contour : contours) {
        double area = contourArea(contour);
        totalArea += area;
    }
    cout << "Total area of all contours: " << totalArea << " square pixels" << endl;
    
    //提取高亮区域并进行图形学处理
    vector<Mat> hsvChannels;
    split(img, hsvChannels);
    Mat valueChannel = hsvChannels[2];
    Mat hlm;
    threshold(valueChannel, hlm, 200, 255, THRESH_BINARY);
    //imwrite("../resources/result_test/高亮区域.png", hlm);
    imshow("高亮区域", hlm);

    // 二值化处理
    Mat b_hlm;
    threshold(hlm, b_hlm, 0, 255, THRESH_BINARY | THRESH_OTSU);
    //imwrite("../resources/result_test/高亮区域二值化.png", b_hlm);
    imshow("高亮区域二值化", b_hlm);

    // 膨胀
    Mat d_hlm;
    dilate(b_hlm, d_hlm, kernel);
    //imwrite("../resources/result_test/高亮区域膨胀.png", d_hlm);
    imshow("高亮区域膨胀", d_hlm);

    // 腐蚀
    Mat e_hlm;
    erode(b_hlm, e_hlm, kernel);
    //imwrite("../resources/result_test/高亮区域腐蚀.png", e_hlm);
    imshow("高亮区域腐蚀", e_hlm);

    // 漫水处理
    Mat f_hlm = d_hlm.clone(); 
    Point seedPoint(f_hlm.cols / 2, f_hlm.rows / 2);
    Scalar newVal = Scalar(255);
    Scalar loDiff = Scalar(5);
    Scalar upDiff = Scalar(5);
    int flags = 4 | FLOODFILL_FIXED_RANGE;
    Rect rect;
    int area = floodFill(f_hlm, seedPoint, newVal, &rect, loDiff, upDiff, flags);
    //imwrite("../resources/result_test/高亮区域漫水填充.png", f_hlm);
    imshow("高亮区域漫水填充", f_hlm);

    //图像绘制
    Mat IMG(600, 600, CV_8UC3, Scalar(255, 255, 255));
    Point center(150, 150); 
    int outerR = 80;
    int innerR = 50;
    circle(IMG, center, outerR, Scalar(0, 60, 120), -1, LINE_AA);
    circle(IMG, center, innerR, Scalar(255, 255, 255), -1, LINE_AA);
    Point rectTopLeft(100, 450);
    Point rectBottomRight(250, 550);
    rectangle(IMG, rectTopLeft, rectBottomRight, Scalar(255, 0, 0), -1, LINE_AA);
    String text = "RMV";
    Point textOrg(350, 300);
    int fontFace = FONT_HERSHEY_SIMPLEX; 
    double fontScale = 2.0;
    Scalar textColor(0, 0, 0);
    int thickness = 3;
    putText(IMG, text, textOrg, fontFace, fontScale, textColor, thickness, LINE_AA); 
    //imwrite("../resources/result_test/绘制图像RMV.png", IMG);
    imshow("绘制图像RMV",IMG);

    //绘制红色外轮廓
    Mat GRAY;
    cvtColor(IMG, GRAY, COLOR_BGR2GRAY);
    Mat EDGES;
    Canny(GRAY, EDGES, 100, 200);
    Mat MORPH;
    morphologyEx(EDGES, MORPH, MORPH_CLOSE, kernel);
    vector<vector<Point>> Contours;
    vector<Vec4i> Hierarchy;
    findContours(MORPH, Contours, Hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    Mat ContourImg = Mat::zeros(IMG.size(), CV_8UC3);
    for (size_t i = 0; i < Contours.size(); i++) {
        drawContours(ContourImg, Contours, (int)i, Scalar(0, 0, 255), 2);
    }
    //imwrite("../resources/result_test/RMV红色外轮廓.png", ContourImg);
    imshow("RMV红色外轮廓", ContourImg);

    //绘制红色的boundingbox
    Mat IMG0=IMG.clone();
    for (size_t i = 0; i < Contours.size(); i++) {
        double Area = contourArea(Contours[i]);
        //if (Area < 500) continue;
        Rect Bbox = boundingRect(Contours[i]);
        rectangle(IMG0, Bbox, Scalar(0, 0, 255), 2);
    }
    //imwrite("../resources/result_test/RMV_bounding box.png", IMG0);
    imshow("RMV_bounding box", IMG0);

    //图片旋转
    int W = IMG.cols;
    int H = IMG.rows;
    Point2f Center(W / 2.0f, H / 2.0f);
    double angle = 35.0;
    double scale = 1.0;
    Mat rotationMatrix = getRotationMatrix2D(Center, angle, scale);
    Mat R_IMG;
    warpAffine(IMG, R_IMG, rotationMatrix, IMG.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar());
    //imwrite("../resources/result_test/RMV_旋转.png", R_IMG);
    imshow("RMV_旋转", R_IMG);

    // 图像裁剪
    Rect roi(0, 0,W/2, H/2);
    Mat C_IMG = IMG(roi);
    //imwrite("../resources/result_test/RMV_裁减.png", C_IMG);
    imshow("RMV_裁减", C_IMG);
    waitKey(0);

    return 0;
}

*/