#ifndef FUZZY_DOCK_H
#define FUZZY_DOCK_H

#include <QPoint>
#include <QRect>

// Прилипание окна к краям рабочей области экрана
namespace fuzzyDock {

enum Edge { Left = 1, Right = 2, Top = 4, Bottom = 8 };

// позиция окна с притяжением к краям area, если до края не больше distance пикселей
QPoint snapped(const QRect &window, const QRect &area, int distance);

// края area, к которым окно прижато (сочетание Edge)
int edges(const QRect &window, const QRect &area);

// позиция, при которой окно остаётся прижатым к краям edges
QPoint docked(const QRect &window, const QRect &area, int edges);

}

#endif // FUZZY_DOCK_H
