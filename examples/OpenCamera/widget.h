#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMutex>
#include <QWaitCondition>

#include "modules/video_capture/video_capture.h"
#include "test_video_capturer.h"

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

Q_SIGNALS:
    void recvFrame(int width, int height, const quint8 *dataY, const quint8 *dataU, const quint8 *dataV, quint32 linesizeY, quint32 linesizeU, quint32 linesizeV);

private Q_SLOTS:
    void on_startBtn_clicked();

    void on_stopBtn_clicked();

    void OpenVideoCaptureDevice();
    void CloseVideoCaptureDevice();

    void on_updateDeviceBtn_clicked();

    void onRecvFrame(int width, int height, const quint8 *dataY, const quint8 *dataU, const quint8 *dataV, quint32 linesizeY, quint32 linesizeU, quint32 linesizeV);

private:
    Ui::Widget *ui;

    std::unique_ptr<webrtc::test::TestVideoCapturer> video_capturer_;

    QMutex m_mutex;
    QWaitCondition m_recvDataCond;
};

#endif // WIDGET_H
