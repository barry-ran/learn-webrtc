[DesktopCapturer-image]: https://raw.githubusercontent.com/barry-ran/learn_webrtc/master/examples/DesktopCapturer/screenshot/main.png

[pc1-image]: https://raw.githubusercontent.com/barry-ran/learn_webrtc/master/examples/peerconnection/pc1/screenshot/main.jpg

# learn_webrtc
学习webrtc，基于webrtc native开发音视频工具

# example展示
## 桌面捕获
[![界面][DesktopCapturer-image]](https://github.com/barry-ran/learn_webrtc/tree/master/examples/DesktopCapturer)

## pc1
[![界面][pc1-image]](https://github.com/barry-ran/learn_webrtc/tree/master/examples/peerconnection/pc1)

# 环境要求
- 可以科学上网（首要前提）
- git

## mac
xcode

## win

### error msvc编译总是失败
m99 webrtc 4844以后不支持msvc编译器了
[webrtc支持的平台](https://webrtc.googlesource.com/src/+/4c29ca654b1100906943ea08b996f4265bc50d9a/g3doc/supported-platforms-and-compilers.md)
[msvc编译器的讨论](https://bugs.chromium.org/p/webrtc/issues/detail?id=14009)



- vs2012 C++开发环境（Windows 10 SDK(10.0.22621.0)）

    也可以给安装包指定命令行参数来安装(vs.exe为vs安装包，在这里[下载](https://visualstudio.microsoft.com/zh-hans/downloads/))
    ```
    vs.exe --add Microsoft.VisualStudio.Workload.NativeDesktop --add Microsoft.VisualStudio.Component.VC.ATLMFC --add Microsoft.VisualStudio.Component.Windows10SDK.20348 --includeRecommended

    ```

    注意：
    - Windows 10 SDK建议最新版本
    - win10 sdk需要安装了Debugging Tools For Windows(必须的，编译要求)，没有安装的话

        进入 控制面板\程序\程序和功能，选择Windows Software Development Kit，右键更改，选择Change，勾选Debugging Tools For Windows，点击Change    

# 编译步骤
## win
- 设置vs2022_install环境变量
    ```
    set vs2022_install=C:\Program Files\Microsoft Visual Studio\2022\Community
    ```
- 执行syc_for_win.bat同步webrtc代码(开发过程只需同步一次即可)
- 执行build_for_win.bat release编译

# 参考文档
- [webrtc官方编译说明](https://webrtc.github.io/webrtc-org/native-code/development/)
- [chromium编译官方说明](https://chromium.googlesource.com/chromium/src/+/master/docs/windows_build_instructions.md)
- [github actions编译webrtc](https://github.com/godotengine/webrtc-actions)
- [释放github actions更多空间](https://github.com/easimon/maximize-build-space)
- [ssh调试github actions](https://github.com/marketplace/actions/debugging-with-tmate)
- [webrtc C++包装](https://github.com/webrtc-sdk/libwebrtc)


