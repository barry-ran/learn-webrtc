#ifndef CONTROLMSG_H
#define CONTROLMSG_H

#include <QObject>
#include <QEvent>
#include <QPointF>

class ControlMsg : public QObject
{
    Q_OBJECT
public:
    enum ControlMsgType {
        CMT_NULL = -1,
        CMT_INJECT_MOUSE = 0,
    };

    explicit ControlMsg(ControlMsgType controlMsgType = ControlMsg::CMT_NULL);
    ControlMsg(const ControlMsg& msg);
    virtual ~ControlMsg();

    void setInjectMouseMsgData(QEvent::Type action, Qt::MouseButton button, QPointF pos);
    void getInjectMouseMsgData(QEvent::Type& action, Qt::MouseButton& button, QPointF& pos);
    QByteArray serializeData();
    static ControlMsg unserializeData(QByteArray& data);

    ControlMsg::ControlMsgType type();
Q_SIGNALS:

public Q_SLOTS:

private:
    struct ControlMsgData {
        ControlMsgType type = CMT_NULL;
        union {
            struct {
                QEvent::Type action;
                Qt::MouseButton button;
                QPointF pos;
            } injectMouse;
        };

        ControlMsgData(){}
        ~ControlMsgData(){}
    };

    ControlMsgData m_data;
};

#endif // CONTROLMSG_H
