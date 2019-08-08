[webrtc标准](http://w3c.github.io/webrtc-pc/)
[优秀博客](https://blog.piasy.com/2018/05/24/WebRTC-Video-Native-Journey/index.html)

# 协议相关

# 编程相关

## Capturer
负责视频数据采集，只有视频才有这一层抽象，它有多种实现：
- 摄像头采集（Android 还有 Camera1/Camera2 两种实现）
- 屏幕采集
- 视频文件采集

## Source
媒体数据源抽象，用于向Track提供媒体数据，一个Source可以被多个Track共享。

主要有三个类：
- MediaSourceInterface 媒体源通用接口，主要判断媒体源的状态，是否是远程媒体
- VideoSourceInterface 视频源接口，主要提供了添加/移除VideoSink的操作
- AudioSourceInterface 音频源接口，继承自MediaSourceInterface，增加了设置音量、添加/移除AudioSink的操作

## Track
媒体数据轨道的抽象，媒体数据从轨道的一端（Source）流向另一端（Sink）。

本质上相当于Source的代理。Track使用Source构造，然后封装了Source的AddSink/RemoveSink接口

因为是代理的Source，所以Track分为两类：AudioTrack和VideoTrack

## Stream
Track容器，可以容纳多个Track

从主要的几个接口就可以看出来：AddTrack、RemoveTrack、GetAudioTracks、GetVideoTracks等

## Sink
媒体数据的消费者，用于处理音视频数据。

对于Video，Sink（VideoSinkInterface）用于处理VideoSource的视频数据，发送端的本地视频渲染、接收端收到远程视频后的渲染，都由Sink负责；

对于Audio，Sink（AudioTrackSinkInterface）用于处理AudioSource的音频数据；

Sink一般的使用流程如下（渲染视频数据为例）：
- 自定义VideoRenderer类继承rtc::VideoSinkInterface<webrtc::VideoFrame>
- 实现OnFrame接口来处理视频帧
- 调用VideoSourceInterface（或者它的子类，例如VideoTrackInterface）的AddOrUpdateSink将VideoRenderer添加到对应VideoSource上，VideoSource有了视频帧数据就会传递到VideoRenderer::OnFrame
- 停止渲染的时候调用VideoSourceInterface（或者它的子类，例如VideoTrackInterface）的RemoveSink将VideoRenderer从VideoSource上移除即可

## Sender 

## Receiver 

## Transceiver

## PeerConnection
它表示一个P2P连接其中的一端(Peer)。

我们知道webrtc是P2P的，所以PeerConnection是webrtc的门面，我们直接使用的都是PeerConnection的接口，它包含了建立p2p连接所需要的所有功能，主要包括设置sdp、创建ice通道、传输媒体数据，传输普通数据等。
