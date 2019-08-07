#ifndef PEERCONNECTIONA_H
#define PEERCONNECTIONA_H

#include "simplepeerconnection.h"

class PeerConnectionA : public SimplePeerConnection

{
    Q_OBJECT
public:
    PeerConnectionA(QObject *parent = nullptr);
    virtual ~PeerConnectionA() override;

Q_SIGNALS:
    void CreateOffered(QString type, QString sdp);

public Q_SLOTS:
    void OnRecvAnswer(QString type, QString sdp);

protected:
    void OnAddTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
            streams) override;
    void OnRemoveTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
    void OnIceConnectionChange(
        webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
    // pc需要联网，才会收到这个回调
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(webrtc::RTCError error) override;
};

#endif // PEERCONNECTIONA_H
