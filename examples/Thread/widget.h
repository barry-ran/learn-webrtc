#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include "rtc_base/thread.h"
#include "rtc_base/message_handler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget, public rtc::MessageHandler
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    virtual void OnMessage(rtc::Message* msg) override;

private Q_SLOTS:
    void on_normalThreadBtn_clicked();
    void on_taskThreadBtn_clicked();

    void on_postTaskBtn_clicked();

private:
    Ui::Widget *ui;

    std::unique_ptr<rtc::Thread> m_normalThread;
    std::unique_ptr<rtc::Thread> m_taskThread;
};
#endif // WIDGET_H
