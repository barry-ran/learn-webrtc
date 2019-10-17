[TOC]

# src 根目录

## base chrome基础组件库，webrtc中没用到

## build 通用编译脚本

## build_overrides webrtc特定编译脚本

## buildtools 编译工具

## rtc_base webrtc基础组件库

## api webrtc主要的对外接口
包括peerconnection、mediastream、音视频、网络传输、ice、datachannel等等

## examples 实例代码

## modules 相对独立的功能模块

### desktop_capture 桌面捕获模块
提供屏幕捕获和窗口捕获功能，回调rgba元数据

## test 实用的测试代码，很有参考性
例如example/peerconnection中摄像头捕获就引用了这里封装的代码，这里还包括跨平台视频捕获（platform_video_capturer）等。

## testing 简单测试代码

## third_party 第三方库