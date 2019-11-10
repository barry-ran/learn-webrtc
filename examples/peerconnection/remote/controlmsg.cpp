#include <QBuffer>

#include "controlmsg.h"
#include "bufferutil.h"

ControlMsg::ControlMsg(ControlMsgType controlMsgType) : QObject(nullptr)
{
    m_data.type = controlMsgType;
}

ControlMsg::ControlMsg(const ControlMsg &msg)
{
    m_data = msg.m_data;
}

ControlMsg::~ControlMsg()
{

}

void ControlMsg::setInjectMouseMsgData(QEvent::Type action, Qt::MouseButton button, QPointF pos)
{
    m_data.injectMouse.action = action;
    m_data.injectMouse.button = button;
    m_data.injectMouse.pos = pos;
}

void ControlMsg::getInjectMouseMsgData(QEvent::Type &action, Qt::MouseButton &button, QPointF &pos)
{
    action = m_data.injectMouse.action;
    button = m_data.injectMouse.button;
    pos = m_data.injectMouse.pos;
}

QByteArray ControlMsg::serializeData()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QBuffer::WriteOnly);
    buffer.putChar(m_data.type);

    switch (m_data.type) {
    case CMT_INJECT_MOUSE:
        BufferUtil::write32(buffer, m_data.injectMouse.action);
        BufferUtil::write32(buffer, (quint32)m_data.injectMouse.button);
        BufferUtil::writeFloat(buffer, m_data.injectMouse.pos.x());
        BufferUtil::writeFloat(buffer, m_data.injectMouse.pos.y());
        break;
    default:
        break;
    }
    buffer.close();
    return byteArray;
}

ControlMsg ControlMsg::unserializeData(QByteArray &data)
{
    ControlMsg msg;
    qint64 len = data.size();
    if (len < 1) {
        return msg;
    }
    QBuffer buf(&data);
    buf.open(QBuffer::ReadOnly);
    char c = 0;
    buf.peek(&c, 1);
    ControlMsgType type = (ControlMsgType)c;
    switch (type) {
    case CMT_INJECT_MOUSE:
        if (len < (1 + 16)) {
            break;
        }
        buf.getChar(&c);
        msg.m_data.type = type;
        msg.m_data.injectMouse.action = (QEvent::Type)BufferUtil::read32(buf);
        msg.m_data.injectMouse.button = (Qt::MouseButton)BufferUtil::read32(buf);
        msg.m_data.injectMouse.pos.setX(BufferUtil::readFloat(buf));
        msg.m_data.injectMouse.pos.setY(BufferUtil::readFloat(buf));
        buf.close();
        data.remove(0, 1 + 16);
        break;
    default:
        break;
    }
    return msg;
}

ControlMsg::ControlMsgType ControlMsg::type()
{
    return m_data.type;
}
