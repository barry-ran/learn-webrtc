#include "widget.h"
#include "ui_widget.h"

#include "peerconnectiona.h"

// unityplugin
//SimplePeerConnection

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    peer_connection_a_(new rtc::RefCountedObject<PeerConnectionA>())
{
    ui->setupUi(this);
}

Widget::~Widget()
{
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
    peer_connection_a_->Start();
    StartLocalRenderer(peer_connection_a_->GetVideoTrack());
}

void Widget::on_callBtn_clicked()
{

}

void Widget::on_hangUpBtn_clicked()
{

}
