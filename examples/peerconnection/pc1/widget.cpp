#include "widget.h"
#include "ui_widget.h"

#include "api/mediastreaminterface.h"
#include "api/peerconnectioninterface.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "media/engine/webrtcvideocapturerfactory.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture_factory.h"
#include "rtc_base/checks.h"

// unityplugin
//SimplePeerConnection

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
          peer_connection_factory_;
    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
          nullptr /* network_thread */, nullptr /* worker_thread */,
          nullptr /* signaling_thread */, nullptr /* default_adm */,
          webrtc::CreateBuiltinAudioEncoderFactory(),
          webrtc::CreateBuiltinAudioDecoderFactory(),
          webrtc::CreateBuiltinVideoEncoderFactory(),
          webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
          nullptr /* audio_processing */);
}

Widget::~Widget()
{
    delete ui;
}
