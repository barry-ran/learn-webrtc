[PeerConnection-introduce]: https://raw.githubusercontent.com/barry-ran/learn_webrtc/master/doc/webrtc native学习：PeerConnection介绍.md

# peerconnection_client
webrtc PeerConnection的官方例子，需要配合peerconnection_server使用，peerconnection_server是一个Signal服务器。

通过[PeerConnection介绍](PeerConnection-introduce)我们知道，要想建立两个PeerConnection的P2P连接，需要交换两种信息：
1. SDP：媒体会话描述信息，包括分辨率和编码格式等，用来协调匹配两个Peer的媒体编解码等信息
2. IceCandidate：ICE相关信息，包括候选地址、网络连接情况等，用来实现NAT穿透

在本例中我们两个PeerConnection位于同一个局域网中，没有使用STUN服务器，没有STUN服务器的话，PeerConnection只可以获得局域网内的ice候选地址，这对于局域网中通信足够了；而peerconnection_server就是负责交换上述两种信息的Signal服务器

