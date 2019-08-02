#include <QDebug>

#include "peerconnectionb.h"

PeerConnectionB::PeerConnectionB(QObject *parent)
    : QObject(parent)
{

}

PeerConnectionB::~PeerConnectionB()
{

}

void PeerConnectionB::OnRecvOffer(QString type, QString sdp)
{

}

void PeerConnectionB::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface> > &streams)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}

void PeerConnectionB::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}

void PeerConnectionB::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}

void PeerConnectionB::OnIceCandidate(const webrtc::IceCandidateInterface *candidate)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}

void PeerConnectionB::OnSuccess(webrtc::SessionDescriptionInterface *desc)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
    /*
    peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);
    std::string sdp;
    desc->ToString(&sdp);

    // 模拟网络发送sdp
    Q_EMIT CreateOffer(webrtc::SdpTypeToString(desc->GetType()), sdp.c_str());
    */
}

void PeerConnectionB::OnFailure(webrtc::RTCError error)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}
