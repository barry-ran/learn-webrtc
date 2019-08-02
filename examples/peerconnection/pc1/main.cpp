#include "widget.h"
#include <QApplication>

#include "rtc_base/win32socketinit.h"
#include "rtc_base/win32socketserver.h"
#include "simplepeerconnection.h"

int main(int argc, char *argv[])
{
    rtc::WinsockInitializer winsock_init;
    rtc::Win32SocketServer w32_ss;
    rtc::Win32Thread w32_thread(&w32_ss);
    rtc::ThreadManager::Instance()->SetCurrentThread(&w32_thread);

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
