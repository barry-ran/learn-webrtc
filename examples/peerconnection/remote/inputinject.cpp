#include "inputinject.h"

#include <Windows.h>
#include <QRect>
#include <QDebug>

InputInject::InputInject()
{

}

void InputInject::injectMouseEvent(QEvent::Type action, Qt::MouseButton button, QPointF pos)
{
    /*
    QRect screenRect(GetSystemMetrics(SM_XVIRTUALSCREEN),
                     GetSystemMetrics(SM_YVIRTUALSCREEN),
                     GetSystemMetrics(SM_CXVIRTUALSCREEN),
                     GetSystemMetrics(SM_CYVIRTUALSCREEN));

    int x = screenRect.width() * pos.x();
    int y = screenRect.height() * pos.y();
    */
    //action = QEvent::MouseMove;
    //button = Qt::LeftButton;
    //pos = QPointF(0.5f, 0.5f);

    // Translate the coordinates of the cursor into the coordinates of the virtual screen.
    QPoint realPos(65535 * pos.x(), 65535 * pos.y());

    DWORD flags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

    INPUT input;
    memset(&input, 0, sizeof(input));

    input.type         = INPUT_MOUSE;
    input.mi.dx        = realPos.x();
    input.mi.dy        = realPos.y();
    input.mi.dwFlags   = flags;
    input.mi.mouseData = 0;

    switch (action) {
    case QEvent::MouseButtonPress:
        if (Qt::MouseButton::LeftButton == button) {
            input.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
        } else {
            input.mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN;
        }
        break;
    case QEvent::MouseButtonRelease:
        if (Qt::MouseButton::LeftButton == button) {
            input.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
        } else {
            input.mi.dwFlags |= MOUSEEVENTF_RIGHTUP;
        }
        break;
    case QEvent::MouseButtonDblClick:
        input.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
        ::SendInput(1, &input, sizeof(input));
        input.mi.dwFlags = flags | MOUSEEVENTF_LEFTUP;
        ::SendInput(1, &input, sizeof(input));
        input.mi.dwFlags = flags | MOUSEEVENTF_LEFTUP;
        ::SendInput(1, &input, sizeof(input));
        input.mi.dwFlags = flags | MOUSEEVENTF_LEFTUP;
        break;
    case QEvent::MouseMove:
        break;
    default:
        return;
    }
    uint uRet = ::SendInput(1, &input, sizeof(input));
    if(uRet <= 0)
    {
        DWORD dwErr=::GetLastError();
        qDebug() << "**************************";
    }
}
