
webrtc/base/messagehandler.h/messagehandler.cc 文件仅仅定义了 MessageHandler 类，和一个模板工具类 FunctorMessageHandler 类。

# MessageHandler
MessageHandler 类的主要功能是定义了消息处理器的基本数据结构，子类在继承了该类之后要重载 OnMessage 函数，并实现消息响应的逻辑。

# FunctorMessageHandler
FunctorMessageHandler 类的主要功能是将一个函数投递到目标线程执行。该类主要通过 functor 模板实现（熟悉 C++ 的读者应该不会对它陌生，不熟悉的可以上网查找），而且定义了一个针对返回值类型为 void 的函数的特化版本（模板的特化和偏特也应该是一个 C++ 程序员掌握的一个知识点，该语法有些难度）。用户不需要创建或者继承 FunctorMessageHandler 类，仅需调用 Thread::Invoke 函数就能使用它的功能。

作者：落冬风
链接：https://www.jianshu.com/p/e3b902f60c7c
来源：简书
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。