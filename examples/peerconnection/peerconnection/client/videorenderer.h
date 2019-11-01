#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

#include <QLabel>
#include <QMutex>
#include <QTime>
#include <QDebug>

#include "api/media_stream_interface.h"
#include <api/video/i420_buffer.h>

class VideoRenderer : public QObject, public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
    Q_OBJECT
public:
    VideoRenderer(webrtc::VideoTrackInterface* track_to_render);
    virtual ~VideoRenderer();

    class TimeConsum
    {
    public:
        TimeConsum(const QString &log)
        : log_(log)
        {
            time_.start();

        }
        ~TimeConsum() {
            qDebug() << log_ << "consum time:" << time_.elapsed();
        }

    private:
        QTime time_;
        QString log_;
    };

    void lock();
    void unlock();
    webrtc::I420BufferInterface *getBuffer();

    template <typename T>
      class AutoLock {
       public:
        explicit AutoLock(T* obj) : obj_(obj) { obj_->lock(); }
        ~AutoLock() { obj_->unlock(); }

       protected:
        T* obj_;
      };

Q_SIGNALS:
    void recvFrame();

protected:
    virtual void OnFrame(const webrtc::VideoFrame& frame);

private:
    rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_ = nullptr;    
    QMutex mutex_;
    rtc::scoped_refptr<webrtc::I420BufferInterface> i420_buffer_;
};

#endif // VIDEORENDERER_H
