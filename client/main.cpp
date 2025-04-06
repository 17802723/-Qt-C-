#include "shadowwindow.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQuickWidgets/QQuickWidget>
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QLoggingCategory>
int main(int argc, char *argv[])
{

    // 启用 QML 调试输出
    QLoggingCategory::setFilterRules("qt.qml.connections=true");
    QApplication a(argc, argv);
    ShadowWindow w;
    w.show();


    return a.exec();
}
