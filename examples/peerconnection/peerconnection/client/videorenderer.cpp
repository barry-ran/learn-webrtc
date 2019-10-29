#include<QDebug>
#include<QImage>

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

void VideoRenderer::OnFrame(const webrtc::VideoFrame &frame)
{
    //qDebug() << Q_FUNC_INFO << ">>>>>>>frame: " << frame.width() << frame.height() << frame.size();
    rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(
                frame.video_frame_buffer()->ToI420());
    if (frame.rotation() != webrtc::kVideoRotation_0) {
      buffer = webrtc::I420Buffer::Rotate(*buffer, frame.rotation());
    }

    // 这段代码最浪费cpu，正式项目中可以使用opengl渲染优化掉
    QImage image(buffer->width(), buffer->height(), QImage::Format_ARGB32);
    libyuv::I420ToARGB(buffer->DataY(), buffer->StrideY(), buffer->DataU(),
                       buffer->StrideU(), buffer->DataV(), buffer->StrideV(),
                       image.bits(), buffer->width() * 32 / 8,
                       buffer->width(), buffer->height());
    // 这里传递image也浪费cpu
    Q_EMIT updateImage(image);
}
