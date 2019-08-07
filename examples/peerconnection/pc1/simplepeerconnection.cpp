#include "simplepeerconnection.h"

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

std::unique_ptr<rtc::Thread> SimplePeerConnection::s_worker_thread;
std::unique_ptr<rtc::Thread> SimplePeerConnection::s_signaling_thread;
rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
                                    SimplePeerConnection::s_peer_connection_factory;

SimplePeerConnection::SimplePeerConnection(QObject* parent)
    : QObject(parent)
{

}

SimplePeerConnection::~SimplePeerConnection()
{

}

bool SimplePeerConnection::InitPeerConnectionFactory()
{
    if (s_peer_connection_factory == nullptr) {
        s_worker_thread.reset(new rtc::Thread());
        s_worker_thread->Start();
        s_signaling_thread.reset(new rtc::Thread());
        s_signaling_thread->Start();

        s_peer_connection_factory = webrtc::CreatePeerConnectionFactory(
            s_worker_thread.get(), s_worker_thread.get(), s_signaling_thread.get(),
            nullptr, webrtc::CreateBuiltinAudioEncoderFactory(),
                    webrtc::CreateBuiltinAudioDecoderFactory(),
                    webrtc::CreateBuiltinVideoEncoderFactory(),
                    webrtc::CreateBuiltinVideoDecoderFactory(),
            nullptr, nullptr);
      }
    return s_peer_connection_factory;
}

void SimplePeerConnection::ClearPeerConnectionFactory()
{
    s_peer_connection_factory = nullptr;
    s_signaling_thread.reset();
    s_worker_thread.reset();
}

webrtc::VideoTrackInterface *SimplePeerConnection::GetVideoTrack()
{
    return video_track_;
}

webrtc::PeerConnectionInterface *SimplePeerConnection::GetPeerConnection()
{
    return peer_connection_;
}

bool SimplePeerConnection::CreatePeerConnection()
{
    RTC_DCHECK(s_peer_connection_factory);
    RTC_DCHECK(!peer_connection_);

    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = true;

    //webrtc::PeerConnectionInterface::IceServer server;
    //server.uri = "stun:stun.l.google.com:19302";
    //config.servers.push_back(server);

    peer_connection_ = s_peer_connection_factory->CreatePeerConnection(
        config, nullptr, nullptr, this);
    return peer_connection_;
}

void SimplePeerConnection::DeletePeerConnection()
{
    peer_connection_ = nullptr;
}

void SimplePeerConnection::CreateTracks()
{
    RTC_DCHECK(s_peer_connection_factory);
    audio_track_ = s_peer_connection_factory->CreateAudioTrack(
                kAudioLabel, s_peer_connection_factory->CreateAudioSource(cricket::AudioOptions()));

    std::unique_ptr<cricket::VideoCapturer> video_device = OpenVideoCaptureDevice();
    if (video_device) {
        video_track_ = s_peer_connection_factory->CreateVideoTrack(
                    kVideoLabel, s_peer_connection_factory->CreateVideoSource(
                        std::move(video_device), nullptr));
        //main_wnd_->StartLocalRenderer(video_track_);
    } else {
      RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
    }
}

void SimplePeerConnection::AddTracks()
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
}

void SimplePeerConnection::CreateOffer()
{
    RTC_DCHECK(peer_connection_);
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options =
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions();
    options.offer_to_receive_audio = 1;
    options.offer_to_receive_video = 1;
    peer_connection_->CreateOffer(this, options);
}

void SimplePeerConnection::SetIceCandidate(const webrtc::IceCandidateInterface *candidate)
{
    RTC_DCHECK(peer_connection_);
    peer_connection_->AddIceCandidate(candidate);
}

std::unique_ptr<cricket::VideoCapturer> SimplePeerConnection::OpenVideoCaptureDevice()
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
