#include "widget.h"
#include <QApplication>

#ifdef Q_OS_WIN
#include "rtc_base/win32_socket_init.h"
#include "rtc_base/win32_socket_server.h"
#endif

#include "simplepeerconnection.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#ifdef Q_OS_WIN
    rtc::WinsockInitializer winsock_init;
    rtc::Win32SocketServer w32_ss;
    rtc::Win32Thread w32_thread(&w32_ss);
    rtc::ThreadManager::Instance()->SetCurrentThread(&w32_thread);
#else
    rtc::ThreadManager::Instance()->SetCurrentThread(rtc::ThreadManager::Instance()->CurrentThread());
#endif

    SimplePeerConnection::InitPeerConnectionFactory();
    int exit;

    {
        QApplication a(argc, argv);
        Widget w;
        w.show();
        exit = a.exec();
    }

    SimplePeerConnection::ClearPeerConnectionFactory();
    return exit;
}
