# peerconnection server简介
这里介绍的是webrtc自带的example中peerconnection的server

简单来说，server就是一个基于http的转发服务器：
1. 通过http协议（长连接）与客户端交互
2. 保存所有客户端信息，在客户端之间转发消息

# 关键类说明
## SocketBase
封装了socket操作，主要是创建/销毁socket

## DataSocket
继承SocketBase，封装了http相关操作：
1. 收发数据
2. 解析http字段
3. 获取/比较http各个字段（http header，method等）

## ListeningSocket
继承SocketBase，分装了监听socket的操作，主要是listen监听和accept接收连接

## ChannelMember
代表一个客户端，封装了客户端相关数据，包括连接状态，id，name，相关DataSocket等

还封装了转发DataSocket data数据到其他ChannelMember的操作

## PeerChannel
通过vector保存管理所有连接的ChannelMember客户端，主要操作有：添加ChannelMember，关闭所有ChannelMember，查找目标ChannelMember等

# 主要流程
1. 创建一个ListeningSocket监听指定端口
2. 创建一个PeerChannel用于管理所有客户端
3. select监听所有socket的事件
4. 如果是ListeningSocket上的连接事件，则创建一个新的DataSocket，并监听其事件
5. 如果是DataSocket上的事件，则收取数据，判断连接状态
6. 如果是新连接的客户端则创建ChannelMember并添加到PeerChannel中
7. 如果是已登录的客户端，则根据其事件类型处理消息（例如退出登录，转发消息给目标ChannelMember等）
