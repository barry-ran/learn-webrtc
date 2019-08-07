#include <QDebug>

#include "peerconnectionb.h"

PeerConnectionB::PeerConnectionB(QObject *parent)
    : SimplePeerConnection(parent)
{

}

PeerConnectionB::~PeerConnectionB()
{

}

void PeerConnectionB::OnRecvOffer(QString type, QString sdp)
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

    // send answer
    if (sdpType == webrtc::SdpType::kOffer) {
        peer_connection_->CreateAnswer(
                    this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    }
}

void PeerConnectionB::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface> > &streams)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
    auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(receiver->track().release());
    if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
        auto* video_track = static_cast<webrtc::VideoTrackInterface*>(track);
        Q_EMIT AddTracked(video_track);
    }
    track->Release();
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
    Q_EMIT OnIceCandidated(candidate);
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}

void PeerConnectionB::OnSuccess(webrtc::SessionDescriptionInterface *desc)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
    peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);
    std::string sdp;
    desc->ToString(&sdp);

    // 模拟网络发送sdp
    Q_EMIT CreateAnswered(webrtc::SdpTypeToString(desc->GetType()), sdp.c_str());
}

void PeerConnectionB::OnFailure(webrtc::RTCError error)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}
