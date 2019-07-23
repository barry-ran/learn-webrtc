#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>

#include "modules/desktop_capture/desktop_capturer.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget,
        public webrtc::DesktopCapturer::Callback
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    // DesktopCapturer::Callback
    // Called after a frame has been captured. |frame| is not nullptr if and
    // only if |result| is SUCCESS.
    void OnCaptureResult(webrtc::DesktopCapturer::Result result,
                         std::unique_ptr<webrtc::DesktopFrame> frame) override;

private Q_SLOTS:
    void on_getWindowsBtn_clicked();

    void on_startCaptureBtn_clicked();

    void on_stopCaptureBtn_clicked();

    void on_sourceListComBox_currentIndexChanged(int index);

private:
    Ui::Widget *ui;

    std::unique_ptr<webrtc::DesktopCapturer> window_capturer_;
    std::unique_ptr<webrtc::DesktopCapturer> screen_capturer_;
    webrtc::DesktopCapturer::SourceId source_ = -1;
    QTimer* timer_ = new QTimer(this);
};

#endif // WIDGET_H
