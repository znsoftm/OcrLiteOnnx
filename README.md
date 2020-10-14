# OcrLiteOnnx

#### Demo下载(win、mac、linux)
[Gitee下载](https://gitee.com/benjaminwan/ocr-lite-onnx/releases)

#### 介绍
ChineseOcr Lite Onnx，超轻量级中文OCR Demo，支持ncnn推理(DBNet+AngleNet+CRNN)

**代码和模型均源自chineseocr lite的onnx分支**

详情请查看 [https://github.com/ouyanghuiyu/chineseocr_lite](https://github.com/ouyanghuiyu/chineseocr_lite)

采用ncnn神经网络前向计算框架[https://github.com/Tencent/ncnn](https://github.com/Tencent/ncnn)

整合了如下算法：
DBNet(图像分割)+AngleNet(文字方向检测)+CRNN(文字识别)

#### 编译环境
1. cmake 3.18.4
2. 内置的ncnn预编译库版本为 20200916 b766c8c
3. opencv动态库版本3.4.x以上

##### Windows编译说明
1.  Windows10 x64 /VS2019
2.  cmake请自行下载&配置
3.  下载opencv-3.4.11-vc14_vc15.exe，[下载地址](https://github.com/opencv/opencv/releases/tag/3.4.11)，把文件解压到项目根目录。解压后目录结构为
```
OcrLiteOnnx/opencv
│  LICENSE.txt
│  LICENSE_FFMPEG.txt
│  README.md.txt
├─build              
└─sources
```
4.  VS2019安装时，至少选中"使用C++的桌面开发"
5.  开始菜单打开"x64 Native Tools Command Prompt for VS 2019"，并转到OcrLiteOnnx根目录
6.  运行```build-win.cmd```
7.  编译完成后运行```run-test-win.cmd```进行测试，或直接运行OcrLiteOnnx.exe，识别test目录下的7张图片

##### Mac编译说明
1.  macOS Catalina 10.15.x
2.  自行下载安装HomeBrew
3.  下载opencv：```brew install opencv@3```
4.  libomp: ```brew install libomp```
5.  编译：```./build.sh```
6.  测试：```./run-test.sh```，或直接运行./OcrLiteOnnx，识别test目录下的7张图片

##### Linux编译说明
1.  Deepin 20 或其它发行版
2.  安装build-essential:略……
3.  下载opencv：各发行版不大一样，略……
4.  编译：```./build.sh```
5.  测试：```./run-test.sh```，或直接运行./OcrLiteOnnx，识别test目录下的7张图片

#### 测试结果说明
1.  *-part-x.jpg为分割后的图片
2.  *-debug-x.jpg为分割后的图片进行方向识别后，校准方向后的图片
3.  *-result.jpg为图像分割画框的结果
4.  *-result.txt为识别的最终结果
5.  最终结果包含：图像分割耗时、文字框分数、文字框坐标、文字方向索引、文字方向分数、文字识别结果、各个文字的分数、文字识别耗时、整张图片总耗时。