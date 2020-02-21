
Webrtc 多线程模块主要涉及 criticalsection、event、messagequeue、thread、messagehandler、physicalsocketserver 等文件. 

# Event
文件路径 webrtc/rtc_base/event.h webrtc/rtc_base/event.c.

event.h/event.cc文件中在 namespace rtc 中, 只有class Event类。
该类主要实现了跨平台的Win32 Event功能。Event类的各个成员函数与Win32 Event所提供的API几乎一致。

在 Linux 系统中，WebRTC 使用了mutex和条件变量来实现Event的功能。首先，对Win 32 API和pthread API做一下类比：

- CreateEvent

    pthread_mutex_init、pthread_cond_init这两个函数用来创建pthread的mute和条件变量。

- CloseHandle

    pthread_mutex_destroy、pthread_cond_destroy这两个函数用来销毁pthread的mute和条件变量。

- SetEvent

    pthread_mutex_lock、pthread_mutex_unlock这两个函数加锁和解锁mutex，

    pthread_cond_broadcast函数用来解除所有等待在该条件变量上的线程的阻塞状态。

- ResetEvent

    pthrea_mutex_lock、pthread_mutex_unlock（已解释）

- WaitForSingleObject

    pthrea_mutex_lock、pthread_mutex_unlock（已解释）。pthrea_cond_wait函数用来使线程阻塞在条件变量上。

下面将大致解释一下 Event 类的实现原理：

Event的主要功能由条件变量实现，mutex只是辅助条件变量起到锁的作用。条件变量的 pthread_cond_wait 和pthread_cond_broadcast函数与Win32 Event的WaitForSingleObject和SetEvent基本类似。Event是否为signal状态由布尔类型的成员变量event_status_控制。是否为manual reset的Event由布尔类型的成员变量is_manual_reset_控制。与Win32 Event不同的状况主要体现在Event的manual reset控制上。

linux 系统下所有调用 Event::Wait 函数的线程会阻塞在pthread_cond_wait 函数上。当 Event::Set 函数被调用时，pthread_cond_broadcast 函数会解除所有等待在 pthread_cond_wait 函数上的线程的阻塞状态。这对于 manual reset 的 Win32 Event 来说没什么问题，问题出在 auto reset 的 Win32 Event 上。Auto reset 的 Win32 Event 每次只能解除一条等待在Event上的线程的阻塞状态，其他线程依然为阻塞状态。这就需要mutex来配合实现了。

在这里要重点解释一下 pthread_cond_wait 函数的第二个参数 pthread_mutex_t *mutex。当线程进入pthread_cond_wait 函数时会解锁 mutex，而在离开 pthread_cond_wait 时会重新加锁 mutex。可以理解为：

```
intpthread_cond_wait(pthread_cond_t *cond,pthread_mutex_t *mutex)
{
pthread_mutex_unlock(mutex);
…
…
…
pthread_mutex_lock(mutex);
return0;
}
```
这是 Win32 没有的行为，需要特别注意。

有了以上的机制后，模拟 auto rest的 Win32 Event 就没问题了。当第一条线程获得 mutex 锁并离开pthread_cond_wait 函数时，其他线程会依然被阻塞在 pthread_mutex_lock(mutex) 函数上，无法离开 pthread_cond_wait 函数。那条成功离开线程会马上检测当前的 Event 是否为 manual reset 的，如果不是就马上将 event_status_ 成员变量设置为 false，并解锁 mutex。这时其他线程才能有机会离开pthread_cond_wait 函数。不过当他们离开 pthread_cond_wait 后立即检测 event_status_ 成员变量，如果为 false 就重新调用 pthread_cond_wait 函数。这就完美实现了 Win32 Event 的 auto reset 的语义。

条件变量和 mutex 的配合是 Event 类的难点。如果读者还是不能完全理解，请仔细阅读以上 3 段的内容（也可以上网查找 pthread_cond_wait 函数），并结合 event.cc 的源代码反复揣摩，应该可以很快理解的（毕竟代码不多，而且也不是WebRTC中真正困难的部分）

作者：落冬风
链接：https://www.jianshu.com/p/7945e324bab4
来源：简书
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。