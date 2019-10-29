#include "main_wnd.h"

#include <QApplication>

#include <string>
#include <vector>

#include "conductor.h"
#include "flag_defs.h"
#include "main_wnd.h"
#include "peer_connection_client.h"
#include "rtc_base/checks.h"
#include "rtc_base/constructor_magic.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/win32_socket_init.h"
#include "rtc_base/win32_socket_server.h"
#include "system_wrappers/include/field_trial.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    rtc::WinsockInitializer winsock_init;
    rtc::Win32SocketServer w32_ss;
    rtc::Win32Thread w32_thread(&w32_ss);
    rtc::ThreadManager::Instance()->SetCurrentThread(&w32_thread);

    QApplication a(argc, argv);

    MainWnd wnd(FLAG_server, FLAG_port, FLAG_autoconnect, FLAG_autocall);
    if (!wnd.Create()) {
        RTC_NOTREACHED();
        return -1;
    }

    rtc::InitializeSSL();
    PeerConnectionClient client;
    rtc::scoped_refptr<Conductor> conductor(
                new rtc::RefCountedObject<Conductor>(&client, &wnd));

    int ret = a.exec();
    rtc::CleanupSSL();
    return ret;
}
