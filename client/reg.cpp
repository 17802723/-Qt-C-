#include "reg.h"
#include "ui_reg.h"

#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QMovie>
#include "chat_server.h"
reg::reg(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::reg)
{
    ui->setupUi(this);
    ui->pushButton->setText("返回");
    setUI();
}

void reg::setUI()
{
    //设置窗体透明
    this->setAttribute(Qt::WA_TranslucentBackground);
    //设置无边框
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setMinimumSize(400, 300);
    ui->label_back->lower();
    ui->label_back->setScaledContents(true);
    ui->label_3->lower();
    //加载登录界面上方的动态背景图
    QMovie* logginB = new QMovie(":/picture./logginBack.gif");
    ui->label_back->setMovie(logginB);
    logginB->start();
    const QPixmap chatPix(":/picture./chat.svg");
    ui->label->setPixmap(chatPix);
    ui->label->setScaledContents(true);
    ui->label->setStyleSheet("background-color: transparent;");
    ui->label->setAttribute(Qt::WA_TranslucentBackground);  // 可选，增强兼容性
    ui->label->setAutoFillBackground(false);               // 禁用自动填充


    // 创建按钮
    minimizeButton = new QPushButton("－", this);
    closeButton = new QPushButton("×", this);

    // 设置按钮样式
    QString buttonStyle = R"(
            QPushButton {
                background-color: %1;
                border-radius: 15px;
                color: white;
                font-family: "Arial";
                font-size: 18px;
                min-width: 30px;
                max-width: 30px;
                min-height: 30px;
                max-height: 30px;
                border: none;
            }
            QPushButton:hover {
                background-color: %2;
            }
        )";

    minimizeButton->setStyleSheet(buttonStyle.arg("#FFD700").arg("#EEB422"));
    closeButton->setStyleSheet(buttonStyle.arg("#FF6B6B").arg("#EE3B3B"));

    // 创建按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // 添加水平弹簧将按钮推到右侧
    buttonLayout->addStretch();

    // 添加按钮（顺序：先最小化后关闭）
    buttonLayout->addWidget(minimizeButton);
    buttonLayout->addWidget(closeButton);

    // 设置按钮容器边距（上，右间距）
    buttonLayout->setContentsMargins(0, 10, 10, 0); // 顶部10px，右侧10px
    buttonLayout->setSpacing(8); // 按钮间距8px

    // 创建主布局并应用
    mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(buttonLayout); // 默认添加到顶部
    mainLayout->setContentsMargins(0, 0, 0, 0); // 主布局无边距

    // 设置按钮布局在垂直布局中的对齐方式
    mainLayout->setAlignment(buttonLayout, Qt::AlignRight | Qt::AlignTop);

    //美化登录按钮
    ui->pushButton_login->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50;  /* 背景色 */
            color: white;              /* 文字颜色 */
            border-radius: 15px;       /* 圆角半径 */
            padding: 12px 24px;        /* 内边距 */
            font-size: 12px;           /* 字体大小 */
            font-weight: bold;         /* 字体粗细 */
            border: 2px solid #45a049; /* 边框样式 */
            min-width: 50px;          /* 最小宽度 */
        }

        QPushButton:hover {
            background-color: #45a049; /* 悬停颜色 */
            border: 2px solid #3d8b40;
        }

        QPushButton:pressed {
            background-color: #3d8b40; /* 按下颜色 */
            padding-top: 13px;         /* 模拟按下效果 */
            padding-bottom: 11px;
        }

        QPushButton:disabled {
            background-color: #cccccc; /* 禁用状态 */
            border-color: #999999;
        }
    )");
    // 添加阴影效果（需要设置窗口背景）
    ui->pushButton_login->setGraphicsEffect(new QGraphicsDropShadowEffect(this));
    ui->pushButton_login->graphicsEffect()->setProperty("blurRadius", 8);
    ui->pushButton_login->graphicsEffect()->setProperty("offset", QPointF(2, 2));
    ui->pushButton_login->graphicsEffect()->setProperty("color", QColor(0, 0, 0, 60));
    ui->pushButton_login->setText("注册");


    //美化账号密码输入框
    //美化账号密码输入框
    // 统一设置样式
    QString editStyle = R"(
        QLineEdit {
            background-color: #f5f5f5;          /* 背景色 */
            border: 2px solid #e0e0e0;          /* 默认边框 */
            border-radius: 12px;                /* 圆角半径 */
            font-size: 14px;
            color: #333;
            selection-background-color: #4CAF50; /* 选中文本背景色 */
            border-radius: 15px;
        }

        QLineEdit:focus {                      /* 获取焦点时的样式 */
            border: 2px solid #4CAF50;
            background-color: white;
        }

        QLineEdit:hover {                      /* 鼠标悬停 */
            border: 2px solid #bdbdbd;
        }

        QLineEdit::placeholder {               /* 提示文字样式 */
            color: #9e9e9e;
            font-style: italic;
        }

        /* 密码框专用样式 */
        QLineEdit[echoMode="2"] {              /* echoMode=2 表示密码模式 */
            lineedit-password-character: 9679; /* Unicode 圆点字符 ● */
        }


    )";
    // 统一设置样式
    ui->lineEdit_username->setStyleSheet(editStyle);
    ui->lineEdit_passwd->setStyleSheet(editStyle);
    ui->lineEdit_username->setPlaceholderText("账号");
    ui->lineEdit_passwd->setPlaceholderText("密码");
    QAction* piture_user = new QAction(QIcon(":/picture./user.png"),"",ui->lineEdit_username);
    ui->lineEdit_username->addAction(piture_user,QLineEdit::LeadingPosition);
    QAction* piture_pwd = new QAction(QIcon(":/picture./lock.png"),"",ui->lineEdit_passwd);
    ui->lineEdit_passwd->addAction(piture_pwd , QLineEdit::LeadingPosition);



    ui->pushButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"  // 背景色
        "   color: white;"               // 文字颜色
        "   border-radius: 5px;"         // 圆角
        "   border: 2px solid #45a049;"  // 边框
        "}"
        "QPushButton:hover {"           // 鼠标悬停状态
        "   background-color: #45a049;"
        "}"
        "QPushButton:pressed {"         // 按下状态
        "   background-color: #3d8b40;"
        "}"
        );
}

void reg::closeAndMin(QWidget *shadowWin)
{
    //缩小和关闭
    connect(minimizeButton, &QPushButton::clicked ,shadowWin, &QWidget::showMinimized);
    connect(closeButton, &QPushButton::clicked, [=](){
        shadowWin->close();
        server.closConnect();
    });
}

void reg::changeWin(QStackedWidget* stackWidget, int index)
{
    connect(ui->pushButton,&QPushButton::clicked,stackWidget,[=](){
        stackWidget->setCurrentIndex(index);
    });

}

reg::~reg()
{
    delete ui;
}

void reg::on_pushButton_login_clicked()
{
    QString user = this->ui->lineEdit_username->text() + this->ui->lineEdit_passwd->text() + '\n';
    qDebug() << user;
    std::string username = this->ui->lineEdit_username->text().toUtf8().constData();
    std::string pwd = this->ui->lineEdit_passwd->text().toUtf8().constData();
    int regis = server.RegisterUser(username,pwd);
    if(regis == 0)
        QMessageBox::about(this , "创建成功" , "用户:" + this->ui->lineEdit_username->text() + "创建成功");
    else if(regis == 1 )
        QMessageBox::about(this , "用户存在" , "用户:" + this->ui->lineEdit_username->text() + " 存在，请更换用户名");
    else if(regis == 2)
        QMessageBox::about(this , "失败" , "网络连接失败或服务器不在线" );
}

