
webrtc/base/ criticalsection.h/criticalsection.cc 文件中包含了3个类 CriticalSection、CritScope 和 TryCritScope。这3个类对于有多线程编程经验的Windows开发人员来说都是非常容易理解的。基本上就是对 Win32 CriticalSection 的简单封装。

# CriticalSection
CriticalSection 主要实现了跨平台实现临界区的功能。在 Linux 平台上使用了 pthread 的 mutex 模拟 Win32 的 Critical Section。以下对比一下 API：

- InitializeCriticalSection

    pthread 的 mutex 初始化略比 Win32 的 CriticalSection 复杂一些。pthread_mutexattr_init 函数用来初始化一个 mutex 属性变量。pthread_mutexattr_settype 函数用来设置 mutex 的各种属性。在 CriticalSection 中为 mutex 设置了 PTHREAD_MUTEX_RECURSIVE。通过网上查询该属性被描述为：

    “如果一个线程对这种类型的互斥锁重复上锁，不会引起死锁，一个线程对这类互斥锁的多次重复上锁必须由这个线程来重复相同数量的解锁，这样才能解开这个互斥锁，别的线程才能得到这个互斥锁。如果试图解锁一个由别的线程锁定的互斥锁将会返回一个错误代码。如果一个线程试图解锁已经被解锁的互斥锁也将会返回一个错误代码。这种类型的互斥锁只能是进程私有的（作用域属性PTHREAD_PROCESS_PRIVATE ）。”

    通过设置不同的属性 pthread mutex 可以实现很多不同的特性。如果读者有兴趣可以上网查询一下不同的属性的意义。不过需要注意到的是在 Event 类中没有 mutex 属性，直接使用默认值创建了mutex。接着使用 CriticalSection 使用 pthread_mutex_init 函数创建了mutex。最后请不要忘记调用pthread_mutexattr_destroy 将 mutex 属性变量销毁掉。

- DeleteCriticalSection

    pthread_mutex_destroy 函数用以销毁 mutex。

- EnterCriticalSection

    pthread_mutex_lock 函数用以对 mutex 加锁。

- TryEnterCriticalSection

    pthread_mutex_trylock 函数用以尝试对 mutex 加锁，如果加锁成功就返回 true，失败就返回 false。

- LeaveCriticalSection
    
    pthread_mutex_unlock 函数用以解锁 mutex.

    在 CriticalSection 中有个调试宏 CS_TRACK_OWNER。如果使用了这个宏， CriticalSection 可以使用 CurrentThreadIsOwner 函数用来判断是否是在当前线程加锁。在的段落重点提到过 CriticalSection使用 PTHREAD_MUTEX_RECURSIVE 属性来创建 mutex，该属性导致在另一个线程线程(非加锁线程)解锁 mutex 会返回一个错误。

# CritScope
利用构造函数加锁 CriticalSection；并利用析构函数在退出代码块的时候解锁 CriticalSection 。该手法对于所有C++开发人员并不陌生。

# TryCritScope
类似于 CritScope，利用构造函数尝试加锁 CriticalSection；并利用析构函数在退出代码块的时候解锁CriticalSection。

对于大多数熟练的Win32开发人员来说前两个文件的内容是很好理解的，不过是2道开胃菜而已。接着我们要真正开始进入分析多路信号分离器的范围了。这也是多线程篇的重点部分。我会尽可能详细的分析整个多路信号分离器的工作原理。不过也会抛弃一些实现细节，比如所有的代码分析都是基于“永久”等待信号事件的流程，不会去分析如何计算下一次等待时间。这样可以让我们更加专注于多路信号分离器的原理，而不被一些细节困扰。

作者：落冬风
链接：https://www.jianshu.com/p/8adba3bfbfc5
来源：简书
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。