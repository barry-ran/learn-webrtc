#ifndef PEERCONNECTIONA_H
#define PEERCONNECTIONA_H

#include "api/create_peerconnection_factory.h"
#include "api/mediastreaminterface.h"

class PeerConnectionA : public webrtc::PeerConnectionObserver,
        public webrtc::CreateSessionDescriptionObserver
{
public:
    PeerConnectionA();

    void Start();
    void Call();
    webrtc::VideoTrackInterface* GetVideoTrack();

protected:
    bool InitializePeerConnectionFactory();
    void DeletePeerConnectionFactory();
    bool InitializePeerConnection();
    void DeletePeerConnection();
    bool CreatePeerConnection(bool dtls);
    void CreateTracks();
    void AddTracks();
    std::unique_ptr<cricket::VideoCapturer> OpenVideoCaptureDevice();    

    //
    // PeerConnectionObserver implementation.
    //

    void OnSignalingChange(
        webrtc::PeerConnectionInterface::SignalingState new_state) override{}
    void OnAddTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
            streams) override;
    void OnRemoveTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
    void OnDataChannel(
        rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
    void OnRenegotiationNeeded() override {}
    void OnIceConnectionChange(
        webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
    void OnIceGatheringChange(
        webrtc::PeerConnectionInterface::IceGatheringState new_state) override{}
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    void OnIceConnectionReceivingChange(bool receiving) override {}

    // CreateSessionDescriptionObserver implementation.
    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(webrtc::RTCError error) override;

private:
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_ = nullptr;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_ = nullptr;
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track_ = nullptr;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_ = nullptr;
};

#endif // PEERCONNECTIONA_H
