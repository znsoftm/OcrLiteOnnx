#include <iosfwd>
#include "OcrLite.h"
#include <opencv/cv.hpp>
#include <LogUtils.h>
#include <RRLib.h>
#include "OcrUtils.h"

OcrLite::OcrLite(const char *path) {
    std::string pathStr = path;
    int dbParam = dbNet.load_param((pathStr + "/dbnet_op.param").c_str());
    int dbModel = dbNet.load_model((pathStr + "/dbnet_op.bin").c_str());
    printf("DBNet load param=%d, model=%d\n", dbParam, dbModel);

    int angleParam = angleNet.load_param((pathStr + "/angle_op.param").c_str());
    int angleModel = angleNet.load_model((pathStr + "/angle_op.bin").c_str());
    printf("AngleNet load param=%d, model=%d\n", angleParam, angleModel);

    int crnnParam = crnnNet.load_param((pathStr + "/crnn_lite_op.param").c_str());
    int crnnModel = crnnNet.load_model((pathStr + "/crnn_lite_op.bin").c_str());

    //load keys
    std::ifstream in((pathStr +"/keys.txt").c_str());
    std::string line;
    int keysSize = 0;
    if (in) {// 有该文件
        while (getline(in, line)) {// line中不包括每行的换行符
            keys.push_back(line);
        }
        keysSize = keys.size();
    } else { // 没有该文件
        printf("no txt file\n");
    }

    printf("CrnnNet load param=%d, model=%d, keys size=%d\n",
           crnnParam, crnnModel, keysSize);
}

OcrLite::~OcrLite() {
    dbNet.clear();
    angleNet.clear();
    crnnNet.clear();
}

std::vector<TextBox>
OcrLite::getTextBoxes(cv::Mat &src, ScaleParam &s,
                      float boxScoreThresh, float boxThresh, float minArea) {
    std::vector<TextBox> rsBoxes;
    ncnn::Mat input = ncnn::Mat::from_pixels_resize(src.data, ncnn::Mat::PIXEL_BGR2RGB,
                                                    src.cols, src.rows,
                                                    s.dstWidth, s.dstHeight);

    input.substract_mean_normalize(meanValsDBNet, normValsDBNet);
    ncnn::Extractor extractor = dbNet.create_extractor();
    extractor.set_num_threads(numThread);
    extractor.input("input0", input);
    ncnn::Mat out;
    extractor.extract("out1", out);

    cv::Mat fMapMat(s.dstHeight, s.dstWidth, CV_32FC1);
    memcpy(fMapMat.data, (float *) out.data, s.dstWidth * s.dstHeight * sizeof(float));

    cv::Mat norfMapMat;

    norfMapMat = fMapMat > boxThresh;

    rsBoxes.clear();
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(norfMapMat, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    for (int i = 0; i < contours.size(); ++i) {
        std::vector<cv::Point> minBox;
        float minEdgeSize, allEdgeSize;
        getMiniBoxes(contours[i], minBox, minEdgeSize, allEdgeSize);

        if (minEdgeSize < minArea)
            continue;
        float score = boxScoreFast(fMapMat, contours[i]);

        if (score < boxScoreThresh)
            continue;

        /*std::vector<cv::Point> newBox;
        unClip(minBox, allEdgeSize, newBox, unClipRatio);

        getMiniBoxes(newBox, minBox, minEdgeSize, allEdgeSize);

        if (minEdgeSize < minArea + 2)
            continue;*/

        for (int j = 0; j < minBox.size(); ++j) {
            minBox[j].x = (minBox[j].x / s.scaleWidth);
            minBox[j].x = (std::min)((std::max)(minBox[j].x, 0), s.srcWidth);

            minBox[j].y = (minBox[j].y / s.scaleHeight);
            minBox[j].y = (std::min)((std::max)(minBox[j].y, 0), s.srcHeight);
        }

        rsBoxes.emplace_back(TextBox(minBox, score));
    }
    reverse(rsBoxes.begin(), rsBoxes.end());
    return rsBoxes;
}

Angle scoreToAngle(ncnn::Mat &score) {
    auto *srcData = (float *) score.data;
    int angleIndex = 0;
    float maxValue = -1000.0f;
    for (int i = 0; i < score.w; i++) {
        if (i == 0)maxValue = srcData[i];
        else if (srcData[i] > maxValue) {
            angleIndex = i;
            maxValue = srcData[i];
        }
    }
    return Angle(angleIndex, maxValue);
}

Angle OcrLite::getAngle(cv::Mat &src) {
    float scale = (float) angleDstHeight / (float) src.rows;
    int angleDstWidth = int((float) src.cols * scale);
    ncnn::Mat input = ncnn::Mat::from_pixels_resize(
            src.data, ncnn::Mat::PIXEL_BGR2RGB,
            src.cols, src.rows,
            angleDstWidth, angleDstHeight);
    input.substract_mean_normalize(meanValsAngle, normValsAngle);
    ncnn::Extractor extractor = angleNet.create_extractor();
    extractor.set_num_threads(numThread);
    extractor.input("input", input);
    ncnn::Mat out;
    extractor.extract("out", out);
    return scoreToAngle(out);
}

TextLine OcrLite::scoreToString(ncnn::Mat &score) {
    auto *srcData = (float *) score.data;
    std::string strRes;
    int lastIndex = 0;
    std::vector<float> scores;
    for (int i = 0; i < score.h; i++) {
        //find max score
        int maxIndex = 0;
        float maxValue = -1000;
        for (int j = 0; j < score.w; j++) {
            if (srcData[i * score.w + j] > maxValue) {
                maxValue = srcData[i * score.w + j];
                maxIndex = j;
            }
        }
        if (maxIndex > 0 && (!(i > 0 && maxIndex == lastIndex))) {
            scores.emplace_back(maxValue);
            strRes.append(keys[maxIndex - 1]);
        }
        lastIndex = maxIndex;
    }
    return TextLine(strRes, scores);
}

TextLine OcrLite::getTextLine(cv::Mat &src) {
    float scale = (float) crnnDstHeight / (float) src.rows;
    int dstWidth = int((float) src.cols * scale);

    ncnn::Mat input = ncnn::Mat::from_pixels_resize(
            src.data, ncnn::Mat::PIXEL_BGR2GRAY,
            src.cols, src.rows, dstWidth, crnnDstHeight);

    input.substract_mean_normalize(meanValsCrnn, normValsCrnn);

    ncnn::Extractor extractor = crnnNet.create_extractor();
    extractor.set_num_threads(numThread);
    extractor.input("input", input);

    // lstm
    ncnn::Mat blob162;
    extractor.extract("1000", blob162);

    // batch fc
    ncnn::Mat blob263(5531, blob162.h);
    for (int i = 0; i < blob162.h; i++) {
        ncnn::Extractor extractor2 = crnnNet.create_extractor();
        extractor2.set_num_threads(numThread);
        ncnn::Mat blob243_i = blob162.row_range(i, 1);
        extractor2.input("1014", blob243_i);

        ncnn::Mat blob263_i;
        extractor2.extract("1015", blob263_i);

        memcpy(blob263.row(i), blob263_i, 5531 * sizeof(float));
    }

    return scoreToString(blob263);
}

cv::Mat GetRotateCropImage(const cv::Mat &srcimage,
                           std::vector<cv::Point2f> box) {
    cv::Mat image;
    srcimage.copyTo(image);
    std::vector<cv::Point2f> points = box;

    float x_collect[4] = {box[0].x, box[1].x, box[2].x, box[3].x};
    float y_collect[4] = {box[0].y, box[1].y, box[2].y, box[3].y};
    int left = int(*std::min_element(x_collect, x_collect + 4));
    int right = int(*std::max_element(x_collect, x_collect + 4));
    int top = int(*std::min_element(y_collect, y_collect + 4));
    int bottom = int(*std::max_element(y_collect, y_collect + 4));

    cv::Mat img_crop;
    image(cv::Rect(left, top, right - left, bottom - top)).copyTo(img_crop);

    for (int i = 0; i < points.size(); i++) {
        points[i].x -= left;
        points[i].y -= top;
    }

    int img_crop_width = int(sqrt(pow(points[0].x - points[1].x, 2) +
                                  pow(points[0].y - points[1].y, 2)));
    int img_crop_height = int(sqrt(pow(points[0].x - points[3].x, 2) +
                                   pow(points[0].y - points[3].y, 2)));

    cv::Point2f pts_std[4];
    pts_std[0] = cv::Point2f(0., 0.);
    pts_std[1] = cv::Point2f(img_crop_width, 0.);
    pts_std[2] = cv::Point2f(img_crop_width, img_crop_height);
    pts_std[3] = cv::Point2f(0.f, img_crop_height);

    cv::Point2f pointsf[4];
    pointsf[0] = cv::Point2f(points[0].x, points[0].y);
    pointsf[1] = cv::Point2f(points[1].x, points[1].y);
    pointsf[2] = cv::Point2f(points[2].x, points[2].y);
    pointsf[3] = cv::Point2f(points[3].x, points[3].y);

    cv::Mat M = cv::getPerspectiveTransform(pointsf, pts_std);

    cv::Mat dst_img;
    cv::warpPerspective(img_crop, dst_img, M,
                        cv::Size(img_crop_width, img_crop_height),
                        cv::BORDER_REPLICATE);

    if (float(dst_img.rows) >= float(dst_img.cols) * 1.5) {
        cv::Mat srcCopy = cv::Mat(dst_img.rows, dst_img.cols, dst_img.depth());
        cv::transpose(dst_img, srcCopy);
        cv::flip(srcCopy, srcCopy, 0);
        return srcCopy;
    } else {
        return dst_img;
    }
}

cv::Mat draw_bbox(cv::Mat &src, const std::vector<std::vector<cv::Point>> &bboxs) {
    cv::Mat dst;
    if (src.channels() == 1) {
        cv::cvtColor(src, dst, cv::COLOR_GRAY2BGR);
    } else {
        dst = src.clone();
    }
    auto color = cv::Scalar(0, 0, 255);
    for (auto bbox :bboxs) {
        cv::line(dst, bbox[0], bbox[1], color, 3);
        cv::line(dst, bbox[1], bbox[2], color, 3);
        cv::line(dst, bbox[2], bbox[3], color, 3);
        cv::line(dst, bbox[3], bbox[0], color, 3);
    }
    return dst;
}

std::string OcrLite::detect(const char *path, const char *imgName, const float imgScale,
                            float boxScoreThresh, float boxThresh, float minArea,
                            float angleScaleWidth, float angleScaleHeight,
                            float textScaleWidth, float textScaleHeight) {

    std::string imgFile = getSrcImgFilePath(path, imgName);

    std::string resultTxtFile = getResultTxtFilePath(path, imgName);
    std::string resultImgFile = getResultImgFilePath(path, imgName);

    std::ofstream resultTxt(resultTxtFile);

    cv::Mat src = cv::imread(imgFile);
    cv::Mat imgBox = src.clone();

    //按比例缩小图像，减少文字分割时间
    ScaleParam scale = getScaleParam(src, imgScale);
    //ScaleParam scale = getScaleParam(src, src.cols / 2);

    printf("=====Start detect=====\n");
    resultTxt << "=====Start detect=====" << std::endl;
    printf("ScaleParam(sw:%d,sh:%d,dw:%d,dH%d,%f,%f)\n", scale.srcWidth, scale.srcHeight,
           scale.dstWidth, scale.dstHeight,
           scale.scaleWidth, scale.scaleHeight);
    resultTxt << "ScaleParam(" << scale.srcWidth << "," << scale.srcHeight << "," << scale.dstWidth << ","
              << scale.dstHeight << "," << scale.scaleWidth << "," << scale.scaleHeight << ")" << std::endl;
    double startTime = getCurrentTime();
    std::vector<TextBox> textBoxes = getTextBoxes(src, scale, boxScoreThresh, boxThresh, minArea);
    printf("TextBoxes Size = %ld\n", textBoxes.size());
    resultTxt << "TextBoxes =" << textBoxes.size() << std::endl;
    double endTimeTextBoxes = getCurrentTime();
    double timeTextBoxes = printTime("Time getTextBoxes", startTime, endTimeTextBoxes);
    resultTxt << "文字分割耗时:" << timeTextBoxes << "ms" << std::endl;

    std::string strRes;
    for (int i = 0; i < textBoxes.size(); ++i) {
        printf("-----TextBox[%d] score(%f)-----\n", i, textBoxes[i].score);
        double startTextBox = getCurrentTime();
        //cv::RotatedRect rectText = getPartRect(textBoxes[i].box, 1.4f, 1.4f);
        //std::vector<cv::Point2f> box = getBox(rectText);

        //cv::Mat angleImg = GetRotateCropImage(src, box);

        cv::Mat angleImg;
        cv::RotatedRect rectAngle = getPartRect(textBoxes[i].box, angleScaleWidth,
                                                angleScaleHeight);
        RRLib::getRotRectImg(rectAngle, src, angleImg);
        printf("rectAngle(%f, %f)\n", rectAngle.size.width, rectAngle.size.height);

        //cv::Mat textImg = GetRotateCropImage(src, box);
        cv::Mat textImg;
        cv::RotatedRect rectText = getPartRect(textBoxes[i].box, textScaleWidth,
                                               textScaleHeight);
        RRLib::getRotRectImg(rectText, src, textImg);
        printf("rectText(%f, %f)\n", rectText.size.width, rectText.size.height);


        drawTextBox(imgBox, rectText);
        for (int p = 0; p < 4; ++p) {
            printf("Pt%d(x: %d, y: %d)\n", p, textBoxes[i].box[p].x, textBoxes[i].box[p].y);
        }

        std::string partImgFile = getPartImgFilePath(path, imgName, i);
        saveImg(textImg, partImgFile.c_str());


        if (angleImg.rows > 1.5 * angleImg.cols) {
            angleImg = matRotateClockWise90(angleImg);
            textImg = matRotateClockWise90(textImg);
        }
        Angle angle = getAngle(angleImg);
        if (angle.index == 0 || angle.index == 2)
            textImg = matRotateClockWise180(textImg);
        printf("angle(index=%d, score=%f)\n", angle.index, angle.score);

        std::string debugImgFile = getDebugImgFilePath(path, imgName, i);
        saveImg(textImg, debugImgFile.c_str());

        TextLine textLine = getTextLine(textImg);
        std::ostringstream txtScores;
        for (int s = 0; s < textLine.scores.size(); ++s) {
            txtScores << textLine.scores[s] << ",";
        }
        printf("text(line=%s, scores={%s})\n", textLine.line.c_str(), std::string(txtScores.str()).c_str());
        strRes.append(textLine.line);
        strRes.append("\n");
        double endTextBox = getCurrentTime();
        double timeTextBox = printTime("Time TextLine", startTextBox, endTextBox);

        printf("line[%d]=%s\n", i, textLine.line.c_str());
        resultTxt << "-----TextBox[" << i << "] score(" << textBoxes[i].score << ")-----" << std::endl;
        resultTxt << "rectAngle(" << rectAngle.size.width << ", " << rectAngle.size.height << ")" << std::endl;
        resultTxt << "rectText(" << rectText.size.width << ", " << rectText.size.height << ")" << std::endl;
        resultTxt << "Pt0(x:" << textBoxes[i].box[0].x << ", y:" << textBoxes[i].box[0].y << ") " << std::endl;
        resultTxt << "Pt1(x:" << textBoxes[i].box[1].x << ", y:" << textBoxes[i].box[1].y << ") " << std::endl;
        resultTxt << "Pt2(x:" << textBoxes[i].box[2].x << ", y:" << textBoxes[i].box[2].y << ") " << std::endl;
        resultTxt << "Pt3(x:" << textBoxes[i].box[3].x << ", y:" << textBoxes[i].box[3].y << ") " << std::endl;
        resultTxt << "angle(index=" << angle.index << ", score=" << angle.score << ") " << std::endl;
        resultTxt << "textScores(";
        for (int j = 0; j < textLine.scores.size(); ++j) {
            resultTxt << textLine.scores[j];
            if (j != textLine.scores.size() - 1) resultTxt << ", ";
        }
        resultTxt << ") " << std::endl;
        resultTxt << "text=" << textLine.line << std::endl;
        resultTxt << "timeTextBox(" << timeTextBox << "ms) " << std::endl;
    }
    double endTime = getCurrentTime();
    double timeFull = printTime("Time Full", startTime, endTime);
    resultTxt << "总耗时:" << timeFull / 1000 << "s" << std::endl;
    resultTxt << "=====End detect=====" << std::endl;
    printf("=====End detect=====\n");
    resultTxt.close();
    cv::imwrite(resultImgFile, imgBox);
    return strRes;
}
