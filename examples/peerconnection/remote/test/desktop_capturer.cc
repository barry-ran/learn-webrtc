/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "desktop_capturer.h"

#include <stdint.h>
#include <memory>

#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "third_party/libyuv/include/libyuv.h"
#include "modules/desktop_capture/desktop_capture_options.h"

namespace webrtc {
namespace test {

DesktopCapturer::DesktopCapturer()
    : i420_buffer_(nullptr)
    , screen_capturer_(nullptr)
{

}

bool DesktopCapturer::Init(size_t width,
                           size_t height,
                           size_t target_fps,
                           size_t capture_device_index) {
    webrtc::DesktopCaptureOptions options = webrtc::DesktopCaptureOptions::CreateDefault();
    // magnification和directx只有ScreenCapturer才支持，windowCapturer只有gdi一种方式
    //options.set_allow_use_magnification_api(true);
    //options.set_allow_directx_capturer(true);
    screen_capturer_ = webrtc::DesktopCapturer::CreateScreenCapturer(options);
    RTC_DCHECK(screen_capturer_);
    screen_capturer_->Start(this);

    webrtc::DesktopCapturer::SourceList sources;
    RTC_DCHECK(screen_capturer_);
    RTC_DCHECK(screen_capturer_->GetSourceList(&sources));
    RTC_DCHECK(!sources.empty());
    screen_capturer_->SelectSource(sources[0].id);

    CaptureFrame();
    return true;
}

DesktopCapturer* DesktopCapturer::Create(size_t width,
                                         size_t height,
                                         size_t target_fps,
                                         size_t capture_device_index) {
    std::unique_ptr<DesktopCapturer> vcm_capturer(new DesktopCapturer());
    if (!vcm_capturer->Init(width, height, target_fps, capture_device_index)) {
        RTC_LOG(LS_WARNING) << "Failed to create DesktopCapturer(w = " << width
                            << ", h = " << height << ", fps = " << target_fps
                            << ")";
        return nullptr;
    }
    return vcm_capturer.release();
}

void DesktopCapturer::Destroy() {
    rtc::Thread::Current()->Clear(this);
    screen_capturer_ = nullptr;
}

void DesktopCapturer::CaptureFrame()
{
    if (screen_capturer_) {
        screen_capturer_->CaptureFrame();
    }
    rtc::Thread::Current()->PostDelayed(RTC_FROM_HERE, 100, this, 0);
}

DesktopCapturer::~DesktopCapturer() {
    Destroy();
}

void DesktopCapturer::OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame)
{
    if (webrtc::DesktopCapturer::Result::SUCCESS == result) {
        int width = frame->size().width();
        int height = frame->size().height();
        if (!i420_buffer_.get() ||
                i420_buffer_->width() * i420_buffer_->height() != width * height) {
            i420_buffer_ = webrtc::I420Buffer::Create(width, height);
        }

        int a = libyuv::ConvertToI420(frame->data(), 0, i420_buffer_->MutableDataY(),
                                      i420_buffer_->StrideY(), i420_buffer_->MutableDataU(),
                                      i420_buffer_->StrideU(), i420_buffer_->MutableDataV(),
                                      i420_buffer_->StrideV(), 0, 0, width, height, width,
                                      height, libyuv::kRotate0, libyuv::FOURCC_ARGB);

        TestVideoCapturer::OnFrame(webrtc::VideoFrame(i420_buffer_, 0, rtc::TimeMillis(), webrtc::kVideoRotation_0));
        //qDebug() << "width: " << frame->size().width() << "height: " << frame->size().height();
    }
}

void DesktopCapturer::OnMessage(rtc::Message *msg)
{
    if (msg->message_id == 0) {
        CaptureFrame();
    }
}

}  // namespace test
}  // namespace webrtc
