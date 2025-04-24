#include "videochatwidget.h"
#include "ui_videochatwidget.h"
#include <QVBoxLayout>
#include <QMediaDevices>
#include <QCameraDevice>
#include <qbuffer.h>
#include <QImageWriter>
#include <QImageReader>
#include <QCamera>
#include <QDialog>
#include <QListWidget>
#include <QtConcurrent/QtConcurrent>
videoChatWidget::videoChatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::videoChatWidget)
{
    ui->setupUi(this);

    ui->label_UserVideo->setAlignment(Qt::AlignCenter);//将内容居中对齐
    ui->label_UserVideo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->label_chatUserVideo->setAlignment(Qt::AlignCenter);
    ui->label_chatUserVideo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    //创建视频接收器
    m_videoSink = new QVideoSink(this);
    // 连接视频帧信号
    connect(m_videoSink, &QVideoSink::videoFrameChanged,
            this, &videoChatWidget::processVideoFrame);
    // 设置摄像头
    setupCamera();

    // 获取可用摄像头列表
    cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        ui->label_UserVideo->setText("没有找到可用的摄像头");
        return;
    }

    // 使用第一个可用摄像头
    m_camera = new QCamera(cameras.first() , this);
    if(!m_camera->isActive())
    {
        ui->label_UserVideo->setText("摄像头被占用");
    }


    // 配置媒体捕获会话
    m_captureSession.setCamera(m_camera);
    m_captureSession.setVideoSink(m_videoSink);


    setUI();

}

videoChatWidget::~videoChatWidget()
{
    delete ui;
}

void videoChatWidget::processVideoFrame(const QVideoFrame &frame)
{
    //在子线程中执行耗费时间的操作
    QtConcurrent::run([this , frame](){
        // 将视频帧转换为图像
        QImage image = frame.toImage();
        if (image.isNull()) {
            return;
        }

        // 调整图像大小以适应标签
        QSize targetSize(640, 480); // 使用固定的目标大小
        QImage scaledImage = image.scaled(targetSize,
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
        // 在标签中显示图像
        //ui->label_UserVideo->setPixmap(QPixmap::fromImage(scaledImage));

        // 使用信号槽机制在主线程中更新UI
        QMetaObject::invokeMethod(this, [this, scaledImage]() {
            ui->label_UserVideo->setPixmap(QPixmap::fromImage(scaledImage));
        }, Qt::QueuedConnection);


        sendImageData(image);
    });

}

void videoChatWidget::sendImageData(const QImage &image)
{
    // 创建缓冲区存储压缩后的图像数据
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    // 将图像压缩为JPEG格式，质量为70（平衡质量和大小）
    QImageWriter writer(&buffer, "jpg");
    writer.setQuality(70);

    // 写入图像数据
    if (!writer.write(image)) {
        qWarning() << "无法写入图像数据:" << writer.errorString();
        return;
    }

    // 关闭缓冲区
    buffer.close();
    emit newImageData(byteArray);



}

void videoChatWidget::acceptImageData(const QByteArray &datagram)
{
    QtConcurrent::run([this,datagram](){
        // 创建QBuffer对象，关联到QByteArray
        QBuffer buffer(const_cast<QByteArray*>(&datagram));
        buffer.open(QIODevice::ReadOnly);

        // 创建QImageReader对象，从buffer读取图像
        QImageReader reader(&buffer);

        // 读取图像
        QImage image = reader.read();


        // 检查图像是否有效
        if (image.isNull()) {
            qDebug() << "接收到无效的图像数据";
        }

        // 调整图像大小以适应标签
        QSize targetSize(640, 480); // 使用固定的目标大小
        QImage scaledImage = image.scaled(targetSize,
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);

        //ui->label_chatUserVideo->setPixmap(QPixmap::fromImage(scaledImage));
        // 使用信号槽机制在主线程中更新UI
        QMetaObject::invokeMethod(this, [this, scaledImage]() {
            ui->label_chatUserVideo->setPixmap(QPixmap::fromImage(scaledImage));
        }, Qt::QueuedConnection);

    });



}

void videoChatWidget::start()
{
    // 启动摄像头
    m_camera->start();
}

void videoChatWidget::stop()
{
    m_camera->stop();
    callTimer->stop();
}

void videoChatWidget::setName(QString userName, QString chatUserName)
{
    ui->label_chatUserName->setText(chatUserName);
    ui->label_UserName->setText(userName);
}

void videoChatWidget::setUI()
{

    // 设置标签的固定大小
    // ui->label_UserVideo->setFixedSize(640, 480);
    // ui->label_chatUserVideo->setFixedSize(640, 480);

    // 设置整体窗口样式
    this->setStyleSheet("QWidget { background-color: #2c3e50; color: white; }");

    // 设置用户名标签样式
    QString labelStyle = "QLabel { font-size: 14px; font-weight: bold; color: #ecf0f1; "
                         "background-color: rgba(52, 73, 94, 180); padding: 4px; "
                         "border-radius: 4px; }";
    ui->label_UserName->setStyleSheet(labelStyle);
    ui->label_chatUserName->setStyleSheet(labelStyle);
    ui->label_UserName->setAlignment(Qt::AlignCenter);
    ui->label_chatUserName->setAlignment(Qt::AlignCenter);

    // 设置视频标签样式
    QString videoLabelStyle = "QLabel { background-color: #34495e; border: 2px solid #3498db; "
                              "border-radius: 8px; }";
    ui->label_UserVideo->setStyleSheet(videoLabelStyle);
    ui->label_chatUserVideo->setStyleSheet(videoLabelStyle);

    // 创建控制按钮布局
    controlLayout = new QHBoxLayout();

    // 创建接受通话按钮
    acceptButton = new QPushButton("接受通话", this);
    acceptButton->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; "
                                "border-radius: 15px; padding: 8px 15px; font-weight: bold; }"
                                "QPushButton:hover { background-color: #27ae60; }"
                                "QPushButton:pressed { background-color: #1e8449; }");
    acceptButton->setFixedSize(100, 30);
    connect(acceptButton, &QPushButton::clicked, this, [this]() {
        emit acceptCall(); // 需要在头文件中添加此信号
    });

    // 创建拒绝通话按钮
    rejectButton = new QPushButton("拒绝通话", this);
    rejectButton->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; "
                                "border-radius: 15px; padding: 8px 15px; font-weight: bold; }"
                                "QPushButton:hover { background-color: #c0392b; }"
                                "QPushButton:pressed { background-color: #a93226; }");
    rejectButton->setFixedSize(100, 30);
    connect(rejectButton, &QPushButton::clicked, this, [this]() {
        // 拒绝通话逻辑
        emit rejectCall(); // 需要在头文件中添加此信号
    });

    // 创建摄像头选择按钮
    QPushButton* cameraSelectButton = new QPushButton("选择摄像头", this);
    cameraSelectButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; "
                                      "border-radius: 15px; padding: 8px 15px; font-weight: bold; }"
                                      "QPushButton:hover { background-color: #2980b9; }"
                                      "QPushButton:pressed { background-color: #1f6aa5; }");
    cameraSelectButton->setFixedSize(100, 30);
    connect(cameraSelectButton, &QPushButton::clicked, this, &videoChatWidget::setCamera);

    // 设置结束视频按钮样式
    ui->pushButton_endVideoChat->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; "
                                               "border-radius: 15px; padding: 8px 15px; font-weight: bold; }"
                                               "QPushButton:hover { background-color: #c0392b; }"
                                               "QPushButton:pressed { background-color: #a93226; }");
    connect(ui->pushButton_endVideoChat, &QPushButton::clicked, this, [this]() {
        emit endCall(); // 需要在头文件中添加此信号
    });

    // 添加按钮到控制布局
    controlLayout->addWidget(cameraSelectButton);
    controlLayout->addWidget(acceptButton);
    controlLayout->addWidget(rejectButton);
    controlLayout->addWidget(ui->pushButton_endVideoChat);
    controlLayout->setAlignment(Qt::AlignCenter);
    controlLayout->setSpacing(10);

    // 替换原有的结束视频按钮布局
    // 首先找到原来的按钮所在的布局
    if (ui->pushButton_endVideoChat->parentWidget()) {
        // 获取垂直布局
        QVBoxLayout* verticalLayout = qobject_cast<QVBoxLayout*>(ui->verticalLayout_2);
        if (verticalLayout) {
            // 移除原来的按钮
            verticalLayout->removeWidget(ui->pushButton_endVideoChat);
            // 添加新的控制布局
            verticalLayout->addLayout(controlLayout);
        }
    }

    // 添加通话状态标签
    statusLabel = new QLabel("通话状态: 准备中", this);
    statusLabel->setStyleSheet("QLabel { color: #f1c40f; font-weight: bold; }");
    statusLabel->setAlignment(Qt::AlignCenter);

    // 将状态标签添加到布局中
    QGridLayout* mainLayout = qobject_cast<QGridLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(statusLabel, 1, 0, 1, 1);
    }

    // 添加通话时长计时器
    timerLabel = new QLabel("00:00:00", this);
    timerLabel->setStyleSheet("QLabel { color: #f1c40f; font-weight: bold; }");
    timerLabel->setAlignment(Qt::AlignCenter);

    // 创建计时器
    callTimer = new QTimer(this);
    QTime callDuration(0, 0, 0);
    connect(callTimer, &QTimer::timeout, this, [callDuration, this]() mutable {
        callDuration = callDuration.addSecs(1);
        timerLabel->setText(callDuration.toString("hh:mm:ss"));
    });

    // 将计时器标签添加到布局中
    if (mainLayout) {
        mainLayout->addWidget(timerLabel, 2, 0, 1, 1);
    }



    // 连接结束通话信号停止计时器
    connect(this, &videoChatWidget::endCall, callTimer, &QTimer::stop);
    connect(this, &videoChatWidget::rejectCall, callTimer, &QTimer::stop);

}

//设置视频通话状态.0:发出，等待接听；1:接收，等待确认；2:视频中
void videoChatWidget::setStatus(int status)
{
    chatStatus = status;
    if(status == 0)
    {
        statusLabel->setText("通话状态: 等待对方接听");
        acceptButton->hide();
        rejectButton->hide();
        statusLabel->show();
        timerLabel->hide();
        ui->pushButton_endVideoChat->show();
    }
    else if(status == 1)
    {
        statusLabel->setText("通话状态: 对方请求视频通话");
        acceptButton->show();
        rejectButton->show();
        statusLabel->show();
        timerLabel->hide();
        ui->pushButton_endVideoChat->hide();

    }
    else if(status == 2)
    {
        statusLabel->setText("通话状态: 通话中");
        acceptButton->hide();
        rejectButton->hide();
        statusLabel->show();
        timerLabel->show();
        // 通话开始时启动计时器
        callTimer->start(1000);
        ui->pushButton_endVideoChat->show();
    }
}

void videoChatWidget::setCamera()
{
    // 创建对话框
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("选择摄像头");
    dialog->setMinimumWidth(300);
    dialog->setStyleSheet("QDialog { background-color: #2c3e50; color: white; }"
                          "QLabel { color: white; }"
                          "QPushButton { background-color: #3498db; color: white; "
                          "border-radius: 4px; padding: 6px 12px; }"
                          "QPushButton:hover { background-color: #2980b9; }"
                          "QPushButton:pressed { background-color: #1f6aa5; }");

    // 创建布局
    QVBoxLayout* layout = new QVBoxLayout(dialog);

    // 创建标签
    QLabel* label = new QLabel("请选择要使用的摄像头设备：", dialog);
    layout->addWidget(label);

    // 创建列表控件
    QListWidget* listWidget = new QListWidget(dialog);
    listWidget->setStyleSheet("QListWidget { background-color: #34495e; color: white; border: 1px solid #3498db; }"
                              "QListWidget::item { padding: 6px; }"
                              "QListWidget::item:selected { background-color: #3498db; }");

    // 获取可用摄像头列表
    cameras = QMediaDevices::videoInputs();

    // 填充列表
    for (const QCameraDevice &camera : cameras) {
        listWidget->addItem(camera.description());
    }


    // 选中当前使用的摄像头
    if (m_camera) {
        for (int i = 0; i < cameras.size(); ++i) {
            if (cameras[i].id() == m_camera->cameraDevice().id()) {
                listWidget->setCurrentRow(i);
                break;
            }
        }
    }

    layout->addWidget(listWidget);

    // 创建按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // 创建确定按钮
    QPushButton* okButton = new QPushButton("确定", dialog);
    connect(okButton, &QPushButton::clicked, dialog, [this, dialog, listWidget]() {
        int selectedIndex = listWidget->currentRow();
        if (selectedIndex >= 0 && selectedIndex < cameras.size()) {
            switchCamera(selectedIndex);
        }
        dialog->accept();
    });

    // 创建取消按钮
    QPushButton* cancelButton = new QPushButton("取消", dialog);
    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    layout->addLayout(buttonLayout);

    // 显示对话框
    dialog->exec();

    // 对话框关闭后自动删除
    dialog->deleteLater();
}

void videoChatWidget::switchCamera(int index)
{
    // 检查索引是否有效
    if (index < 0 || index >= cameras.size()) {
        return;
    }



    // 释放旧的摄像头对象
    if (m_camera) {
        m_captureSession.setCamera(nullptr);
        delete m_camera;
    }

    // 创建新的摄像头对象
    m_camera = new QCamera(cameras[index], this);
    // 配置媒体捕获会话
    m_captureSession.setCamera(m_camera);
    m_captureSession.setVideoSink(m_videoSink);

    m_camera->start();


}

QString videoChatWidget::chatUser()
{
    return ui->label_chatUserName->text();
}

void videoChatWidget::setupCamera()
{



}
