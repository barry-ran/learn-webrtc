#include "main_wnd.h"

#include <QApplication>
#ifdef Q_OS_MAC
#include <QTimer>
#endif

#include <string>
#include <vector>

#include "conductor.h"
#include "flag_defs.h"
#include "main_wnd.h"
#include "peer_connection_client.h"
#include "rtc_base/checks.h"
#include "rtc_base/constructor_magic.h"
#include "rtc_base/ssl_adapter.h"
#ifdef Q_OS_WIN
#include "rtc_base/win32_socket_init.h"
#include "rtc_base/win32_socket_server.h"
#endif
#include "system_wrappers/include/field_trial.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#ifdef Q_OS_WIN
    rtc::WinsockInitializer winsock_init;
    // 内部创建message wnd，定时在主线程处理message queue消息
    rtc::Win32SocketServer w32_ss;
    rtc::Win32Thread w32_thread(&w32_ss);
    rtc::ThreadManager::Instance()->SetCurrentThread(&w32_thread);
#endif
    QApplication a(argc, argv);

    MainWnd wnd(FLAG_server, FLAG_port);
    if (!wnd.Create()) {
        RTC_NOTREACHED();
        return -1;
    }

    rtc::InitializeSSL();
    PeerConnectionClient client;
    rtc::scoped_refptr<Conductor> conductor(
                new rtc::RefCountedObject<Conductor>(&client, &wnd));

#ifdef Q_OS_MAC
    // for rtc::Thread::Current()->socketserver()->CreateAsyncSocket;
    // 定时在主线程处理message queue消息（这里主要为了socket消息）
    QTimer processSocket(&a);
    QObject::connect(&processSocket, &QTimer::timeout, [=]() {
        rtc::Thread* thread = rtc::Thread::Current();
        thread->ProcessMessages(0);
    });
    processSocket.start(100);
#endif

    int ret = a.exec();

#ifdef Q_OS_MAC
    processSocket.stop();
#endif

    rtc::CleanupSSL();
    return ret;
}
