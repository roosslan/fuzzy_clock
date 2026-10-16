#include <QtGlobal>

#include "fuzzy_dock.h"

namespace fuzzyDock {

QPoint snapped(const QRect &window, const QRect &area, int distance)
{
    QPoint p = window.topLeft();

    if (qAbs(window.left() - area.left()) <= distance)
        p.setX(area.left());
    else if (qAbs(window.right() - area.right()) <= distance)
        p.setX(area.right() - window.width() + 1);

    if (qAbs(window.top() - area.top()) <= distance)
        p.setY(area.top());
    else if (qAbs(window.bottom() - area.bottom()) <= distance)
        p.setY(area.bottom() - window.height() + 1);

    return p;
}

int edges(const QRect &window, const QRect &area)
{
    int result = 0;
    if (window.left() == area.left())
        result |= Left;
    else if (window.right() == area.right())
        result |= Right;
    if (window.top() == area.top())
        result |= Top;
    else if (window.bottom() == area.bottom())
        result |= Bottom;
    return result;
}

QPoint docked(const QRect &window, const QRect &area, int edges)
{
    QPoint p = window.topLeft();
    if (edges & Left)
        p.setX(area.left());
    else if (edges & Right)
        p.setX(area.right() - window.width() + 1);
    if (edges & Top)
        p.setY(area.top());
    else if (edges & Bottom)
        p.setY(area.bottom() - window.height() + 1);
    return p;
}

}
