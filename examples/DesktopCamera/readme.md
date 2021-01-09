[DesktopCapturer-image]: https://raw.githubusercontent.com/barry-ran/learn_webrtc/master/examples/DesktopCapturer/screenshot/main.png

![界面][DesktopCapturer-image]

# DesktopCapturer
在webrtc桌面捕获模块中主要包括两部分，屏幕捕获和窗口捕获：

- 桌面捕获，使用了三种方案，可自行设置使用哪种方案：
    1. gdi
    2. directx
    3. magnifier
- 窗口捕获，只有一种方案，且有bug
    1. GetWindowDc + PrintWindow

    这种方法没法获取硬件加速窗口或者opengl等绘制的显存窗口（例如Electron开发的客户端，qtwebengine窗口等），获取到一片黑
    
    可以使用magnifier方案，然后将目标窗口以外的所有窗口（包括桌面）都忽略掉，达到目标窗口捕获的效果

# 使用方法
    1. 创建DesktopCapturer
    2. 设置回调处理函数
    3. 定时CaptureFrame()捕获
    4. 回调函数中处理回调数据（回调数据为rgba）

# 参考链接
[视频直播：Windows中各类画面源的截取和合成方法总结](https://www.jianshu.com/p/0bbb9c4be735)

[若干种窗口画面的捕获方法](https://blog.csdn.net/felicityWSH/article/details/62218390)
