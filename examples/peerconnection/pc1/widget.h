#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include "api/scoped_refptr.h"
#include "videorenderer.h"

namespace Ui {
class Widget;
}

class PeerConnectionA;
class PeerConnectionB;
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
    void StopLocalRenderer();

    void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
    void StopRemoteRenderer();

private Q_SLOTS:
    void on_startBtn_clicked();

    void on_callBtn_clicked();

    void on_hangUpBtn_clicked();
    void onRecvLocalFrame();
    void onRecvRemoteFrame();

private:
    Ui::Widget *ui;

    rtc::scoped_refptr<PeerConnectionA> peer_connection_a_;
    rtc::scoped_refptr<PeerConnectionB> peer_connection_b_;
    std::unique_ptr<VideoRenderer> local_renderer_;
    std::unique_ptr<VideoRenderer> remote_renderer_;
};

#endif // WIDGET_H
