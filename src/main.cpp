#include "RRLib.h"
#include "OcrLite.h"

int main(int argc, char **argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    if (argc <= 1) {
        OcrLite ocrLite("../models");
        fprintf(stderr,
                "Usage: %s img/path models/dir imgScale(option) boxScoreThresh(option) boxThresh(option) minArea(option) angleScaleWidth(option) angleScaleHeight(option) textScaleWidth(option) textScaleHeight(option)\n",
                argv[0]);
        fprintf(stderr, "Example: %s ../test/1.jpg ../models\n", argv[0]);
        fprintf(stderr, "Example: %s ../test/1.jpg ../models 1.0 0.6 0.3 3 1.3 1.3 1.6 1.6\n", argv[0]);
        for (int i = 1; i <= 7; ++i) {
            ocrLite.detect("../test/", std::to_string(i).append(".jpg").c_str());
        }
    } else {
        std::string argImgPath, imgPath, imgName, modelsDir;
        modelsDir = "../models";
        float imgScale = 1.f;
        float boxScoreThresh = 0.6f;
        float boxThresh = 0.3f;
        float minArea = 3.f;
        float angleScaleWidth = 1.3f;
        float angleScaleHeight = 1.3f;
        float textScaleWidth = 1.6f;
        float textScaleHeight = 1.6f;

        for (int i = 1; i < argc; ++i) {
            printf("argv[%d]=%s, ", i, argv[i]);
            switch (i) {
                case 1:
                    argImgPath = std::string(argv[i]);
                    imgPath = argImgPath.substr(0, argImgPath.find_last_of('/') + 1);
                    imgName = argImgPath.substr(argImgPath.find_last_of('/') + 1);
                    printf("imgPath=%s, imgName=%s\n", imgPath.c_str(), imgName.c_str());
                    break;
                case 2:
                    modelsDir = argv[i];
                    printf("modelsPath=%s\n", modelsDir.c_str());
                    break;
                case 3:
                    imgScale = strtof(argv[i], NULL);
                    printf("scale=%f\n", imgScale);
                    break;
                case 4:
                    boxScoreThresh = strtof(argv[i], NULL);
                    printf("boxScoreThresh=%f\n", boxScoreThresh);
                    break;
                case 5:
                    boxThresh = strtof(argv[i], NULL);
                    printf("boxThresh=%f\n", boxThresh);
                    break;
                case 6:
                    minArea = strtof(argv[i], NULL);
                    printf("minArea=%f\n", minArea);
                    break;
                case 7:
                    angleScaleWidth = strtof(argv[i], NULL);
                    printf("angleScaleWidth=%f\n", angleScaleWidth);
                    break;
                case 8:
                    angleScaleHeight = strtof(argv[i], NULL);
                    printf("angleScaleHeight=%f\n", angleScaleHeight);
                    break;
                case 9:
                    textScaleWidth = strtof(argv[i], NULL);
                    printf("textScaleWidth=%f\n", textScaleWidth);
                    break;
                case 10:
                    textScaleHeight = strtof(argv[i], NULL);
                    printf("textScaleHeight=%f\n", textScaleHeight);
                    break;
            }
        }
        OcrLite ocrLite(modelsDir.c_str());
        ocrLite.detect(imgPath.c_str(), imgName.c_str(), imgScale, boxScoreThresh, boxThresh, minArea, angleScaleWidth,
                       angleScaleHeight, textScaleWidth, textScaleHeight);
    }

    return 0;
}
