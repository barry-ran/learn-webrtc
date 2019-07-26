#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include "api/scoped_refptr.h"
#include "videorenderer.h"

namespace Ui {
class Widget;
}

class PeerConnectionA;
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
    void StopLocalRenderer();

private Q_SLOTS:
    void on_startBtn_clicked();

    void on_callBtn_clicked();

    void on_hangUpBtn_clicked();

private:
    Ui::Widget *ui;

    rtc::scoped_refptr<PeerConnectionA> peer_connection_a_ = nullptr;
    std::unique_ptr<VideoRenderer> local_renderer_ = nullptr;
};

#endif // WIDGET_H
