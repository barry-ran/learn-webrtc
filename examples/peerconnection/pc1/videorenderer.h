#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

#include "api/mediastreaminterface.h"

class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
public:
    VideoRenderer(webrtc::VideoTrackInterface* track_to_render);

protected:
    virtual void OnFrame(const webrtc::VideoFrame& frame);

private:
    rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_ = nullptr;
};

#endif // VIDEORENDERER_H
