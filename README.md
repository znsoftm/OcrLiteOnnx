# OcrLiteOnnx

#### Release下载
[Gitee下载](https://gitee.com/benjaminwan/ocr-lite-onnx/releases)

#### 介绍
chineseocr lite onnx，超轻量级中文ocr demo，支持ncnn推理 (dbnet+anglenet+crnn)

**代码和模型均源自chineseocr lite的onnx分支**

详情请查看 [https://github.com/ouyanghuiyu/chineseocr_lite](https://github.com/ouyanghuiyu/chineseocr_lite)

采用ncnn神经网络前向计算框架[https://github.com/Tencent/ncnn](https://github.com/Tencent/ncnn)

整合了如下算法：
dbnet(图像分割)+anglenet(文字方向检测)+crnn(文字识别)

#### 编译环境
1. cmake 3.18.4
2. 内置的ncnn预编译库版本为 20200916 b766c8c
3. opencv动态库版本3.4.x以上

##### Windows编译说明
1.  Windows10 x64 /VS2019
2.  cmake请自行下载&配置
3.  下载opencv-3.4.11-vc14_vc15.exe，[下载地址](https://github.com/opencv/opencv/releases/tag/3.4.11)，把文件解压到项目根目录。解压后目录结构为OcrLiteOnnx/opencv/build
4.  VS2019安装时，至少选中"使用C++的桌面开发"
5.  开始菜单打开"x64 Native Tools Command Prompt for VS 2019"，并转到OcrLiteOnnx根目录
6.  运行```build-win.cmd```
7.  编译完成后运行```run-test-win.cmd```进行测试

##### Mac编译说明
1.  macOS Catalina 10.15.6
2.  下载opencv：```brew install opencv@3```
3.  编译：```./build.sh```
4.  测试：```./run-test.sh```

##### Linux编译说明
1.  Deepin 20 
2.  下载opencv：各发行版不大一样，略……
3.  编译：```./build.sh```
4.  测试：```./run-test.sh```

