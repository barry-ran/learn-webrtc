#include <QMessageBox>
#include <QDebug>

#include "peerconnectiona.h"

PeerConnectionA::PeerConnectionA(QObject *parent)
    : SimplePeerConnection(parent)
{

}

PeerConnectionA::~PeerConnectionA()
{

}

void PeerConnectionA::OnRecvAnswer(QString type, QString sdp)
{
    RTC_DCHECK(peer_connection_);
    std::string type_str = type.toStdString();
    std::string sdpStr = sdp.toStdString();

    // create session description
    absl::optional<webrtc::SdpType> type_maybe =
            webrtc::SdpTypeFromString(type_str);
    webrtc::SdpType sdpType = *type_maybe;

    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
            webrtc::CreateSessionDescription(sdpType, sdpStr, &error);

    // set remote session description
    peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(),
                                           session_description.release());
}

void PeerConnectionA::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface> > &streams)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}

void PeerConnectionA::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}

void PeerConnectionA::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}

void PeerConnectionA::OnIceCandidate(const webrtc::IceCandidateInterface *candidate)
{
    // 模拟网络发送IceCandidated
    Q_EMIT OnIceCandidated(candidate);
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}

void PeerConnectionA::OnSuccess(webrtc::SessionDescriptionInterface *desc)
{    
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;    
    peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);
    std::string sdp;
    desc->ToString(&sdp);

    // 模拟网络发送sdp
    Q_EMIT CreateOffered(webrtc::SdpTypeToString(desc->GetType()), sdp.c_str());
}

void PeerConnectionA::OnFailure(webrtc::RTCError error)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}
