# 什么是SDP
SDP（Session Description Protocol）是一种通用的会话描述协议，主要用来描述多媒体会话，用途包括会话声明、会话邀请、会话初始化等。

WebRTC主要在连接建立阶段用到SDP，连接双方通过信令服务交换会话信息，包括音视频编解码器(codec)、主机候选地址、网络传输协议等。

下面先简单介绍下SDP的格式、常用属性，然后通过WebRTC连接建立过程生成的SDP实例进行进一步讲解。

# 协议格式说明
SDP的格式非常简单，由多个行组成，每个行都是如下格式。
```
<type>=<value>
```
其中：
- <type>：大小写敏感的一个字符，代表特定的属性，比如v代表版本；
- <value>：结构化文本，格式与属性类型有关，UTF8编码；
- =两边不允许存在空格；
- =*表示是可选的；

# 常见属性
以下面的SDP为例：
```
v=0
o=alice 2890844526 2890844526 IN IP4 host.anywhere.com
s=
c=IN IP4 host.anywhere.com
t=0 0
m=audio 49170 RTP/AVP 0
a=rtpmap:0 PCMU/8000
m=video 51372 RTP/AVP 31
a=rtpmap:31 H261/90000
m=video 53000 RTP/AVP 32
a=rtpmap:32 MPV/90000
```
## 协议版本号：v=
格式如下，注意，没有子版本号。
```
v=0
```

## 会话发起者：o
格式如下，其中，username、session-id、nettype、addrtype、unicast-address 一起，唯一标识一个会话。
```
o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
```
各字段含义如下：
- username：发起者的用户名，不允许存在空格，如果应用不支持用户名，则为-。
- sess-id：会话id，由应用自行定义，规范的建议是NTP(Network Time Protocol)时间戳。
- sess-version：会话版本，用途由应用自行定义，只要会话数据发生变化时（比如编码），sess-version随着递增就行。同样的，规范的建议是NTP时间戳。
- nettype：网络类型，比如IN表示Internet。
- addrtype：地址类型，比如IP4、IV6
- unicast-address：域名，或者IP地址。

## 会话名 s=
必选，有且仅有一个s=字段，且不能为空。如果实在没有有意义的会话名，可以赋一个空格，即s=。
```
s=<session name>
```

## 连接数据：c=
格式如下：
```
c=<nettype> <addrtype> <connection-address>
```
每个SDP至少需要包含一个会话级别的c=字段，或者在每个媒体描述后面各包含一个c=字段。（媒体描述后的c=会覆盖会话级别的c=）

- nettype：网络类型，比如IN，表示 Internet。
- addrtype：地址类型，比如IP4、IP6。
- connection-address：如果是广播，则为广播地址组；如果是单播，则为单播地址；
举例01：
```
c=IN IP4 224.2.36.42/127
```
举例02：
```
c=IN IP4 host.anywhere.com
```
## 媒体描述：m=
SDP可能同时包含多个媒体描述。格式如下：

```
m=<media> <port> <proto> <fmt> ...
```
其中：

- media：媒体类型。包括 video、audio、text、application、message等。
- port：传输媒体流的端口，具体含义取决于使用的网络类型（在c=中声明）和使用的协议(proto，在m=中声明)。
- proto：传输协议，具体含义取决于c=中定义的地址类型，比如c=是IP4，那么这里的传输协议运行在IP4之上。比如：
- UDP：传输层协议是UDP。
- RTP/AVP：针对视频、音频的RTP协议，跑在UDP之上。
- RTP/SAVP：针对视频、音频的SRTP协议，跑在UDP之上。
- fmt：媒体格式的描述，可能有多个。根据 proto 的不同，fmt 的含义也不同。比如 proto 为 RTP/SAVP 时，fmt 表示 RTP payload 的类型。如果有多个，表示在这次会话中，多种payload类型可能会用到，且第一个为默认的payload类型。

举例，下面表示媒体类型是视频，采用SRTP传输流媒体数据，且RTP包的类型可能是122、102...119，默认是122。
```
m=video 9 UDP/TLS/RTP/SAVPF 122 102 100 101 124 120 123 119
```
对于 RTP/SAVP，需要注意的是，payload type 又分两种类型：

- 静态类型：参考 RTP/AVP audio and video payload types。
- 动态类型：在a=fmtp:里进行定义。(a=为附加属性，见后面小节)

举例，下面的SDP中：

对于audio，111 是动态类型，表示opus/48000/2。

对于video，122 是动态类型，表示H264/90000。
```
m=audio 9 UDP/TLS/RTP/SAVPF 111 103 104 9 0 8 126
a=rtpmap:111 opus/48000/2
m=video 9 UDP/TLS/RTP/SAVPF 122 102 100 101 124 120 123 119
a=rtpmap:122 H264/90000
```

## 附加属性：a=
作用：用于扩展SDP。

有两种作用范围：会话级别(session-level)、媒体级别（media-level）。

- 媒体级别：媒体描述（m=）后面可以跟任意数量的 a= 字段，对媒体描述进行扩展。
- 会话级别：在第一个媒体字段(media field)前，添加的 a= 字段是会话级别的。

有如下两种格式：
```
a=<attribute>
a=<attribute>:<value>
```
格式1举例：
```
a=recvonly
```
格式2举例：
```
a=rtpmap:0 PCMU/8000
```

## 时间：t=
作用：声明会话的开始、结束时间。

格式如下：
```
t=<start-time> <stop-time>
```
如果<stop-time>是0，表示会话没有结束的边界，但是需要在<start-time>之后会话才是活跃(active)的。如果<start-time>是0，表示会话是永久的。

举例：
```
t=0 0
```
# SDP在webrtc中的使用
webrtc建立连接利用的是Offer/Answer模型，Offerer发给Answerer的请求消息称为请求offer，内容包括媒体流类型、各个媒体流使用的编码集，以及将要用于接收媒体流的IP和端口。Answerer收到offer之后，回复给Offerer的消息称为answer，内容包括要使用的媒体编码，是否接收该媒体流以及告诉Offerer其用于接收媒体流的IP和端口。

Offer/Answer模型包括两个实体，一个是请求主体Offerer，另外一个是响应实体Answerer，两个实体只是在逻辑上进行区分，在一定条件可以转换。

在WebRTC连接流程中，在创建PeerConnectionA后，就会去创建一个offerSDP，并设置为localSDP。通过signaling发送 PeerB。 peerB收到peerA的SDP后，把收到的SDP设置为RemoteSDP。在设置完成后，PeerB再生成AnswerSDP，设置为localSDP，通过signaling通道发送给PeerA，PeerA收到后AnswerSDP后，设置为RemoteSDP，以上流程完成了SDP的交换。

# WebRTC实例
下面例子来自腾讯云WebRTC服务的远端offer。

WebRTC中的SDP 是由一个会话层和多个媒体层组成的， 而对于每个媒体层，WebRTC 又将其细划为四部分，即媒体流、网络描述、安全描述和服务质量描述。
```
// sdp版本号为0
v=0
// o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
// 用户名为空，会话id是8100750360520823155，会话版本是2（后面如果有类似改变编码的操作，sess-version加1），地址类型为IP4，地址为127.0.0.1（这里可以忽略）
o=- 7595655801978680453 2 IN IP4 112.90.139.105
// 会话名为空
s=-
// 会话的起始时间，都为0表示没有限制
t=0 0
a=ice-lite
// 音频、视频的传输的传输采取多路复用，通过同一个RTP通道传输音频、视频，可以参考 https://tools.ietf.org/html/draft-ietf-mmusic-sdp-bundle-negotiation-54
a=group:BUNDLE 0 1
// WMS是WebRTC Media Stram的缩写，这里给Media Stream定义了一个唯一的标识符。一个Media Stream可以有多个track（video track、audio track），这些track就是通过这个唯一标识符关联起来的，具体见下面的媒体行(m=)以及它对应的附加属性(a=ssrc:)
// 可以参考这里 http://tools.ietf.org/html/draft-ietf-mmusic-msid
a=msid-semantic: WMS 5Y2wZK8nANNAoVw6dSAHVjNxrD1ObBM2kBPV

//=========================媒体层=========================

// m=<media> <port> <proto> <fmt> ...
// 本次会话有音频，端口为9（可忽略，端口9为Discard Protocol专用），采用UDP传输加密的RTP包，并使用基于SRTCP的音视频反馈机制来提升传输质量，111、103、104等是audio可能采用的编码（参见前面m=的说明）
m=audio 9 UDP/TLS/RTP/SAVPF 111 103 104 9 0 8 126

//=========================网络描述=========================

// 音频发送者的IP4地址，WebRTC采用ICE，这里的 0.0.0.0 可直接忽略
c=IN IP4 0.0.0.0
// RTCP采用的端口、IP地址（可忽略）
a=rtcp:9 IN IP4 0.0.0.0

//=========================视频安全描述=========================

// ice-ufrag、ice-pwd 分别为ICE协商用到的认证信息
a=ice-ufrag:58142170598604946
a=ice-pwd:71696ad0528c4adb02bb40e1
// DTLS协商过程的指纹信息
a=fingerprint:sha-256 7F:98:08:AC:17:6A:34:DB:CF:3B:EC:93:ED:57:3F:5A:9E:1F:4A:F3:DB:D5:BF:66:EE:17:58:E0:57:EC:1B:19
// 当前客户端在DTLS协商过程中，既可以作为客户端，也可以作为服务端，具体可参考 RFC4572
a=setup:actpass

//=========================视频流描述=========================

// 当前媒体行的标识符（在a=group:BUNDLE 0 1 这行里面用到，这里0表示audio）
a=mid:0
// RTP允许扩展首部，这里表示采用了RFC6464定义的针对audio的扩展首部，用来调节音量，比如在大型会议中，有多个音频流，就可以用这个来调整音频混流的策略
// 这里没有vad=1，表示不启用这个音量控制
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
// 表示既可以发送音频，也可以接收音频
a=sendrecv
// 表示启用多路复用，RTP、RTCP共用同个通道
a=rtcp-mux
// 下面几行都是对audio媒体行的补充说明（针对111），包括rtpmap、rtcp-fb、fmtp
// rtpmap：编解码器为opus，采样率是48000，2声道
a=rtpmap:111 opus/48000/2

//=========================服务质量描述=========================

// rtcp-fb：基于RTCP的反馈控制机制，可以参考 https://tools.ietf.org/html/rfc5124、https://webrtc.org/experiments/rtp-hdrext/transport-wide-cc-02/
a=rtcp-fb:111 transport-cc
a=rtcp-fb:111 nack
// 最小的音频打包时间
a=fmtp:111 minptime=20
// 跟前面的rtpmap类似
a=rtpmap:126 telephone-event/8000
// ssrc用来对媒体进行描述，格式为a=ssrc:<ssrc-id> <attribute>:<value>，具体可参考 RFC5576
// cname用来唯一标识媒体的数据源
a=ssrc:16864608 cname:YZcxBwerFFm6GH69
// msid后面带两个id，第一个是MediaStream的id，第二个是audio track的id（跟后面的mslabel、label对应）
a=ssrc:16864608 msid:5Y2wZK8nANNAoVw6dSAHVjNxrD1ObBM2kBPV 128f4fa0-81dd-4c3a-bbcd-22e71e29d178
a=ssrc:16864608 mslabel:5Y2wZK8nANNAoVw6dSAHVjNxrD1ObBM2kBPV
a=ssrc:16864608 label:128f4fa0-81dd-4c3a-bbcd-22e71e29d178
// 跟audio类似，不赘述
m=video 9 UDP/TLS/RTP/SAVPF 122 102 125 107 124 120 123 119
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:58142170598604946
a=ice-pwd:71696ad0528c4adb02bb40e1
a=fingerprint:sha-256 7F:98:08:AC:17:6A:34:DB:CF:3B:EC:93:ED:57:3F:5A:9E:1F:4A:F3:DB:D5:BF:66:EE:17:58:E0:57:EC:1B:19
a=setup:actpass
a=mid:1
a=extmap:2 urn:ietf:params:rtp-hdrext:toffset
a=extmap:3 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:4 urn:3gpp:video-orientation
a=extmap:5 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a=sendrecv
a=rtcp-mux
a=rtcp-rsize
a=rtpmap:122 H264/90000
a=rtcp-fb:122 ccm fir
a=rtcp-fb:122 nack
a=rtcp-fb:122 nack pli
a=rtcp-fb:122 goog-remb
a=rtcp-fb:122 transport-cc
a=fmtp:122 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f
a=rtpmap:102 rtx/90000
a=fmtp:102 apt=122
a=rtpmap:125 H264/90000
a=rtcp-fb:125 ccm fir
a=rtcp-fb:125 nack
a=rtcp-fb:125 nack pli
a=rtcp-fb:125 goog-remb
a=rtcp-fb:125 transport-cc
a=fmtp:125 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f
a=rtpmap:107 rtx/90000
a=fmtp:107 apt=125
a=rtpmap:124 H264/90000
a=rtcp-fb:124 ccm fir
a=rtcp-fb:124 nack
a=rtcp-fb:124 nack pli
a=rtcp-fb:124 goog-remb
a=rtcp-fb:124 transport-cc
a=fmtp:124 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=4d0032
a=rtpmap:120 rtx/90000
a=fmtp:120 apt=124
a=rtpmap:123 H264/90000
a=rtcp-fb:123 ccm fir
a=rtcp-fb:123 nack
a=rtcp-fb:123 nack pli
a=rtcp-fb:123 goog-remb
a=rtcp-fb:123 transport-cc
a=fmtp:123 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=640032
a=rtpmap:119 rtx/90000
a=fmtp:119 apt=123
a=ssrc-group:FID 33718809 50483271
a=ssrc:33718809 cname:ovaCctnHP9Asci9c
a=ssrc:33718809 msid:5Y2wZK8nANNAoVw6dSAHVjNxrD1ObBM2kBPV 1d7fc300-9889-4f94-9f35-c0bcc77a260d
a=ssrc:33718809 mslabel:5Y2wZK8nANNAoVw6dSAHVjNxrD1ObBM2kBPV
a=ssrc:33718809 label:1d7fc300-9889-4f94-9f35-c0bcc77a260d
a=ssrc:50483271 cname:ovaCctnHP9Asci9c
a=ssrc:50483271 msid:5Y2wZK8nANNAoVw6dSAHVjNxrD1ObBM2kBPV 1d7fc300-9889-4f94-9f35-c0bcc77a260d
a=ssrc:50483271 mslabel:5Y2wZK8nANNAoVw6dSAHVjNxrD1ObBM2kBPV
a=ssrc:50483271 label:1d7fc300-9889-4f94-9f35-c0bcc77a260d
```

# 写在后面
SDP协议格式本身很简单，难点一般在于应用层在不同场景下扩展出来的属性，以及不同扩展属性对应的含义。比如上面举的例子，扩展属性、属性值的说明分散在数十个RFC里，查找、理解都费了一番功夫。

如有错漏，敬请指出。

# 相关链接
[rfc4566](https://tools.ietf.org/html/rfc4566)
[WebRTC的带注释的示例SDP](https://datatracker.ietf.org/doc/draft-ietf-rtcweb-sdp/?include_text=1)
