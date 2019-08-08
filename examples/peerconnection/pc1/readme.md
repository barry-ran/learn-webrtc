[PeerConnection-introduce]: https://github.com/barry-ran/learn_webrtc/blob/master/doc/webrtc%20native%E5%AD%A6%E4%B9%A0%EF%BC%9APeerConnection%E4%BB%8B%E7%BB%8D.md

[pc1-image]: https://raw.githubusercontent.com/barry-ran/learn_webrtc/master/examples/
peerconnection/pc1/screenshot/main.jpg

[pc1-sequence-image]: https://raw.githubusercontent.com/barry-ran/learn_webrtc/master/doc/image/pc1.jpg

![界面][pc1-image]

# pc1
不用服务器的情况下，建立两个PeerConnection的P2P连接

通过[PeerConnection介绍][PeerConnection-introduce]我们知道，要想建立两个PeerConnection的P2P连接，需要交换两种信息：
1. SDP：媒体会话描述信息，包括分辨率和编码格式等，用来协调匹配两个Peer的媒体编解码等信息
2. IceCandidate：ICE相关信息，包括候选地址、网络连接情况等，用来实现NAT穿透

在本例中我们两个PeerConnection位于同一个应用中，没有使用STUN服务器和Signal服务器，没有STUN服务器的话，PeerConnection只可以获得局域网内的ice候选地址，这对于本例足够了；没有Signal服务器的话，我们需要手动传递上述两种信息；在例子中我们使用Qt的信号槽来传递SDP和IceCandidate信息。

![时序图][pc1-sequence-image]

