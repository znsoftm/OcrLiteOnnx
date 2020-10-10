#ifndef OCRLITEONNX_OCRLITE_H
#define OCRLITEONNX_OCRLITE_H

#include "opencv2/core.hpp"
#include "ncnn/net.h"
#include "OcrStruct.h"

class OcrLite {
public:
    OcrLite(const char *path);

    ~OcrLite();

    std::string detect(const char *path, const char *imgName, const float imgScale = 1.f,
                       float boxScoreThresh = 0.6f, float boxThresh = 0.3f, float minArea = 3.f,
                       float angleScaleWidth = 1.3f, float angleScaleHeight = 1.3f,
                       float textScaleWidth = 1.6f, float textScaleHeight = 1.6f);

private:
    std::vector<TextBox> getTextBoxes(cv::Mat &src, ScaleParam &s,
                                      float boxScoreThresh, float boxThresh, float minArea);

    Angle getAngle(cv::Mat &src);

    TextLine getTextLine(cv::Mat &src);

    TextLine scoreToString(ncnn::Mat &score);

    ncnn::Net dbNet, angleNet, crnnNet;
    int numThread = 4;

    const float meanValsDBNet[3] = {0.485 * 255, 0.456 * 255, 0.406 * 255};
    const float normValsDBNet[3] = {1.0 / 0.229 / 255.0, 1.0 / 0.224 / 255.0, 1.0 / 0.225 / 255.0};

    int angleDstWidth = 196;
    int angleDstHeight = 32;
    const float meanValsAngle[3] = {0.485 * 255, 0.456 * 255, 0.406 * 255};
    const float normValsAngle[3] = {1.0 / 0.229 / 255.0, 1.0 / 0.224 / 255.0, 1.0 / 0.225 / 255.0};

    int crnnDstHeight = 32;
    const float meanValsCrnn[3] = {127.5, 127.5, 127.5};
    const float normValsCrnn[3] = {1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5};

    std::vector<std::string> keys;
};

#endif //OCRLITEONNX_OCRLITE_H
