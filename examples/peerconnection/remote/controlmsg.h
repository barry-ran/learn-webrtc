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

    explicit ControlMsg(ControlMsgType controlMsgType);

    void setInjectMouseMsgData(QEvent::Type action, Qt::MouseButton button, QPointF pos);

    QByteArray serializeData();
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
