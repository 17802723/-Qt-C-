#include "shadowwindow.h"
#include <QApplication>
#include <QWidget>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QQmlApplicationEngine>
#include "chat_server.h"
#include "loggin.h"
#include "reg.h"
#include <QQmlContext>
ShadowWindow::ShadowWindow(QWidget *parent)
    : QWidget{parent}
{
    // 设置无边框和透明背景
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    //登录界面
    setLoginWin();

    // 设置阴影效果
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect;
    shadowEffect->setBlurRadius(15);          // 阴影模糊半径
    shadowEffect->setColor(QColor(0, 0, 0, 160)); // 阴影颜色和透明度
    shadowEffect->setOffset(0, 0);            // 阴影偏移量
    //给嵌套QWidget设置阴影
    stackedWidget->setGraphicsEffect(shadowEffect);

    int width = this->width()-10;
    int height = this->height()-10;
    logginWidget->setGeometry(5,5,width,height);
    //logginWidgett->setStyleSheet("QWidget{border-radius:4px;background:rgba(255,255,255,1);}");  //设置圆角




    // 主窗口布局，留出阴影空间
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15); // 边距需≥模糊半径
    mainLayout->addWidget(stackedWidget);
    mainLayout->setAlignment(Qt::AlignCenter);
    // 窗口大小
    resize(450, 350);

}

void ShadowWindow::setLoginWin()
{
    // 登录界面
    logginWidget = new loggin(this);
    logginWidget->setStyleSheet("background: white; border-radius: 8px;");
    logginWidget->closeAndMin(this);

    //注册界面
    regi = new reg(this);
    regi->closeAndMin(this);
    stackedWidget = new QStackedWidget(this);

    //登录成功后进入主界面
    connect(logginWidget , &loggin::successLoggin , this , [&](){
        qmleng = new QQmlApplicationEngine;

        qmleng->rootContext()->setContextProperty("server", &server);
        qmleng->load(QUrl(QStringLiteral("qrc:/chat.qml")));
        server.startAcceptMessage();
        server.requestAccessFriendAdd();
        delete logginWidget;
        delete regi;
        this->close();
    });

    connect(&server , &chat_server::requestStartCapture , this ,[this](){
        server.startAudioChat();
    });
    connect(&server , &chat_server::requestStopCapture , this ,[this](){
        server.stopAudioChat();
    });
    connect(&server , &chat_server::stopAudio , this ,[this](){
        server.stopvideo();
    });
    connect(&server , &chat_server::videoChat_audio , this ,[this](){
        server.startVideoChat_audio();
    });



    int login_num = stackedWidget->addWidget(logginWidget);
    int reg_num = stackedWidget->addWidget(regi);

    stackedWidget->setCurrentIndex(reg_num);

    logginWidget->changeWin(stackedWidget,reg_num);
    regi->changeWin(stackedWidget,login_num);

    stackedWidget->setCurrentIndex(login_num);


}

void ShadowWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(logginWidget)
    {
        if (m_isDragging && event->buttons() & Qt::LeftButton) {
            // 计算新窗口位置 = 当前鼠标全局位置 - 差值
            QPoint newPos = event->globalPosition().toPoint() - m_dragPosition;
            move(newPos); // 移动窗口
            event->accept();
        }
        QWidget::mouseMoveEvent(event);
    }
}

void ShadowWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 计算差值 = 鼠标全局位置 - 窗口左上角坐标
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_isDragging = true;
        event->accept(); // 标记事件已处理
    }
    QWidget::mousePressEvent(event); // 传递事件给子控件（可选）
}
