#include <QMessageBox>
#include <QDebug>

#include "peerconnectiona.h"

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "media/engine/webrtcvideocapturerfactory.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture_factory.h"
#include "rtc_base/checks.h"

const char kAudioLabel[] = "audio_label";
const char kVideoLabel[] = "video_label";
const char kStreamId[] = "stream_id";

PeerConnectionA::PeerConnectionA()
{

}

void PeerConnectionA::Start()
{
    RTC_DCHECK(InitializePeerConnectionFactory());
    CreateTracks();
}

void PeerConnectionA::Call()
{
    if (!peer_connection_factory_) {
        QMessageBox::warning(nullptr, "Error", "You should start first",
                             QMessageBox::Ok);
        return;
    }

    if (peer_connection_.get()) {
        QMessageBox::warning(nullptr, "Error", "We only support connecting to one peer at a time",
                             QMessageBox::Ok);
        return;
    }
    if (InitializePeerConnection()) {
      //peer_id_ = peer_id;
      webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options =
      webrtc::PeerConnectionInterface::RTCOfferAnswerOptions();
      options.offer_to_receive_audio = 1;
      options.offer_to_receive_video = 1;
      peer_connection_->CreateOffer(this, options);
      const webrtc::SessionDescriptionInterface* sdi = peer_connection_->local_description();
      //std::string tmp;
      //sdi->ToString(&tmp);
      //qDebug() << "local sdi >>>>" << tmp.c_str();
    } else {
        QMessageBox::warning(nullptr, "Error", "Failed to initialize PeerConnection",
                             QMessageBox::Ok);
    }
}

webrtc::VideoTrackInterface *PeerConnectionA::GetVideoTrack()
{
    return video_track_;
}

bool PeerConnectionA::InitializePeerConnectionFactory()
{
    RTC_DCHECK(!peer_connection_factory_);
    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
        nullptr /* network_thread */, nullptr /* worker_thread */,
        nullptr /* signaling_thread */, nullptr /* default_adm */,
        webrtc::CreateBuiltinAudioEncoderFactory(),
        webrtc::CreateBuiltinAudioDecoderFactory(),
        webrtc::CreateBuiltinVideoEncoderFactory(),
        webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
        nullptr /* audio_processing */);

    if (!peer_connection_factory_) {
        QMessageBox::warning(nullptr, "Error", "Failed to initialize PeerConnectionFactory",
                             QMessageBox::Ok);
        return false;
    }
    return true;
}

void PeerConnectionA::DeletePeerConnectionFactory()
{
    peer_connection_factory_ = nullptr;
}

bool PeerConnectionA::InitializePeerConnection()
{
    RTC_DCHECK(peer_connection_factory_);
    RTC_DCHECK(!peer_connection_);

    if (!CreatePeerConnection(/*dtls=*/true)) {
        QMessageBox::warning(nullptr, "Error", "CreatePeerConnection failed",
                             QMessageBox::Ok);
        DeletePeerConnection();
    }

    AddTracks();
    return peer_connection_ != nullptr;
}

void PeerConnectionA::DeletePeerConnection()
{
    peer_connection_ = nullptr;
}

bool PeerConnectionA::CreatePeerConnection(bool dtls)
{
    RTC_DCHECK(peer_connection_factory_);
    RTC_DCHECK(!peer_connection_);

    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = dtls;

    //webrtc::PeerConnectionInterface::IceServer server;
    //server.uri = "stun:stun.l.google.com:19302";
    //config.servers.push_back(server);

    peer_connection_ = peer_connection_factory_->CreatePeerConnection(
        config, nullptr, nullptr, this);
    return peer_connection_ != nullptr;
}

void PeerConnectionA::CreateTracks()
{
    RTC_DCHECK(peer_connection_factory_);
    audio_track_ = peer_connection_factory_->CreateAudioTrack(
                kAudioLabel,
                peer_connection_factory_->CreateAudioSource(cricket::AudioOptions()));

    std::unique_ptr<cricket::VideoCapturer> video_device = OpenVideoCaptureDevice();
    if (video_device) {
        video_track_ = peer_connection_factory_->CreateVideoTrack(
                    kVideoLabel, peer_connection_factory_->CreateVideoSource(
                        std::move(video_device), nullptr));
        //main_wnd_->StartLocalRenderer(video_track_);
    } else {
      RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
    }
}

void PeerConnectionA::AddTracks()
{
    RTC_DCHECK(peer_connection_);
    RTC_DCHECK(audio_track_);
    RTC_DCHECK(video_track_);

    if (!peer_connection_->GetSenders().empty()) {
        return;  // Already added tracks.
    }
    auto result_or_error = peer_connection_->AddTrack(audio_track_, {kStreamId});
    if (!result_or_error.ok()) {
        RTC_LOG(LS_ERROR) << "Failed to add audio track to PeerConnection: "
                          << result_or_error.error().message();
    }

    result_or_error = peer_connection_->AddTrack(video_track_, {kStreamId});
    if (!result_or_error.ok()) {
      RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
                        << result_or_error.error().message();
    }
    //main_wnd_->SwitchToStreamingUI();
}

std::unique_ptr<cricket::VideoCapturer> PeerConnectionA::OpenVideoCaptureDevice()
{
    std::vector<std::string> device_names;
    {
        std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
            webrtc::VideoCaptureFactory::CreateDeviceInfo());
        if (!info) {
          return nullptr;
        }
        int num_devices = info->NumberOfDevices();
        for (int i = 0; i < num_devices; ++i) {
          const uint32_t kSize = 256;
          char name[kSize] = {0};
          char id[kSize] = {0};
          if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
            device_names.push_back(name);
          }
        }
    }

    cricket::WebRtcVideoDeviceCapturerFactory factory;
    std::unique_ptr<cricket::VideoCapturer> capturer;
    for (const auto& name : device_names) {
      capturer = factory.Create(cricket::Device(name, 0));
      if (capturer) {
        break;
      }
    }
    return capturer;
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
    //peer_connection_->SetLocalDescription(
    //    DummySetSessionDescriptionObserver::Create(), desc);

    //std::string sdp;
    //desc->ToString(&sdp);
}

void PeerConnectionA::OnFailure(webrtc::RTCError error)
{
    qDebug() << ">>>>>>>>>>>>>>" << Q_FUNC_INFO;
}
