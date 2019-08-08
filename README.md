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
## win
- 可以科学上网（首要前提）
- git
- vs2017（安装时手动选择ATL、MFC和Windows 10 SDK(10.0.17134.0)）

    也可以给安装包指定命令行参数来安装(vs.exe为vs安装包，在这里[下载](https://visualstudio.microsoft.com/zh-hans/downloads/))
    ```
    vs.exe --add Microsoft.VisualStudio.Workload.NativeDesktop --add Microsoft.VisualStudio.Component.VC.ATLMFC --add Microsoft.VisualStudio.Component.Windows10SDK.17134 --includeRecommended

    ```

    注意：
    - Windows 10 SDK必须安装10.0.17134.0版本，webrtc编译工具链里强制要求，不过webrtc也有计划移除版本要求 # TODO(crbug.com/773476): remove version requirement.
    - win10 sdk需要安装了Debugging Tools For Windows，没有安装的话

        进入 控制面板\程序\程序和功能，选择Windows Software Development Kit，右键更改，选择Change，勾选Debugging Tools For Windows，点击Change    

# 编译步骤
## win
- 执行syc_for_win.bat同步webrtc代码(开发过程只需同步一次即可)
- 执行build_for_win.bat编译
