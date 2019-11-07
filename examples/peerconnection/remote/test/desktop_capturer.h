/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef DESKTOP_CAPTURER_H_
#define DESKTOP_CAPTURER_H_

#include "api/scoped_refptr.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "test_video_capturer.h"
#include "api/video/i420_buffer.h"
#include "rtc_base/thread.h"
#include "rtc_base/message_handler.h"


// https://my.oschina.net/u/988511/blog/3010478
namespace webrtc {
namespace test {

class DesktopCapturer : public TestVideoCapturer,
        public rtc::MessageHandler,
        public webrtc::DesktopCapturer::Callback {
public:
    static DesktopCapturer* Create(size_t width,
                                   size_t height,
                                   size_t target_fps,
                                   size_t capture_device_index);
    virtual ~DesktopCapturer() override;

    // DesktopCapturer::Callback
    // Called after a frame has been captured. |frame| is not nullptr if and
    // only if |result| is SUCCESS.
    void OnCaptureResult(webrtc::DesktopCapturer::Result result,
                         std::unique_ptr<webrtc::DesktopFrame> frame) override;

    virtual void OnMessage(rtc::Message* msg);

private:
    DesktopCapturer();
    bool Init(size_t width,
              size_t height,
              size_t target_fps,
              size_t capture_device_index);
    void Destroy();

    void CaptureFrame();

private:
    rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer_;
    std::unique_ptr<webrtc::DesktopCapturer> screen_capturer_;
};

}  // namespace test
}  // namespace webrtc

#endif  // DESKTOP_CAPTURER_H_
