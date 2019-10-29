#include <QDebug>
#include <QMessageBox>
#include <QKeyEvent>

#include "main_wnd.h"
#include "ui_mainwnd.h"

MainWnd::MainWnd(const char* server,
                 int port,                 
                 QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWnd)
    , destroyed_(false)
    , callback_(nullptr)
    , server_(server)
{
    ui->setupUi(this);
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%i", port);
    port_ = buffer;


    peersModel_ = new QStringListModel(this);
    ui->peersListView->setModel(peersModel_);
    ui->peersListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this, &MainWnd::EmitUIThreadCallback, this, &MainWnd::OnUIThreadCallback, Qt::QueuedConnection);    

    localVideoLabel_ = new QLabel(this);
    localVideoLabel_->setObjectName(QString::fromUtf8("localVideoLabel"));
    localVideoLabel_->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);\n"
"color: rgb(255, 255, 255);"));
    localVideoLabel_->setAlignment(Qt::AlignCenter);
    localVideoLabel_->setFixedSize(100, 100);

    localVideoLabel_->move(width() - localVideoLabel_->width(),
                           height() - localVideoLabel_->height());
    localVideoLabel_->hide();
}

MainWnd::~MainWnd()
{    
    delete ui;
}

bool MainWnd::Create()
{
    SwitchToConnectUI();
    show();
    return true;
}

void MainWnd::RegisterObserver(MainWndCallback *callback)
{
    callback_ = callback;
}

bool MainWnd::IsWindow()
{
    return true;
}

void MainWnd::SwitchToConnectUI()
{
    ui->connectWidget->show();
    ui->clientsWidget->hide();
    ui->videoWidget->hide();
    localVideoLabel_->hide();
    ui_ = CONNECT_TO_SERVER;
}

void MainWnd::SwitchToPeerList(const Peers &peers)
{
    ui->connectWidget->hide();
    ui->clientsWidget->show();
    ui->videoWidget->hide();
    localVideoLabel_->hide();

    QStringList data;
    Peers::const_iterator i = peers.begin();
    for (; i != peers.end(); ++i)
    {
        data << i->second.c_str();
    }
    peersModel_->setStringList(data);
    i = peers.begin();
    for (int count = 0; i != peers.end(); ++i)
    {
        peersModel_->setProperty(QString::number(count).toUtf8(), i->first);
        count++;
    }
    ui_ = LIST_PEERS;
}

void MainWnd::SwitchToStreamingUI()
{
    ui->connectWidget->hide();
    ui->clientsWidget->hide();
    ui->videoWidget->show();
    localVideoLabel_->show();
    localVideoLabel_->raise();
    ui_ = STREAMING;
}

void MainWnd::StartLocalRenderer(webrtc::VideoTrackInterface *local_video)
{
    local_renderer_.reset(new VideoRenderer(local_video));
    connect(local_renderer_.get(), &VideoRenderer::updateImage, this, &MainWnd::OnUpdateLocalImage, Qt::QueuedConnection);
}

void MainWnd::StopLocalRenderer()
{
    local_renderer_.reset();
}

void MainWnd::StartRemoteRenderer(webrtc::VideoTrackInterface *remote_video)
{
    remote_renderer_.reset(new VideoRenderer(remote_video));
    connect(remote_renderer_.get(), &VideoRenderer::updateImage, this, &MainWnd::OnUpdateRemoteImage, Qt::QueuedConnection);
}

void MainWnd::StopRemoteRenderer()
{
    remote_renderer_.reset();
}

void MainWnd::QueueUIThreadCallback(int msg_id, void *data)
{
    Q_EMIT EmitUIThreadCallback(msg_id, data);
}

void MainWnd::resizeEvent(QResizeEvent *event)
{
    localVideoLabel_->move(width() - localVideoLabel_->width(),
                           height() - localVideoLabel_->height());
}

void MainWnd::closeEvent(QCloseEvent *event)
{
    if (callback_) {
        if (ui_ == STREAMING) {
            callback_->DisconnectFromCurrentPeer();
        } else if (ui_ == LIST_PEERS){
            callback_->DisconnectFromServer();
        }
    }
}

void MainWnd::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key_Escape == event->key()) {
        if (callback_) {
            if (ui_ == STREAMING) {
                callback_->DisconnectFromCurrentPeer();
            } else if (ui_ == LIST_PEERS){
                callback_->DisconnectFromServer();
            }
        }
    }
}

void MainWnd::MessageBox(const char *caption, const char *text, bool is_error)
{
    QMessageBox::warning(this, caption, text,
                         is_error ? QMessageBox::Abort : QMessageBox::Ok);
}


void MainWnd::on_connectBtn_clicked()
{
    if (!callback_) {
        return;
    }
    std::string server(ui->ipEdt->text().toStdString());
    std::string port_str(ui->portEdt->text().toStdString());
    int port = port_str.length() ? atoi(port_str.c_str()) : 0;
    callback_->StartLogin(server, port);
}

void MainWnd::on_peersListView_doubleClicked(const QModelIndex &index)
{
    int peer_id = peersModel_->property(QString::number(index.row()).toUtf8()).toInt();
    callback_->ConnectToPeer(peer_id);
}

void MainWnd::OnUIThreadCallback(int msg_id, void *data)
{
    callback_->UIThreadCallback(msg_id, data);
}

void MainWnd::OnUpdateLocalImage(QImage image)
{
    localVideoLabel_->setPixmap(QPixmap::fromImage(image.scaled(localVideoLabel_->width(), localVideoLabel_->height())));
}

void MainWnd::OnUpdateRemoteImage(QImage image)
{
    ui->remoteVideoLabel->setPixmap(QPixmap::fromImage(image.scaled(ui->remoteVideoLabel->width(), ui->remoteVideoLabel->height())));
}
