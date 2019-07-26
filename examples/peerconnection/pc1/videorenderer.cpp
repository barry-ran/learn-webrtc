#include<QDebug>

#include "videorenderer.h"

VideoRenderer::VideoRenderer(webrtc::VideoTrackInterface* track_to_render)
    : rendered_track_(track_to_render)
{
    rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants());
}

void VideoRenderer::OnFrame(const webrtc::VideoFrame &frame)
{
    qDebug() << Q_FUNC_INFO << ">>>>>>>frame: " << frame.width() << frame.height() << frame.size();
}
