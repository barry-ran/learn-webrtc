#include <QMessageBox>
#include <QDebug>

#include "peerconnectiona.h"

class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
 public:
  static DummySetSessionDescriptionObserver* Create() {
    return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
  }
  virtual void OnSuccess() { RTC_LOG(INFO) << __FUNCTION__; }
  virtual void OnFailure(webrtc::RTCError error) {
    RTC_LOG(INFO) << __FUNCTION__ << " " << ToString(error.type()) << ": "
                  << error.message();
  }
};

PeerConnectionA::PeerConnectionA(QObject *parent)
    : QObject (parent)
{

}

PeerConnectionA::~PeerConnectionA()
{

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
