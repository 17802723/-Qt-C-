#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <QObject>
#include <QString>
#include <asio.hpp>
#include <asio/ip/tcp.hpp>
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QGuiApplication>
#include <QImage>
#include <QBuffer>
#include <QtCore/QtGlobal>  // 必须包含
#include <QClipboard>      // 剪贴板操作头文件
#include <QFile>
#include <QGuiApplication> // GUI应用程序基础头文件
#include <QMimeData>
#include "audio.h"




enum MessageType {
    Register_REQUEST = 0,//注册请求
    LOGIN_REQUEST = 1,//登录请求
    LOGIN_RESPONSE = 2,
    TEXT_MESSAGE = 3,//文本消息
    FILE_MESSAGE = 4,//文件消息
    SerarchUser_MESSAGE = 5, //查询用户请求
    AddFriendsRequest = 6 ,//好友申请包
    SerarchUserResponse = 7 , //查询用户请求回应
    AddFriendsResponse =8, //好友申请回应
    ReturnAddReiend =9, //将数据库中的好友申请表发送到客户端
    FriendAddRequest = 10, //客户端申请获取好友列表
    ResponeFriendRequest = 11, //回应好友申请
    FriendListsToClient = 12, //服务器发送给客户端好友列表
    TextMessageHeader = 13, //客户端发来文本消息
    TextMessageHistoryToClient = 14, //服务器向客户端发送文本信息聊天记录
    PictureMessageHeader = 15, // 客户端发送带图片的消息
    FileMessageHeader = 16, //客户端发送文件消息
    FileSocket = 17, //声明该socket是专门用于传输文件的
    fileMessageHeaderToClient = 18, //给客户端发送文件消息
    ReceiveFileMessageHeader = 19 ,  //从服务器下载文件
    voiceChatRequestHeader = 20 , //发起通讯请求
    acceptChatRequestHeader = 21 , //接收到通讯请求
    responeVoiceChatHeader = 22, //回应电话通讯
    voiceChatRespone = 23 , //服务器给客户端回应语音消息
    transAudioToServer = 24 //客户端传输音频出去

};

//客户端发给服务端的包头
#pragma pack(push, 1)  // 1字节对齐
struct PacketHeader {
    uint16_t message_type;  // 消息类型（Login, Text, File, etc.）
    size_t length; //包长度
};
#pragma pack(pop)

//服务端发给客户端的包头
#pragma pack(push, 1)  // 1字节对齐
struct PacketHeaderServer
{
    uint16_t message_type;  // 消息类型
    size_t length; //包长度
};
#pragma pack(pop)


#pragma pack(push, 1)
struct RegisterRequest {
    char username[256] = {};     // 用户名
    char password[256] = {};     // 密码
};
#pragma pack(pop)

//注册请求回应包
#pragma pack(push, 1)
struct RegisterResponse {
    uint16_t status_code; // 状态码（0 = 成功，1 = 用户名已存在等）
};
#pragma pack(pop)

//登录请求回应包
#pragma pack(push, 1)
struct LoginResponse {
    uint16_t status_code; // 状态码（0 = 成功，1 = 用户名已存在 , 2 = 密码错误）
};
#pragma pack(pop)

//查询用户
#pragma pack(push, 1)
struct serarchUserRequest{
    char username[256];     // 用户名
};
#pragma pack(pop)
//查询用户是否存在的回应
#pragma pack(push, 1)
struct serarchUserResponse {
    bool status_code; // 状态码（true = 用户存在，false = 用户名不存在 ）
};
#pragma pack(pop)
//好友申请包
#pragma pack(push, 1)
struct addFrirendReauest
{
    char SenderName[256] = {};     // 发送者
    char ReceiverName[256] = {};     //接受者
};
#pragma pack(pop)

//好友申请回应包（不是好友申请通没通过，只是回应有没有成功把申请发出去）
#pragma pack(push, 1)
struct addFrirendResponse
{
    bool state;
};
#pragma pack(pop)

//数据库中的好友申请表，服务器发给客户端的
#pragma pack(push, 1)
struct SendFriendRequestToClient
{
    uint16_t status_code; //0为正在等待被接受 ， 1为接受，2为拒绝
    char SenderName[256] = {};     // 发送者
    char ReceiverName[256] = {};     //接受者
};
#pragma pack(pop)

//客户端发送的好友列表
#pragma pack(push, 1)
struct FriendListTTT
{
    std::vector<char> dataBuffer;
};
#pragma pack(pop)
//文本消息包
#pragma pack(push, 1) // 按1字节对齐
struct TextMessagePacket
{
    char SenderName[256] = {};     // 发送者
    char ReceiverName[256] = {};     //接受者
    char TextMessage[2046] = {};   //文本消息
    char messageTime[100] = {};    //消息发送时间
};
#pragma pack(pop)

//消息记录
#pragma pack(push, 1) // 按1字节对齐
struct MessageHistory
{
    uint16_t messageType;//1为文本消息 ， 2为文件消息
    char SenderName[256] = {};     // 发送者
    char ReceiverName[256] = {};     //接受者
    QString Message = {};   //文本消息或图片是数据流，文件消息是文件路径
    char messageTime[100] = {};    //消息发送时间
};
#pragma pack(pop)

//发送带有图片消息的包，分为两个，第一个是含有发送者和接受者信息的包，第二个是具体消息的包
#pragma pack(push , 1)
struct pictureMessagePacket_first
{
    char SenderName[256] = {};     // 发送者
    char ReceiverName[256] = {};     //接受者
    char messageTime[100] = {};    //消息发送时间
    uint16_t packetSize; //一共发送多少个包
};
#pragma pack(pop)

#pragma pack(push, 1)
struct pictureMessagePacket
{
    char TextMessage[2046] = {};   //文本消息
    bool isLastPacket; //是否是最后一个包
};
#pragma pack(pop)



class chat_server : public QObject
{
    Q_OBJECT
private:

    //创建socket，用于连接服务器
    asio::ip::tcp::socket* server_socket;
    //创建socket，用于传输文件
    //asio::ip::tcp::socket* file_socket;
    std::unordered_map<std::string , asio::ip::tcp::socket*> fileSocekt;
    bool isTransFile = false;
    bool isConnect = false;
    void connectToServer();
    RegisterResponse* respon;
    bool m_searchResult = false;
    bool searchResult() const { return m_searchResult; }
    std::string username;
    std::string password;
    PacketHeaderServer* serverHeader= new PacketHeaderServer;
    std::thread io_thread_;                        // 专用网络线程
    SendFriendRequestToClient* FrirendRequest;
    FriendListTTT* FriendsListData ;
    std::vector<std::string> friendsList;
    std::vector<char>* rawData;
    std::vector<MessageHistory> message;
    TextMessagePacket* textmessage_packet = new TextMessagePacket;
    std::atomic<bool> io_thread_running{false};
    AudioHandler* audioChat;
    QFile* AudioFile;
    asio::ip::tcp::socket* SendAudioSocekt;//发送音频的socket
    asio::ip::tcp::socket* ReciverAudioSocekt;//接受音频的socket
    QString audioChatName ;

public:
    //创建 io_context，负责执行异步操作
    asio::io_context* server_io_context;
    static QString server_ip;
    static int server_port;
    void closConnect();
    chat_server();
    int RegisterUser(std::string usernameRegister ,std::string passwordRegister);
    int LoginUser(std::string usernameRegister ,std::string passwordRegister);
    void acceptMessage();
    void startAcceptMessage();
    void requestAccessFriendAdd();
    void acceptPictureMessage(std::string* pictureMessage , std::string receiverName, std::string senderName , std::string messageTime);
    void sendFileMessage(long long* remainingSize , QFile* inputFile , QString fileName);
    void sendFile(const QString& receiverName , const QString& senderName , QString filePath);
    void receiverFile(long long* remainingSize , QFile* inputFile , QString fileName , asio::ip::tcp::socket* receiverFileSocekt);
    void startAudioChat();
    void stopAudioChat();
    void startAcceptAudio();
    void stopvideo();

    Q_INVOKABLE  bool searchUser(const QString& userName);
    Q_INVOKABLE QString getUserName(){return QString::fromStdString(username);}
    Q_INVOKABLE void sendMessqge(const QString& MessqgejsonStr);
    Q_INVOKABLE void sendAddFriend(const QString& ReceiverName);
    Q_INVOKABLE void respondFriendRequest(const QString& SenderName , int statue);
    Q_INVOKABLE void sendTextMessage(const QString& reveiverName , const QString& TextMessage);
    Q_INVOKABLE void sendLargeMessage_pictureOrText(const QString& reveiverName , const QString& Message);
    Q_INVOKABLE bool hasImage() const;    // 检查剪贴板是否包含图片
    Q_INVOKABLE QString getImageBase64() const;//获取剪贴板图片的Base64编码（PNG格式）
    Q_INVOKABLE bool hasText() const; //检查剪贴板是否包含文本
    Q_INVOKABLE QString getText() const;//获取剪贴板纯文本内容
    Q_INVOKABLE void openImageFileDialog();//发送图片
    Q_INVOKABLE void openFileDialog(const QString& receiverName , const QString& senderName);//发送文件
    Q_INVOKABLE void openFilePath(QString FileName , QString sender , QString receiver);//双击文件窗口后打开对应文件夹
    Q_INVOKABLE void downLoadFile(const QString& receiverName , const QString& senderName , QString FileName , QString SavePath);
    Q_INVOKABLE void selectDownLoadFilePath(const QString& receiverName , const QString& senderName , QString FileName);
    Q_INVOKABLE QString QmlHtmlPro(QString html);
    Q_INVOKABLE void sendVoiceChat(QString senderName , QString receiverName);
    Q_INVOKABLE void responeVoicdChat(QString senderName , QString receiverName , uint16_t status_code);//回应电话通讯
    Q_INVOKABLE void sendFile_tuoru(QString filePath , const QString& receiverName , const QString& senderName);


signals:
    void serachFriendRes(bool successStatus);//发去qml的信号，通知是否成功搜索到好友
    void addFriendRes(bool successStatus);//发去qml的信号，用于通知是否成功发送请求
    void sendFriendAddList(const QString& sender , const QString& receiver , const int& status);
    void newFriend(const QString& Friend);
    void newTextMessage(const QString& sender , const QString& receiver , const QString& text , const QString& time);
    void newPictureMessage(QString base64);
    void newFileMessage_send_TransProgress(QString FileName , QString sender , QString receiver , float TransferProgress , int fileIndex);
    void newFileMessage_send(QString FileName , QString sender , QString receiver , long long fileSize);
    void newFileMessage(const QString& sender , const QString& receiver , const QString& fileName , const QString& time , long fileSize);
    void downloadFileTransProgress(QString FileName , QString sender , QString receiver , float TransferProgress , int fileIndex);
    void acceptNewVoiceChat(const QString& sender , const QString& receiver);
    void isAcceptVoiceChat(bool is);
    void requestStartCapture();
    void requestStopCapture();
    void stopAudio();




};

extern chat_server server;


#endif // CHAT_SERVER_H
