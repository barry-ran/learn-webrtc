# thread

使用 WebRTC `rtc::Thread` 投递任务的示例。

> 若编译报错 `rtc::Runnable not found`，可改用 `std::function` / lambda 方式向线程投递任务。

```cpp
#include "rtc_base/thread.h"

class ThreadExample {
 public:
  void Start() {
    thread_.Start();
    // 使用 lambda 投递任务，替代 rtc::Runnable
    thread_.PostTask([this]() { DoWork(); });
  }

  void Stop() {
    thread_.Stop();
  }

 private:
  void DoWork() {
    // 在 rtc::Thread 中执行的任务
  }

  rtc::Thread thread_;
};
```
