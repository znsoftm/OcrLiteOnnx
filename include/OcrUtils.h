#ifndef OCRLITEONNX_OCRUTILS_H
#define OCRLITEONNX_OCRUTILS_H

#include <opencv2/core.hpp>
#include "OcrStruct.h"

ScaleParam getScaleParam(cv::Mat &src, const float scale);

ScaleParam getScaleParam(cv::Mat &src, const int targetSize);

cv::RotatedRect getPartRect(std::vector<cv::Point> &box, float scaleWidth, float scaleHeight);

std::vector<cv::Point2f> getBox(cv::RotatedRect &rect);

void drawTextBox(cv::Mat &boxImg, cv::RotatedRect &rect);

cv::Mat matRotateClockWise180(cv::Mat src);

cv::Mat matRotateClockWise90(cv::Mat src);

int getMiniBoxes(std::vector<cv::Point> &invec,
                 std::vector<cv::Point> &minboxvec,
                 float &minedgesize, float &alledgesize
);

float boxScoreFast(cv::Mat &mapmat, std::vector<cv::Point> &_box);

int unClip(std::vector<cv::Point> &minboxvec, float alledgesize,
           std::vector<cv::Point> &outvec, float unclip_ratio);

void saveImg(cv::Mat &img, const char *imgPath);

std::string getSrcImgFilePath(const char *path, const char *imgName);

std::string getResultTxtFilePath(const char *path, const char *imgName);

std::string getResultImgFilePath(const char *path, const char *imgName);

std::string getPartImgFilePath(const char *path, const char *imgName, int i);

std::string getDebugImgFilePath(const char *path, const char *imgName, int i);

#endif //OCRLITEONNX_OCRUTILS_H
