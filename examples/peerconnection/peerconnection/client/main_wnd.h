#ifndef MAINWND_H
#define MAINWND_H

#include <QWidget>
#include <QStringListModel>

#include <map>
#include <memory>
#include <string>

#include "api/media_stream_interface.h"
#include "api/video/video_frame.h"
#include "peer_connection_client.h"
#include "media/base/media_channel.h"
#include "media/base/video_common.h"
#if defined(WEBRTC_WIN)
#include "rtc_base/win32.h"
#endif  // WEBRTC_WIN

#include "videorenderer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWnd; }
QT_END_NAMESPACE

class MainWndCallback {
 public:
  virtual void StartLogin(const std::string& server, int port) = 0;
  virtual void DisconnectFromServer() = 0;
  virtual void ConnectToPeer(int peer_id) = 0;
  virtual void DisconnectFromCurrentPeer() = 0;
  virtual void UIThreadCallback(int msg_id, void* data) = 0;
  virtual void Close() = 0;

 protected:
  virtual ~MainWndCallback() {}
};

class MainWindow {
 public:
  virtual ~MainWindow() {}

  enum UI {
    CONNECT_TO_SERVER,
    LIST_PEERS,
    STREAMING,
  };

  virtual void RegisterObserver(MainWndCallback* callback) = 0;

  virtual bool IsWindow() = 0;
  virtual void MessageBox(const char* caption,
                          const char* text,
                          bool is_error) = 0;

  virtual UI current_ui() = 0;

  virtual void SwitchToConnectUI() = 0;
  virtual void SwitchToPeerList(const Peers& peers) = 0;
  virtual void SwitchToStreamingUI() = 0;

  virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video) = 0;
  virtual void StopLocalRenderer() = 0;
  virtual void StartRemoteRenderer(
      webrtc::VideoTrackInterface* remote_video) = 0;
  virtual void StopRemoteRenderer() = 0;

  virtual void QueueUIThreadCallback(int msg_id, void* data) = 0;
};

class MainWnd : public QWidget, public MainWindow
{
    Q_OBJECT

public:
    MainWnd(const char* server, int port, QWidget *parent = nullptr);
    ~MainWnd();

    bool Create();

    virtual void RegisterObserver(MainWndCallback* callback);
    virtual bool IsWindow();
    virtual void SwitchToConnectUI();
    virtual void SwitchToPeerList(const Peers& peers);
    virtual void SwitchToStreamingUI();
    virtual void MessageBox(const char* caption, const char* text, bool is_error);
    virtual UI current_ui() { return ui_; }

    virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
    virtual void StopLocalRenderer();
    virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
    virtual void StopRemoteRenderer();

    virtual void QueueUIThreadCallback(int msg_id, void* data);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

private Q_SLOTS:
    void on_connectBtn_clicked();
    void on_peersListView_doubleClicked(const QModelIndex &index);
    void OnUIThreadCallback(int msg_id, void* data);
    void OnUpdateRemoteImage(QImage image);
    void OnUpdateLocalImage(QImage image);

Q_SIGNALS:
    void EmitUIThreadCallback(int msg_id, void* data);

private:
    Ui::MainWnd *ui;

    UI ui_;
    std::unique_ptr<VideoRenderer> local_renderer_;
    std::unique_ptr<VideoRenderer> remote_renderer_;
    //HWND wnd_;
    //DWORD ui_thread_id_;
    QStringListModel *peersModel_;
    bool destroyed_;
    MainWndCallback* callback_;
    std::string server_;
    std::string port_;        
    QLabel *localVideoLabel_ = nullptr;
};
#endif // MAINWND_H
