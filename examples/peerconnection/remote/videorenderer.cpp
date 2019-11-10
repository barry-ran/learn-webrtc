#include<QDebug>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>

#include "videorenderer.h"
#include "api/video/i420_buffer.h"
#include "third_party/libyuv/include/libyuv/convert_argb.h"

VideoRenderer::VideoRenderer(webrtc::VideoTrackInterface* track_to_render)
    : rendered_track_(track_to_render)
{
    rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants());
}

VideoRenderer::~VideoRenderer()
{
    rendered_track_->RemoveSink(this);
}

void VideoRenderer::lock()
{
    mutex_.lock();
}

void VideoRenderer::unlock()
{
    mutex_.unlock();
}

webrtc::I420BufferInterface *VideoRenderer::getBuffer()
{
    return i420_buffer_;
}

void VideoRenderer::OnFrame(const webrtc::VideoFrame &frame)
{
    //TimeConsum tc(Q_FUNC_INFO);
    AutoLock<VideoRenderer> lock(this);
    i420_buffer_ = frame.video_frame_buffer()->ToI420();

    //qDebug() << Q_FUNC_INFO << ">>>>>>>frame: " << frame.width() << frame.height() << frame.size();    
    if (frame.rotation() != webrtc::kVideoRotation_0) {
      i420_buffer_ = webrtc::I420Buffer::Rotate(*i420_buffer_, frame.rotation());
    }

    Q_EMIT recvFrame();
}
