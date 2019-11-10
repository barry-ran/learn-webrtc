#include <QDebug>
#include <QFile>

#include "widget.h"
#include "ui_widget.h"

#include "modules/desktop_capture/desktop_capture_options.h"
#include "rtc_base/checks.h"
#include <third_party/libyuv/include/libyuv.h>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //setWindowFlag(Qt::WindowMaximizeButtonHint);

    // step.1 创建DesktopCapturer
    // window
    // windowCapturer只有gdi一种方式
    // 某些窗口捕获不到画面（例如显存中的（js开发的桌面应用，opengl渲染的页面等）），全黑，gdi获取不到
    // todo 为什么chrome的web例子可以录制窗口？
    // https://webrtc.github.io/samples/src/content/getusermedia/getdisplaymedia/
    window_capturer_ = webrtc::DesktopCapturer::CreateWindowCapturer(webrtc::DesktopCaptureOptions::CreateDefault());
    RTC_DCHECK(window_capturer_);
    // step.2 设置回调
    window_capturer_->Start(this);

    // screen
    webrtc::DesktopCaptureOptions options = webrtc::DesktopCaptureOptions::CreateDefault();
    // magnification和directx只有ScreenCapturer才支持，windowCapturer只有gdi一种方式
    //options.set_allow_use_magnification_api(true);
    //options.set_allow_directx_capturer(true);
    screen_capturer_ = webrtc::DesktopCapturer::CreateScreenCapturer(options);
    RTC_DCHECK(screen_capturer_);
    screen_capturer_->Start(this);

    connect(timer_, &QTimer::timeout, this, [this](){
        // step.4 定时CaptureFrame()
        // 统计CaptureFrame耗时，超过16ms的话最好不要放在ui线程中
        if (ui->windowRadio->isChecked()) {
            window_capturer_->CaptureFrame();
        } else {
            screen_capturer_->CaptureFrame();
        }
    });

    connect(this, &Widget::recvFrame, this, &Widget::onRecvFrame, Qt::QueuedConnection);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame)
{
    // step.3 处理回调视频帧
    qDebug() << "result: " << (int)result;    
    if (webrtc::DesktopCapturer::Result::SUCCESS == result) {
        QMutexLocker locker(&mutex_);
        int width = frame->size().width();
        // width必须是8的整数倍，否则opengl解码crash（居然被我找到原因了）
        width &= ~7;
        int height = frame->size().height();
        if (width == 0) {
            width = 1;
        }
        if (!i420_buffer_.get() ||
                i420_buffer_->width() * i420_buffer_->height() != width * height) {
            i420_buffer_ = webrtc::I420Buffer::Create(width, height);
        }

        libyuv::ConvertToI420(frame->data(), 0, i420_buffer_->MutableDataY(),
                              i420_buffer_->StrideY(), i420_buffer_->MutableDataU(),
                              i420_buffer_->StrideU(), i420_buffer_->MutableDataV(),
                              i420_buffer_->StrideV(), 0, 0, frame->size().width(),
                              frame->size().height(), width,height,
                              libyuv::kRotate0, libyuv::FOURCC_ARGB);

        Q_EMIT recvFrame();
#if 0
        std::string filename("d:/capture_screen_");
        filename += std::to_string(width);
        filename += "x";
        filename += std::to_string(height);
        filename += ".yuv";
        QFile file(filename.c_str());
        //已读写方式打开文件，
        //如果文件不存在会自动创建文件
        if(file.open(QIODevice::WriteOnly | QIODevice::Append)){
            file.write((char*)i420_buffer_->MutableDataY(), i420_buffer_->StrideY() * height);
            file.write((char*)i420_buffer_->MutableDataU(), (i420_buffer_->StrideU() * height) >> 1);
            file.write((char*)i420_buffer_->MutableDataV(), (i420_buffer_->StrideV() * height) >> 1);
            //关闭文件
            file.close();
        }
#endif

#if 0
        // save rgba
        QFile file("test.rgba");
        //已读写方式打开文件，
        //如果文件不存在会自动创建文件
        if(file.open(QIODevice::WriteOnly | QIODevice::Append)){
            file.write((char*)frame->data(), frame->size().width() * frame->size().height() * 4);
            //关闭文件
            file.close();
        }
#endif
    }
}

void Widget::on_getWindowsBtn_clicked()
{
    webrtc::DesktopCapturer::SourceList sources;
    if (ui->windowRadio->isChecked()) {
        RTC_DCHECK(window_capturer_);
        RTC_DCHECK(window_capturer_->GetSourceList(&sources));
    } else {
        RTC_DCHECK(screen_capturer_);
        RTC_DCHECK(screen_capturer_->GetSourceList(&sources));
    }

    ui->sourceListComBox->clear();
    for (auto it = sources.begin(); it != sources.end(); ++it) {
        qDebug() << " id: " << it->id << "source list: title:" << it->title.c_str();
        QString title(it->title.c_str());
        if (title.isEmpty()) {
            title = QString::number(it->id);
        }
        ui->sourceListComBox->addItem(title, QVariant::fromValue(it->id));
    }
}

void Widget::on_startCaptureBtn_clicked()
{
    if (source_ == -1) {
        return;
    }
    if (timer_->isActive()) {
        timer_->stop();
    }
    timer_->start(50);

}

void Widget::on_stopCaptureBtn_clicked()
{
    timer_->stop();
}

void Widget::on_sourceListComBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    source_ = ui->sourceListComBox->currentData().toInt();
    qDebug() << "source_: " << source_;
    if (ui->windowRadio->isChecked()) {
        RTC_DCHECK(window_capturer_);
        window_capturer_->SelectSource(source_);
    } else {
        RTC_DCHECK(screen_capturer_);
        screen_capturer_->SelectSource(source_);
    }
}

void Widget::onRecvFrame()
{
    QMutexLocker locker(&mutex_);

    if (i420_buffer_) {
        ui->videoWidget->setFrameSize(QSize(i420_buffer_->width(), i420_buffer_->height()));
        ui->videoWidget->updateTextures(i420_buffer_->MutableDataY(), i420_buffer_->MutableDataU(), i420_buffer_->MutableDataV(),
                                        i420_buffer_->StrideY(), i420_buffer_->StrideU(), i420_buffer_->StrideV());
    }
}
