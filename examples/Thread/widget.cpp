#include <QDebug>

#include "widget.h"
#include "ui_widget.h"

class MyRunnable : public rtc::Runnable {
  void Run(rtc::Thread* thread) override {
      for (int i=0; i<100; i++) {
          qDebug() << "MyRunnable::Run";
      }
  }
};

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::OnMessage(rtc::Message *msg)
{
    qDebug() << "Widget::OnMessage";
}


void Widget::on_normalThreadBtn_clicked()
{
    m_normalThread = std::move(rtc::Thread::Create());
    static std::unique_ptr<MyRunnable> runnable(new MyRunnable());
    qDebug() << "m_normalThread start";
    m_normalThread->Start(runnable.get());

    // stop will join
    m_normalThread->Stop();
    qDebug() << "m_normalThread stop";
}

void Widget::on_taskThreadBtn_clicked()
{
    m_taskThread = std::move(rtc::Thread::Create());
    m_taskThread->Start();
}

void Widget::on_postTaskBtn_clicked()
{
    if (m_taskThread) {
        m_taskThread->PostDelayed(RTC_FROM_HERE, 10, this, 0);
    }
}
