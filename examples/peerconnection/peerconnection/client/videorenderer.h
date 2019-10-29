#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

#include <QLabel>
#include "api/media_stream_interface.h"

class VideoRenderer : public QObject, public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
    Q_OBJECT
public:
    VideoRenderer(webrtc::VideoTrackInterface* track_to_render);
    virtual ~VideoRenderer();

Q_SIGNALS:
    void updateImage(QImage image);

protected:
    virtual void OnFrame(const webrtc::VideoFrame& frame);

private:
    rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_ = nullptr;    
};

#endif // VIDEORENDERER_H
