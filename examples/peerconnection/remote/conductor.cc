/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "conductor.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/types/optional.h"
#include "api/audio/audio_mixer.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/audio_options.h"
#include "api/create_peerconnection_factory.h"
#include "api/rtp_sender_interface.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "defaults.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "p2p/base/port_allocator.h"
#include "pc/video_track_source.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/ref_counted_object.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "platform_video_capturer.h"

namespace {
// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

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

}  // namespace

class CapturerTrackSource : public webrtc::VideoTrackSource {
public:
    static rtc::scoped_refptr<CapturerTrackSource> Create() {
        const size_t kWidth = 640;
        const size_t kHeight = 480;
        const size_t kFps = 30;
        const size_t kDeviceIndex = 0;
        std::unique_ptr<webrtc::test::TestVideoCapturer> capturer;

        capturer = webrtc::test::CreateVideoCapturer(kWidth, kHeight, kFps, kDeviceIndex, true);
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

Conductor::Conductor(PeerConnectionClient* client, MainWindow* main_wnd)
    : peer_id_(-1), loopback_(false), client_(client), main_wnd_(main_wnd) {
    client_->RegisterObserver(this);
    main_wnd->RegisterObserver(this);
}

Conductor::~Conductor() {
    RTC_DCHECK(!peer_connection_);
}

bool Conductor::connection_active() const {
    return peer_connection_ != nullptr;
}

void Conductor::Close() {
    client_->SignOut();
    DeletePeerConnection();
}

bool Conductor::InitializePeerConnection() {
    RTC_DCHECK(!peer_connection_factory_);
    RTC_DCHECK(!peer_connection_);
#if defined(Q_OS_MAC)
    network_thread_ = rtc::Thread::CreateWithSocketServer();
    network_thread_->Start();
    worker_thread_ = rtc::Thread::Create();
    worker_thread_->Start();
    signaling_thread_ = rtc::Thread::Create();
    signaling_thread_->Start();
#endif

    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
            #if defined(Q_OS_MAC)
                network_thread_.get(), worker_thread_.get(), signaling_thread_.get(),
            #elif defined(Q_OS_WIN)
                nullptr, nullptr, nullptr,
            #endif
                nullptr /* default_adm */,
                webrtc::CreateBuiltinAudioEncoderFactory(),
                webrtc::CreateBuiltinAudioDecoderFactory(),
                webrtc::CreateBuiltinVideoEncoderFactory(),
                webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
                nullptr /* audio_processing */);

    if (!peer_connection_factory_) {
        main_wnd_->MessageBox("Error", "Failed to initialize PeerConnectionFactory",
                              true);
        DeletePeerConnection();
        return false;
    }

    if (!CreatePeerConnection(/*dtls=*/true)) {
        main_wnd_->MessageBox("Error", "CreatePeerConnection failed", true);
        DeletePeerConnection();
    }

    AddTracks();

    return peer_connection_ != nullptr;
}

bool Conductor::ReinitializePeerConnectionForLoopback() {
    loopback_ = true;
    std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders =
            peer_connection_->GetSenders();
    peer_connection_ = nullptr;
    if (CreatePeerConnection(/*dtls=*/false)) {
        for (const auto& sender : senders) {
            peer_connection_->AddTrack(sender->track(), sender->stream_ids());
        }
        peer_connection_->CreateOffer(
                    this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    }
    return peer_connection_ != nullptr;
}

bool Conductor::CreatePeerConnection(bool dtls) {
    RTC_DCHECK(peer_connection_factory_);
    RTC_DCHECK(!peer_connection_);

    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = dtls;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = GetPeerConnectionString();
    config.servers.push_back(server);

    peer_connection_ = peer_connection_factory_->CreatePeerConnection(
                config, nullptr, nullptr, this);
    return peer_connection_ != nullptr;
}

void Conductor::DeletePeerConnection() {
    main_wnd_->StopLocalRenderer();
    main_wnd_->StopRemoteRenderer();
    peer_connection_ = nullptr;
    data_channel_ = nullptr;
    peer_connection_factory_ = nullptr;
    peer_id_ = -1;
    loopback_ = false;
}

void Conductor::EnsureStreamingUI() {
    RTC_DCHECK(peer_connection_);
    if (main_wnd_->IsWindow()) {
        if (main_wnd_->current_ui() != MainWindow::STREAMING)
            main_wnd_->SwitchToStreamingUI();
    }
}

//
// PeerConnectionObserver implementation.
//

void Conductor::OnAddTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
        streams) {
    RTC_LOG(INFO) << __FUNCTION__ << " " << receiver->id();
    main_wnd_->QueueUIThreadCallback(NEW_TRACK_ADDED,
                                     receiver->track().release());
}

void Conductor::OnRemoveTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
    RTC_LOG(INFO) << __FUNCTION__ << " " << receiver->id();
    main_wnd_->QueueUIThreadCallback(TRACK_REMOVED, receiver->track().release());
}

void Conductor::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
    RTC_LOG(INFO) << __FUNCTION__ << "PeerConnectionObserver::DataChannel("
                  << channel << ", " << data_channel_.get() << ")";

    data_channel_ = channel;
    data_channel_->RegisterObserver(this);
}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
    RTC_LOG(INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();
    // For loopback test. To save some connecting delay.
    if (loopback_) {
        if (!peer_connection_->AddIceCandidate(candidate)) {
            RTC_LOG(WARNING) << "Failed to apply the received candidate";
        }
        return;
    }

    QJsonObject jsonObject;
    jsonObject.insert(kCandidateSdpMidName, candidate->sdp_mid().c_str());
    jsonObject.insert(kCandidateSdpMlineIndexName, candidate->sdp_mline_index());

    std::string sdp;
    if (!candidate->ToString(&sdp)) {
        RTC_LOG(LS_ERROR) << "Failed to serialize candidate";
        return;
    }
    jsonObject.insert(kCandidateSdpName, sdp.c_str());

    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObject);
    SendMessage(jsonDoc.toJson().data());
}

//
// PeerConnectionClientObserver implementation.
//

void Conductor::OnSignedIn() {
    RTC_LOG(INFO) << __FUNCTION__;
    main_wnd_->SwitchToPeerList(client_->peers());
}

void Conductor::OnDisconnected() {
    RTC_LOG(INFO) << __FUNCTION__;

    DeletePeerConnection();

    if (main_wnd_->IsWindow())
        main_wnd_->SwitchToConnectUI();
}

void Conductor::OnPeerConnected(int id, const std::string& name) {
    RTC_LOG(INFO) << __FUNCTION__;
    // Refresh the list if we're showing it.
    if (main_wnd_->current_ui() == MainWindow::LIST_PEERS)
        main_wnd_->SwitchToPeerList(client_->peers());
}

void Conductor::OnPeerDisconnected(int id) {
    RTC_LOG(INFO) << __FUNCTION__;
    if (id == peer_id_) {
        RTC_LOG(INFO) << "Our peer disconnected";
        main_wnd_->QueueUIThreadCallback(PEER_CONNECTION_CLOSED, NULL);
    } else {
        // Refresh the list if we're showing it.
        if (main_wnd_->current_ui() == MainWindow::LIST_PEERS)
            main_wnd_->SwitchToPeerList(client_->peers());
    }
}

void Conductor::OnMessageFromPeer(int peer_id, const std::string& message) {
    RTC_DCHECK(peer_id_ == peer_id || peer_id_ == -1);
    RTC_DCHECK(!message.empty());

    if (!peer_connection_.get()) {
        RTC_DCHECK(peer_id_ == -1);
        peer_id_ = peer_id;

        if (!InitializePeerConnection()) {
            RTC_LOG(LS_ERROR) << "Failed to initialize our PeerConnection instance";
            client_->SignOut();
            return;
        }
    } else if (peer_id != peer_id_) {
        RTC_DCHECK(peer_id_ != -1);
        RTC_LOG(WARNING)
                << "Received a message from unknown peer while already in a "
                   "conversation with a different peer.";
        return;
    }

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(message.c_str(), &json_error);
    if(json_error.error != QJsonParseError::NoError) {
        RTC_LOG(WARNING) << "Received unknown message. " << message;
        return;
    }

    std::string type_str;
    std::string json_object;

    if (parse_doucment.isObject()) {
        QJsonObject obj = parse_doucment.object();
        if(obj.contains(kSessionDescriptionTypeName))  {
            QJsonValue name_value = obj.take(kSessionDescriptionTypeName);
            if(name_value.isString()) {
                type_str = name_value.toString().toStdString();
            }
        }
    }

    if (!type_str.empty()) {
        if (type_str == "offer-loopback") {
            // This is a loopback call.
            // Recreate the peerconnection with DTLS disabled.
            if (!ReinitializePeerConnectionForLoopback()) {
                RTC_LOG(LS_ERROR) << "Failed to initialize our PeerConnection instance";
                DeletePeerConnection();
                client_->SignOut();
            }
            return;
        }
        absl::optional<webrtc::SdpType> type_maybe =
                webrtc::SdpTypeFromString(type_str);
        if (!type_maybe) {
            RTC_LOG(LS_ERROR) << "Unknown SDP type: " << type_str;
            return;
        }
        webrtc::SdpType type = *type_maybe;
        std::string sdp;
        if (parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains(kSessionDescriptionSdpName))  {
                QJsonValue name_value = obj.take(kSessionDescriptionSdpName);
                if(name_value.isString()) {
                    sdp = name_value.toString().toStdString();
                }
            }
        }
        if (sdp.empty()) {
            RTC_LOG(WARNING) << "Can't parse received session description message.";
            return;
        }

        webrtc::SdpParseError error;
        std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
                webrtc::CreateSessionDescription(type, sdp, &error);
        if (!session_description) {
            RTC_LOG(WARNING) << "Can't parse received session description message. "
                             << "SdpParseError was: " << error.description;
            return;
        }
        RTC_LOG(INFO) << " Received session description :" << message;
        peer_connection_->SetRemoteDescription(
                    DummySetSessionDescriptionObserver::Create(),
                    session_description.release());
        if (type == webrtc::SdpType::kOffer) {
            peer_connection_->CreateAnswer(
                        this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
        }
    } else {
        std::string sdp_mid;
        int sdp_mlineindex = -1;
        std::string sdp;
        if (parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains(kCandidateSdpMidName))  {
                QJsonValue name_value = obj.take(kCandidateSdpMidName);
                if(name_value.isString()) {
                    sdp_mid = name_value.toString().toStdString();
                }
            }
        }
        if (parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains(kCandidateSdpMlineIndexName))  {
                QJsonValue name_value = obj.take(kCandidateSdpMlineIndexName);
                if(name_value.isDouble()) {
                    sdp_mlineindex = name_value.toInt();
                }
            }
        }
        if (parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains(kCandidateSdpName))  {
                QJsonValue name_value = obj.take(kCandidateSdpName);
                if(name_value.isString()) {
                    sdp = name_value.toString().toStdString();
                }
            }
        }
        if (sdp_mid.empty() || sdp.empty() || -1 == sdp_mlineindex) {
            RTC_LOG(WARNING) << "Can't parse received message.";
            return;
        }
        webrtc::SdpParseError error;
        std::unique_ptr<webrtc::IceCandidateInterface> candidate(
                    webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error));
        if (!candidate.get()) {
            RTC_LOG(WARNING) << "Can't parse received candidate message. "
                             << "SdpParseError was: " << error.description;
            return;
        }
        if (!peer_connection_->AddIceCandidate(candidate.get())) {
            RTC_LOG(WARNING) << "Failed to apply the received candidate";
            return;
        }
        RTC_LOG(INFO) << " Received candidate :" << message;
    }
}

void Conductor::OnMessageSent(int err) {
    // Process the next pending message if any.
    main_wnd_->QueueUIThreadCallback(SEND_MESSAGE_TO_PEER, nullptr);
}

void Conductor::OnServerConnectionFailure() {
    main_wnd_->MessageBox("Error", ("Failed to connect to " + server_).c_str(),
                          true);
}

//
// MainWndCallback implementation.
//

void Conductor::StartLogin(const std::string& server, int port) {
    if (client_->is_connected())
        return;
    server_ = server;
    client_->Connect(server, port, GetPeerName());
}

void Conductor::DisconnectFromServer() {
    if (client_->is_connected())
        client_->SignOut();
}

void Conductor::ConnectToPeer(int peer_id) {
    RTC_DCHECK(peer_id_ == -1);
    RTC_DCHECK(peer_id != -1);

    if (peer_connection_.get()) {
        main_wnd_->MessageBox(
                    "Error", "We only support connecting to one peer at a time", true);
        return;
    }

    if (InitializePeerConnection()) {
        peer_id_ = peer_id;

        // 必须在CreateOffer之前，因为CreateOffer中将datachannel信息带出去的
        webrtc::DataChannelInit config;
        config.ordered = true;
        config.reliable = true;
        data_channel_ = peer_connection_->CreateDataChannel("data_channel", &config);
        data_channel_->RegisterObserver(this);

        peer_connection_->CreateOffer(
                    this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    } else {
        main_wnd_->MessageBox("Error", "Failed to initialize PeerConnection", true);
    }
}

void Conductor::AddTracks() {
    if (!peer_connection_->GetSenders().empty()) {
        return;  // Already added tracks.
    }

    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
                peer_connection_factory_->CreateAudioTrack(
                    kAudioLabel, peer_connection_factory_->CreateAudioSource(
                        cricket::AudioOptions())));
    auto result_or_error = peer_connection_->AddTrack(audio_track, {kStreamId});
    if (!result_or_error.ok()) {
        RTC_LOG(LS_ERROR) << "Failed to add audio track to PeerConnection: "
                          << result_or_error.error().message();
    }

    video_device_ =
            CapturerTrackSource::Create();
    if (video_device_) {
        rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
                    peer_connection_factory_->CreateVideoTrack(kVideoLabel, video_device_));
        main_wnd_->StartLocalRenderer(video_track_);

        result_or_error = peer_connection_->AddTrack(video_track_, {kStreamId});
        if (!result_or_error.ok()) {
            RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
                              << result_or_error.error().message();
        }
    } else {
        RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
    }

    main_wnd_->SwitchToStreamingUI();
}

void Conductor::DataChannelSend(const QByteArray& data)
{
    if (data_channel_) {
        webrtc::DataBuffer buffer(rtc::CopyOnWriteBuffer(data.data(), data.length()), true);
        data_channel_->Send(buffer);
    }
}

void Conductor::OnStateChange()
{
    RTC_LOG(INFO) << __FUNCTION__;
}

void Conductor::OnMessage(const webrtc::DataBuffer &buffer)
{
    RTC_LOG(INFO) << __FUNCTION__;
    RTC_LOG(INFO) << std::string(buffer.data.data<char>(), buffer.data.size());
}

void Conductor::OnBufferedAmountChange(uint64_t sent_data_size)
{
    RTC_LOG(INFO) << __FUNCTION__ << ":"
                  << "DataChannelObserver::BufferedAmountChange(" << sent_data_size << ")";
}

void Conductor::DisconnectFromCurrentPeer() {
    RTC_LOG(INFO) << __FUNCTION__;
    if (peer_connection_.get()) {
        client_->SendHangUp(peer_id_);
        DeletePeerConnection();
    }

    if (main_wnd_->IsWindow())
        main_wnd_->SwitchToPeerList(client_->peers());
}

void Conductor::UIThreadCallback(int msg_id, void* data) {
    switch (msg_id) {
    case PEER_CONNECTION_CLOSED:
        RTC_LOG(INFO) << "PEER_CONNECTION_CLOSED";
        DeletePeerConnection();

        if (main_wnd_->IsWindow()) {
            if (client_->is_connected()) {
                main_wnd_->SwitchToPeerList(client_->peers());
            } else {
                main_wnd_->SwitchToConnectUI();
            }
        } else {
            DisconnectFromServer();
        }
        break;

    case SEND_MESSAGE_TO_PEER: {
        RTC_LOG(INFO) << "SEND_MESSAGE_TO_PEER";
        std::string* msg = reinterpret_cast<std::string*>(data);
        if (msg) {
            // For convenience, we always run the message through the queue.
            // This way we can be sure that messages are sent to the server
            // in the same order they were signaled without much hassle.
            pending_messages_.push_back(msg);
        }

        if (!pending_messages_.empty() && !client_->IsSendingMessage()) {
            msg = pending_messages_.front();
            pending_messages_.pop_front();

            if (!client_->SendToPeer(peer_id_, *msg) && peer_id_ != -1) {
                RTC_LOG(LS_ERROR) << "SendToPeer failed";
                DisconnectFromServer();
            }
            delete msg;
        }

        if (!peer_connection_.get())
            peer_id_ = -1;

        break;
    }

    case NEW_TRACK_ADDED: {
        auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(data);
        if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
            auto* video_track = static_cast<webrtc::VideoTrackInterface*>(track);
            main_wnd_->StartRemoteRenderer(video_track);
        }
        track->Release();
        break;
    }

    case TRACK_REMOVED: {
        // Remote peer stopped sending a track.
        auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(data);
        track->Release();
        break;
    }

    default:
        RTC_NOTREACHED();
        break;
    }
}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
    peer_connection_->SetLocalDescription(
                DummySetSessionDescriptionObserver::Create(), desc);

    std::string sdp;
    desc->ToString(&sdp);

    // For loopback test. To save some connecting delay.
    if (loopback_) {
        // Replace message type from "offer" to "answer"
        std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
                webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, sdp);
        peer_connection_->SetRemoteDescription(
                    DummySetSessionDescriptionObserver::Create(),
                    session_description.release());
        return;
    }

    QJsonObject jsonObject;
    jsonObject.insert(kSessionDescriptionTypeName, webrtc::SdpTypeToString(desc->GetType()));
    jsonObject.insert(kSessionDescriptionSdpName, sdp.c_str());

    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObject);
    SendMessage(jsonDoc.toJson().data());
}

void Conductor::OnFailure(webrtc::RTCError error) {
    RTC_LOG(LERROR) << ToString(error.type()) << ": " << error.message();
}

void Conductor::SendControlMsg(const QByteArray& data)
{
    DataChannelSend(data);
}

void Conductor::SendMessage(const std::string& json_object) {
    std::string* msg = new std::string(json_object);
    main_wnd_->QueueUIThreadCallback(SEND_MESSAGE_TO_PEER, msg);
}
