#include <QBuffer>

#include "controlmsg.h"
#include "bufferutil.h"

ControlMsg::ControlMsg(ControlMsgType controlMsgType) : QObject(nullptr)
{
    m_data.type = controlMsgType;
}

void ControlMsg::setInjectMouseMsgData(QEvent::Type action, Qt::MouseButton button, QPointF pos)
{
    m_data.injectMouse.action = action;
    m_data.injectMouse.button = button;
    m_data.injectMouse.pos = pos;
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
