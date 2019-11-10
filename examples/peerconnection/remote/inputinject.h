#ifndef INPUTINJECT_H
#define INPUTINJECT_H

#include <QEvent>
#include <QPointF>

class InputInject
{
public:
    InputInject();

    static void injectMouseEvent(QEvent::Type action, Qt::MouseButton button, QPointF pos);
};

#endif // INPUTINJECT_H
