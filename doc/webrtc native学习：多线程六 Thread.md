
rtc::Thread 为 WebRTC内部实现的线程类，在 WebRTC中有广泛的应用，WebRTC内部 network thread、 worker thread、signal thread 等均要求为此线程类实例；

为了线程安全，在某些功能模块的使用上，有要求其必需在指定的线程中才能调用的基本要求，比如音频模块：ADM 的创建必须要在 WebRTC 的 worker thread 中进行。

# 接口介绍
rtc::Thread 继承自消息队列rtc::MessageQueue ，内部提供了丰富的创建、管理接口，其借助于 线程局部存储/线程局部静态变量 实现线程的安全调用，以及是否是当前线程的有效判断。

![](rtc-thread.png)

# RTC_DISALLOW_COPY_AND_ASSIGN

RTC_DISALLOW_COPY_AND_ASSIGN 和 RTC_DISALLOW_ASSIGN 配合使用，通过禁用类的拷贝构造函数 和 = 操作符，提高代码的安全性。

# rtc::Thread 成员
```
// 继承自此类的子类，必须在其析构中主动调用 Stop 方法
class RTC_LOCKABLE Thread : public MessageQueue {
 public:
  // 已弃用的默认构造函数，不要使用，应该使用下面的静态方法进行创建类实例
  // 建议在开发过程中使用 CreateWithSocketServer 创建
  Thread();

  explicit Thread(SocketServer* ss);
  explicit Thread(std::unique_ptr<SocketServer> ss);

  // 所有的子类均必须保证在其析构函数中或者之前明确调用 Stop() 方法
  // 这样做是为了避免 Thread::PreRun 调用 Run() 方法时，析构函数对虚函数表的修改
  ~Thread() override;

  // rtc::Thread 提供的创建类实例的静态方法
  static std::unique_ptr<Thread> CreateWithSocketServer();
  static std::unique_ptr<Thread> Create();
  
  // 返回当前线程的实例指针
  static Thread* Current();

  // 单实例类，在 Thread 内部作为一个友元类，辅助 Thread 用于避免在指定的作用域中同步调用 Invoke 方法
  // 如果发生了同步方法的调用，则会触发一个断言
  class ScopedDisallowBlockingCalls {
   public:
    ScopedDisallowBlockingCalls();
    ~ScopedDisallowBlockingCalls();
   private:
    Thread* const thread_;
    const bool previous_state_;
  };

  // 判断调用者所在线程是否处于本线程实例中
  bool IsCurrent() const;

  // 将线程睡眠指定的毫秒数，默认返回 true，除非被 POSIX 发送的信号终止，它才会返回 false
  static bool SleepMs(int millis);

  // 设置线程名字，它必须在 调用 Start() 之前调用，这个方法是为了方便调试时查看
  // 如果 @param obj 为空，它将追加到 @param name 后面
  const std::string& name() const { return name_; }
  bool SetName(const std::string& name, const void* obj);

  // 开始执行此线程，如果 runnable 不为空，则thread直接运行runnable，否则启动包含消息队列的多路信号分离器功能的线程
  // 一般传入 nullptr，即表示启动包含消息队列的多路信号分离器功能的线程
  bool Start(Runnable* runnable = nullptr);

  // 通知线程结束，并阻塞等待；如果调用继承的 Quit 方法，则只会结束消息队列循环，而不会结束线程
  // 注意：一定不要在线程内部调用此方法    
  virtual void Stop();

  // 默认情况下，将调用 ProcessMessages(kForever),如果要做其它工作，则需要重写 Run()
  // 如果需要接受或者处理消息，则需要自己主动调用 ProcessMessages 方法
  virtual void Run();

  // 向消息队列发消息，并阻塞等待其执行完毕
  virtual void Send(const Location& posted_from,
                    MessageHandler* phandler,
                    uint32_t id = 0,
                    MessageData* pdata = nullptr);

  // 在Invoke调用线程保证functor方法被this线程调用，并且Invoke调用线程阻塞等待this线程执行完functor才继续执行，相当于消息的同步通知，即 Send（内部就是用Send函数实现的）
  template <class ReturnT, class FunctorT>
  ReturnT Invoke(const Location& posted_from, const FunctorT& functor) {
    FunctorMessageHandler<ReturnT, FunctorT> handler(functor);
    InvokeInternal(posted_from, &handler);
    return handler.MoveResult();
  }

  // 继承与 MessageQueue
  void Clear(MessageHandler* phandler,
             uint32_t id = MQID_ANY,
             MessageList* removed = nullptr) override;
  void ReceiveSends() override;

  // ProcessMessages will process I/O and dispatch messages until:
  //  1) cms milliseconds have elapsed (returns true)
  //  2) Stop() is called (returns false)
  bool ProcessMessages(int cms);

  // 判断此线程是否是我们自己通过标准的构造函数进行创建的，如果是就返回 true
  // 如果是通过 ThreadManager::WrapCurrentThread() 创建的则返回 false
  // 不能对 IsOwned 返回 false 线程对象调用 Start
  bool IsOwned();

  // 获取平台相关的线程句柄和 Id
#if defined(WEBRTC_WIN)
  HANDLE GetHandle() const {
    return thread_;
  }
  DWORD GetId() const {
    return thread_id_;
  }
#elif defined(WEBRTC_POSIX)
  pthread_t GetPThread() {
    return thread_;
  }
#endif

  // Expose private method running() for tests.
  //
  // DANGER: this is a terrible public API.  Most callers that might want to
  // call this likely do not have enough control/knowledge of the Thread in
  // question to guarantee that the returned value remains true for the duration
  // of whatever code is conditionally executing because of the return value!
  bool RunningForTest() { return running(); }

  // Sets the per-thread allow-blocking-calls flag and returns the previous
  // value. Must be called on this thread.
  bool SetAllowBlockingCalls(bool allow);

  // These functions are public to avoid injecting test hooks. Don't call them
  // outside of tests.
  // This method should be called when thread is created using non standard
  // method, like derived implementation of rtc::Thread and it can not be
  // started by calling Start(). This will set started flag to true and
  // owned to false. This must be called from the current thread.
  bool WrapCurrent();
  void UnwrapCurrent();

 protected:
  // Same as WrapCurrent except that it never fails as it does not try to
  // acquire the synchronization access of the thread. The caller should never
  // call Stop() or Join() on this thread.
  void SafeWrapCurrent();

  // Blocks the calling thread until this thread has terminated.
  void Join();

  static void AssertBlockingIsAllowedOnCurrentThread();

  friend class ScopedDisallowBlockingCalls;

 private:
  struct ThreadInit {
    Thread* thread;
    Runnable* runnable;
  };

#if defined(WEBRTC_WIN)
  static DWORD WINAPI PreRun(LPVOID context);
#else
  static void *PreRun(void *pv);
#endif

  // ThreadManager calls this instead WrapCurrent() because
  // ThreadManager::Instance() cannot be used while ThreadManager is
  // being created.
  // The method tries to get synchronization rights of the thread on Windows if
  // |need_synchronize_access| is true.
  bool WrapCurrentWithThreadManager(ThreadManager* thread_manager,
                                    bool need_synchronize_access);

  // Return true if the thread was started and hasn't yet stopped.
  bool running() { return running_.Wait(0); }

  // Processes received "Send" requests. If |source| is not null, only requests
  // from |source| are processed, otherwise, all requests are processed.
  void ReceiveSendsFromThread(const Thread* source);

  // If |source| is not null, pops the first "Send" message from |source| in
  // |sendlist_|, otherwise, pops the first "Send" message of |sendlist_|.
  // The caller must lock |crit_| before calling.
  // Returns true if there is such a message.
  bool PopSendMessageFromThread(const Thread* source, _SendMessage* msg);

  void InvokeInternal(const Location& posted_from, MessageHandler* handler);

  std::list<_SendMessage> sendlist_;
  std::string name_;
  Event running_;  // Signalled means running.

#if defined(WEBRTC_POSIX)
  pthread_t thread_;
#endif

#if defined(WEBRTC_WIN)
  HANDLE thread_;
  DWORD thread_id_;
#endif

  bool owned_;
  bool blocking_calls_allowed_;  // By default set to |true|.

  friend class ThreadManager;

  RTC_DISALLOW_COPY_AND_ASSIGN(Thread);  // 禁用拷贝构造和操作运算符 =
};
```

# 应用示例

- 创建实例
    ```
    shared_ptr<rtc::Thread> _worker_thread_ptr(std::move(rtc::Thread::CreateWithSocketServer()));
    shared_ptr<rtc::Thread> _signal_thread_ptr(std::move(rtc::Thread::CreateWithSocketServer()));
    _worker_thread_ptr->Start();
    _signal_thread_ptr->Start();
    ......
    _worker_thread_ptr->Stop();
    _signal_thread_ptr->Stop();
    ```

- 创建音频模块 ADM
    ```
    rtc::scoped_refptr<webrtc::AudioDeviceModule> _adm_ptr =
    _worker_thread_ptr->Invoke<rtc::scoped_refptr<webrtc::AudioDeviceModule>>(RTC_FROM_HERE, [] {
        //create adm
        return webrtc::AudioDeviceModule::Create(0,
            webrtc::AudioDeviceModule::AudioLayer::kWindowsCoreAudio);
    });
    ```

# webrtc中的应用
总体来说webrtc分两大线程分别为signaling thread和 worker thread。signaling thread主要负责事件和状态，worker thread主要跟各种数据流相关。
为此webrtc也是煞费苦心，比如：
```
/*android\webrtc\src\talk\app\webrtc\peerconnectionfactory.cc*/
bool PeerConnectionFactory::Initialize() {
  RTC_DCHECK(signaling_thread_->IsCurrent());
  rtc::InitRandom(rtc::Time());

  default_allocator_factory_ = PortAllocatorFactory::Create(worker_thread_);
  if (!default_allocator_factory_)
    return false;
/*
  Invoke就是在worker_thread_线程中调用了CreateMediaEngine_w方法,
  在webrtc库中有很多类似的用法
*/
cricket::MediaEngineInterface* media_engine =
      worker_thread_->Invoke<cricket::MediaEngineInterface*>(rtc::Bind(
      &PeerConnectionFactory::CreateMediaEngine_w, this));

  channel_manager_.reset(
      new cricket::ChannelManager(media_engine, worker_thread_));

  channel_manager_->SetVideoRtxEnabled(true);
  if (!channel_manager_->Init()) {
    return false;
  }

  dtls_identity_store_ = new RefCountedDtlsIdentityStore(
      signaling_thread_, worker_thread_);

  return true;
}
```

通常来说有如下规律：

- OnSomeMethod，以on命名的函数表示是一个信号的槽函数。
- SomeMethod_w 以”_w”结尾的函数表示此函数将在worker thread线程中运行
- SignalSomeName 信号的申明，可以调用槽函数

# 补充说明
1. Thread::Invoke的作用是当前线程将任务同步交给目标线程执行，内部调用Send实现，Send和Invoke都是同步，post系列是异步
2. 创建Thread的时候，如果没有传SocketServer参数，Thread会自己创建一个，用来作为多路事件分离的触发器，这也是我们为什么可以直接