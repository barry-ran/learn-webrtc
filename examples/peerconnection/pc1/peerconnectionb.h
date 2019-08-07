#ifndef PEERCONNECTIONB_H
#define PEERCONNECTIONB_H

#include "simplepeerconnection.h"

class PeerConnectionB : public SimplePeerConnection
{
    Q_OBJECT
public:
    PeerConnectionB(QObject *parent = nullptr);
    virtual ~PeerConnectionB() override;

Q_SIGNALS:
    void CreateAnswered(QString type, QString sdp);
    void AddTracked(webrtc::VideoTrackInterface* vieo_brack);

public Q_SLOTS:
    void OnRecvOffer(QString type, QString sdp);

protected:
    void OnAddTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
            streams) override;
    void OnRemoveTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
    void OnIceConnectionChange(
        webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(webrtc::RTCError error) override;
};

#endif // PEERCONNECTIONB_H
