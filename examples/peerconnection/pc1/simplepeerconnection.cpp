#include "simplepeerconnection.h"

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture_factory.h"
#include "rtc_base/checks.h"
#include "pc/video_track_source.h"
#include "test_video_capturer.h"
#include "platform_video_capturer.h"


const char kAudioLabel[] = "audio_label";
const char kVideoLabel[] = "video_label";
const char kStreamId[] = "stream_id";

std::unique_ptr<rtc::Thread> SimplePeerConnection::s_worker_thread;
std::unique_ptr<rtc::Thread> SimplePeerConnection::s_network_thread;
std::unique_ptr<rtc::Thread> SimplePeerConnection::s_signaling_thread;
rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
                                    SimplePeerConnection::s_peer_connection_factory;

class CapturerTrackSource : public webrtc::VideoTrackSource {
 public:
  static rtc::scoped_refptr<CapturerTrackSource> Create() {
    const size_t kWidth = 640;
    const size_t kHeight = 480;
    const size_t kFps = 30;
    const size_t kDeviceIndex = 0;
    std::unique_ptr<webrtc::test::TestVideoCapturer> capturer;

    capturer = webrtc::test::CreateVideoCapturer(kWidth, kHeight, kFps, kDeviceIndex);
    if (capturer) {
      return new rtc::RefCountedObject<CapturerTrackSource>(std::move(capturer));
    }

    return nullptr;
  }

 protected:
  explicit CapturerTrackSource(
      std::unique_ptr<webrtc::test::TestVideoCapturer> capturer)
      : VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {}

 private:
  rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
    return capturer_.get();
  }
  std::unique_ptr<webrtc::test::TestVideoCapturer> capturer_;
};

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
        s_network_thread = rtc::Thread::CreateWithSocketServer();
        s_network_thread->Start();
        s_worker_thread = rtc::Thread::Create();
        s_worker_thread->Start();
        s_signaling_thread = rtc::Thread::Create();
        s_signaling_thread->Start();
        // 1. signal thread 不能是rtc::Thread::Current()为啥？
        // 2. 三种线程的作用和区别

        s_peer_connection_factory = webrtc::CreatePeerConnectionFactory(
#if defined(Q_OS_MAC)
                    s_network_thread.get(), s_worker_thread.get(), s_signaling_thread.get(),
#elif defined(Q_OS_WIN)
                    nullptr, nullptr, nullptr,
#endif
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
    s_network_thread.reset();
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

    rtc::scoped_refptr<CapturerTrackSource> video_device =
          CapturerTrackSource::Create();

    if (video_device) {
        video_track_ =s_peer_connection_factory->CreateVideoTrack(kVideoLabel, video_device);
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
