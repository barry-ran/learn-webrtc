#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

#include <QLabel>
#include "api/media_stream_interface.h"

class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
public:
    VideoRenderer(webrtc::VideoTrackInterface* track_to_render, QLabel* label);
    virtual ~VideoRenderer();

protected:
    virtual void OnFrame(const webrtc::VideoFrame& frame);

private:
    rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_ = nullptr;
    QLabel* label_ = nullptr;
};

#endif // VIDEORENDERER_H
