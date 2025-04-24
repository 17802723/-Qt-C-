#ifndef VIDEOCHATWIDGET_H
#define VIDEOCHATWIDGET_H

#include <QWidget>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <QThread>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
namespace Ui {
class videoChatWidget;
}

class videoChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit videoChatWidget(QWidget *parent = nullptr);
    ~videoChatWidget();
    // 处理视频帧
    void processVideoFrame(const QVideoFrame &frame);
    void sendImageData(const QImage &image);//发送视频的每一帧出去
    void acceptImageData(const QByteArray &datagram);//接受视频帧
    void start();//开始
    void stop();//
    void setName(QString userName, QString chatUserName);
    void setUI();//美化界面
    void setStatus(int status);//设置视频通话状态.0:发出，等待接听；1:接收，等待确认；2:视频中
    void setCamera();
    void switchCamera(int index);
    int chatStatus;//当前通话状态0：正在呼叫，1:正在等待确认，2:视频通话中
    QString chatUser();//返回通话用户


private:
    Ui::videoChatWidget *ui;
    QCamera *m_camera;                   // 摄像头对象
    QMediaCaptureSession m_captureSession; // 媒体捕获会话
    QVideoSink *m_videoSink;             // 视频接收器
    void setupCamera();                  // 设置摄像头
    // 可用摄像头列表
    QList<QCameraDevice> cameras;
    // 创建控制按钮布局
    QHBoxLayout* controlLayout;
    // 创建接受通话按钮
    QPushButton* acceptButton;
    // 创建拒绝通话按钮
    QPushButton* rejectButton;
    // 添加通话状态标签
    QLabel* statusLabel;
    // 添加通话时长计时器
    QLabel* timerLabel;
    // 创建计时器
    QTimer* callTimer;
    QComboBox *cameraComboBox;


signals:
    void newImageData(const QByteArray &image);
    // 通话控制信号
    void acceptCall(int index = 0);
    void rejectCall(int index = 1);
    void endCall(int index = 2);
};

#endif // VIDEOCHATWIDGET_H
