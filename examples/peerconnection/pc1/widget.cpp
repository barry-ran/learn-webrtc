#include <QMessageBox>

#include "widget.h"
#include "ui_widget.h"

#include "peerconnectiona.h"
#include "peerconnectionb.h"

// sdp交换流程
// a CreateOffer
// a OnSuccess
// a SetLocalDescription
// b SetRemoteDescription
// b answer
// b OnSuccess
// b SetLocalDescription
// a SetRemoteDescription

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , peer_connection_a_(new rtc::RefCountedObject<PeerConnectionA>())
    , peer_connection_b_(new rtc::RefCountedObject<PeerConnectionB>())
{
    ui->setupUi(this);
    // 模拟信令服务器将在a和b之间转发sdp信息和IceCandidate信息
    connect(peer_connection_a_, &PeerConnectionA::CreateOffered,
            peer_connection_b_, &PeerConnectionB::OnRecvOffer, Qt::DirectConnection);
    connect(peer_connection_b_, &PeerConnectionB::CreateAnswered,
            peer_connection_a_, &PeerConnectionA::OnRecvAnswer, Qt::DirectConnection);
    connect(peer_connection_a_, &SimplePeerConnection::OnIceCandidated,
            peer_connection_b_, &SimplePeerConnection::SetIceCandidate, Qt::DirectConnection);
    connect(peer_connection_b_, &SimplePeerConnection::OnIceCandidated,
            peer_connection_a_, &SimplePeerConnection::SetIceCandidate, Qt::DirectConnection);

    // 渲染b端的视频
    connect(peer_connection_b_, &PeerConnectionB::AddTracked, this,
            [this](webrtc::VideoTrackInterface* vieo_brack){
        StartRemoteRenderer(vieo_brack);
    }, Qt::DirectConnection);

    ui->callBtn->setEnabled(false);
    ui->hangUpBtn->setEnabled(false);
}

Widget::~Widget()
{
    StopLocalRenderer();
    StopRemoteRenderer();
    delete ui;
}

void Widget::StartLocalRenderer(webrtc::VideoTrackInterface* local_video) {
    local_renderer_.reset(new VideoRenderer(local_video));
    connect(local_renderer_.get(), &VideoRenderer::updateImage, this, &Widget::OnUpdateLocalImage, Qt::QueuedConnection);
}

void Widget::StopLocalRenderer() {
    local_renderer_.reset();
}

void Widget::StartRemoteRenderer(webrtc::VideoTrackInterface *remote_video)
{
    remote_renderer_.reset(new VideoRenderer(remote_video));
    connect(remote_renderer_.get(), &VideoRenderer::updateImage, this, &Widget::OnUpdateRemoteImage, Qt::QueuedConnection);
}

void Widget::StopRemoteRenderer()
{
    remote_renderer_.reset();
}

void Widget::on_startBtn_clicked()
{
    peer_connection_a_->CreateTracks();
    StartLocalRenderer(peer_connection_a_->GetVideoTrack());

    ui->startBtn->setEnabled(false);
    ui->callBtn->setEnabled(true);
}

void Widget::on_callBtn_clicked()
{
    if (!peer_connection_a_->GetVideoTrack()) {
        QMessageBox::warning(this, "Error", "You should start first",
                             QMessageBox::Ok);
        return;
    }

    if (peer_connection_a_->GetPeerConnection()) {
        QMessageBox::warning(this, "Error", "We only support connecting to one peer at a time",
                             QMessageBox::Ok);
        return;
    }

    // create peer a
    if (!peer_connection_a_->CreatePeerConnection()) {
        QMessageBox::warning(this, "Error", "CreatePeerConnection failed",
                             QMessageBox::Ok);
        peer_connection_a_->DeletePeerConnection();
        return;
    }

    // create peer b
    if (!peer_connection_b_->CreatePeerConnection()) {
        QMessageBox::warning(this, "Error", "CreatePeerConnection failed",
                             QMessageBox::Ok);
        peer_connection_a_->DeletePeerConnection();
        return;
    }

    // peer a send offer
    peer_connection_a_->AddTracks();
    peer_connection_a_->CreateOffer();

    ui->callBtn->setEnabled(false);
    ui->hangUpBtn->setEnabled(true);
}

void Widget::on_hangUpBtn_clicked()
{
    peer_connection_a_->DeletePeerConnection();
    peer_connection_b_->DeletePeerConnection();

    ui->callBtn->setEnabled(true);
    ui->hangUpBtn->setEnabled(false);
}

void Widget::OnUpdateRemoteImage(QImage image)
{
    ui->remoteLabel->setPixmap(QPixmap::fromImage(image.scaled(ui->remoteLabel->width(), ui->remoteLabel->height())));
}

void Widget::OnUpdateLocalImage(QImage image)
{
    ui->localLabel->setPixmap(QPixmap::fromImage(image.scaled(ui->localLabel->width(), ui->localLabel->height())));
}
