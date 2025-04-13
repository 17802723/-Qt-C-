#ifndef SHADOWWINDOW_H
#define SHADOWWINDOW_H

#include "loggin.h"
#include "reg.h"

#include <QObject>
#include <QWidget>
#include <QStackedWidget>
#include <QQmlApplicationEngine>
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
class ShadowWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ShadowWindow(QWidget *parent = nullptr);
    void setLoginWin();
    QStackedWidget* stackedWidget ;
private:
    loggin *logginWidget;
    bool m_isDragging = false; // 拖动状态标识
    QPoint m_dragPosition; // 记录鼠标按下时的全局位置与窗口位置的差值
    reg* regi;
    QQmlApplicationEngine* qmleng;

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
};
#endif // SHADOWWINDOW_H
