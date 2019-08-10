#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include "media/base/videocapturer.h"

namespace Ui {
class Widget;
}

class Widget
        : public QWidget
        , public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    // VideoSinkInterface
    void OnFrame(const webrtc::VideoFrame& frame) override;

private Q_SLOTS:
    void on_startBtn_clicked();

    void on_stopBtn_clicked();

    void OpenVideoCaptureDevice();
    void CloseVideoCaptureDevice();

    void on_updateDeviceBtn_clicked();

private:
    Ui::Widget *ui;

    std::unique_ptr<cricket::VideoCapturer> video_capturer_;
};

#endif // WIDGET_H
