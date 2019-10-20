#ifndef SIMPLEPEERCONNECTION_H
#define SIMPLEPEERCONNECTION_H

#include <QObject>

#include "api/create_peerconnection_factory.h"
#include "api/media_stream_interface.h"

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

class SimplePeerConnection : public QObject
        , public webrtc::PeerConnectionObserver
        , public webrtc::CreateSessionDescriptionObserver
{
    Q_OBJECT
public:
    SimplePeerConnection(QObject* parent = nullptr);
    ~SimplePeerConnection() override;

    static bool InitPeerConnectionFactory();
    static void ClearPeerConnectionFactory();
    webrtc::VideoTrackInterface* GetVideoTrack();
    webrtc::PeerConnectionInterface* GetPeerConnection();

    bool CreatePeerConnection();
    void DeletePeerConnection();
    void CreateTracks();
    void AddTracks();
    void CreateOffer();

Q_SIGNALS:
    void OnIceCandidated(const webrtc::IceCandidateInterface *candidate);

public Q_SLOTS:
    void SetIceCandidate(const webrtc::IceCandidateInterface *candidate);

protected:

protected:
    //
    // PeerConnectionObserver implementation.
    //
    void OnSignalingChange(
            webrtc::PeerConnectionInterface::SignalingState new_state) override {}
    void OnAddTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
            streams) override {}
    void OnRemoveTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override {}
    void OnDataChannel(
        rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
    void OnRenegotiationNeeded() override {}
    void OnIceConnectionChange(
        webrtc::PeerConnectionInterface::IceConnectionState new_state) override {}
    void OnIceGatheringChange(
        webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {}
    void OnIceConnectionReceivingChange(bool receiving) override {}

    // CreateSessionDescriptionObserver implementation.
    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {}
    void OnFailure(webrtc::RTCError error) override {}

protected:
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track_;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

    static std::unique_ptr<rtc::Thread> s_worker_thread;
    static std::unique_ptr<rtc::Thread> s_signaling_thread;
    static rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
                                        s_peer_connection_factory;
};

#endif // SIMPLEPEERCONNECTION_H
