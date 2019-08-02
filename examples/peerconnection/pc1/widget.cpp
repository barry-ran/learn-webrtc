#include <QMessageBox>

#include "widget.h"
#include "ui_widget.h"

#include "peerconnectiona.h"
#include "peerconnectionb.h"

// unityplugin
//SimplePeerConnection

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , peer_connection_a_(new rtc::RefCountedObject<PeerConnectionA>())
    , peer_connection_b_(new rtc::RefCountedObject<PeerConnectionB>())
{
    ui->setupUi(this);
    // 模拟服务器将a的offer发送给b
    //connect(peer_connection_a_, &PeerConnectionA::CreateOffer,
    //        peer_connection_b_, &PeerConnectionB::OnRecvOffer);
}

Widget::~Widget()
{
    StopLocalRenderer();
    delete ui;
}

void Widget::StartLocalRenderer(webrtc::VideoTrackInterface* local_video) {
    local_renderer_.reset(new VideoRenderer(local_video, ui->localLabel));
}

void Widget::StopLocalRenderer() {
    local_renderer_.reset();
}

void Widget::on_startBtn_clicked()
{
    peer_connection_a_->CreateTracks();
    StartLocalRenderer(peer_connection_a_->GetVideoTrack());
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

    if (!peer_connection_a_->CreatePeerConnection()) {
        QMessageBox::warning(this, "Error", "CreatePeerConnection failed",
                             QMessageBox::Ok);
        peer_connection_a_->DeletePeerConnection();
        return;
    }
    peer_connection_a_->AddTracks();
    peer_connection_a_->CreateOffer();
}

void Widget::on_hangUpBtn_clicked()
{

}
