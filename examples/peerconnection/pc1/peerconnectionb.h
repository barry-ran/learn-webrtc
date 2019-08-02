#ifndef PEERCONNECTIONB_H
#define PEERCONNECTIONB_H

#include <QObject>

#include "simplepeerconnection.h"

class PeerConnectionB
        : public QObject
        , public SimplePeerConnection
{
    Q_OBJECT
public:
    PeerConnectionB(QObject *parent = nullptr);
    virtual ~PeerConnectionB() override;

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
