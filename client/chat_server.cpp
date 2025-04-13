#include "chat_server.h"
#include <QBuffer>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <memory>
#include <vector>
#include <QString>
#include <QFileDialog>
#include <QMimeDatabase>
#include <QFile>
#include <QTimer>
#include <QDesktopServices>
#include <windows.h>
#include <shellapi.h>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QTextDocument>
#include <QTextBlock>
//QString chat_server::server_ip = "192.168.202.132";
QString chat_server::server_ip = "47.122.48.34";

int chat_server::server_port = 1145;

chat_server server;

void chat_server::connectToServer()
{

    if(isConnect)
        return;
    try
    {
        // 创建 io_context，负责执行异步操作
        server_io_context = new asio::io_context;

        // 解析服务器地址和端口，得到服务器的端点（endpoint）
        asio::ip::tcp::resolver resolver(*server_io_context);
        auto endpoints = resolver.resolve(server_ip.toStdString(),std::to_string(server_port));

        // 创建 socket 连接到服务器
        server_socket = new asio::ip::tcp::socket(*server_io_context);
        asio::connect(*server_socket,endpoints);
        isConnect = true;

        //创建传输文件的socket
        // file_socket = new asio::ip::tcp::socket(*server_io_context);
        // asio::connect(*file_socket,endpoints);



    }
    catch(const std::exception& e)
    {
        qDebug() << "Exception: " << e.what();
    }
}


void chat_server::closConnect()
{
    if(isConnect)
        server_socket->close();
}

chat_server::chat_server()
{
    connectToServer();

}

int chat_server::RegisterUser(std::string usernameRegister, std::string passwordRegister)
{
    if(!isConnect)
        connectToServer();
    if(!isConnect)
        return 2;//服务器未在运行
    //发送包头
    PacketHeader header{Register_REQUEST};
    //将包体转换成二进制字节流
    std::vector<char> header_buffer(sizeof(PacketHeader));
    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
    asio::write(*server_socket , asio::buffer(header_buffer));
    //发送注册请求包
    RegisterRequest request;
    std::memset(request.username , '\0' , sizeof(request.username));
    std::memset(request.password , '\0' , sizeof(request.password));
    std::strncpy(request.username , usernameRegister.c_str() , sizeof(request.username) - 1);
    std::strncpy(request.password , passwordRegister.c_str() , sizeof(request.password) - 1);

    std::string user(request.username);
    std::string pwd(request.password);
    qDebug() << "用户名: " <<user << " 密码:"<<pwd;
    //将包体转换成二进制字节流
    std::vector<char> request_buffer(sizeof(RegisterRequest));
    memset(request_buffer.data(),'\0',sizeof(RegisterRequest));
    std::memcpy(request_buffer.data() , &request ,sizeof(RegisterRequest));
    asio::write(*server_socket , asio::buffer(request_buffer, sizeof(RegisterRequest)));

    respon = new RegisterResponse;
    respon->status_code = 0;
    asio::read(*server_socket , asio::buffer(respon , sizeof(RegisterResponse)));
    if(respon->status_code == 0)
    {
        return 0;
        qDebug() << "创建成功";
    }
    else
    {
        return 1;
        qDebug() << "用户存在";
    }


}

int chat_server::LoginUser(std::string usernameRegister, std::string passwordRegister)
{
    if(!isConnect)
        connectToServer();
    if(!isConnect)
        return 3;

    //发送包头
    PacketHeader header{LOGIN_REQUEST};
    std::vector<char> header_buffer(sizeof(PacketHeader));
    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
    asio::write(*server_socket , asio::buffer(header_buffer));

    //发送登录请求包（由于登录请求内容与注册一样，所以用同一个包）
    RegisterRequest request;
    std::memset(request.username , '\0' , sizeof(request.username));
    std::memset(request.password , '\0' , sizeof(request.password));
    std::strncpy(request.username , usernameRegister.c_str() , sizeof(request.username) - 1);
    std::strncpy(request.password , passwordRegister.c_str() , sizeof(request.password) - 1);

    std::string user(request.username);
    std::string pwd(request.password);
    qDebug() << "用户名: " <<user << " 密码:"<<pwd;
    std::vector<char> request_buffer(sizeof(RegisterRequest));
    memset(request_buffer.data(),'\0',sizeof(RegisterRequest));
    std::memcpy(request_buffer.data() , &request ,sizeof(RegisterRequest));
    asio::write(*server_socket , asio::buffer(request_buffer, sizeof(RegisterRequest)));

    LoginResponse loginRespon ;
    asio::read(*server_socket , asio::buffer(&loginRespon , sizeof(RegisterResponse)));
    if(loginRespon.status_code == 0)
    {
        username = usernameRegister;
        password = usernameRegister;
        qDebug() << "登录成功";
        return 0;

    }
    else if(loginRespon.status_code == 1)
    {
        qDebug() << "用户不存在";
        return 1;

    }
    else if(loginRespon.status_code == 2)
    {
        qDebug() << "密码错误";
        return 2;

    }


}

void chat_server::acceptMessage()
{
    if(!isConnect)
        return;


    qDebug() << "准备读取服务器发送的数据";
    // serverHeader = new PacketHeaderServer;
    //auto serverHeader = std::make_shared<PacketHeaderServer>(); // 改用局部 shared_ptr
    asio::async_read(*server_socket , asio::buffer(serverHeader , sizeof(PacketHeaderServer)),
    [&](asio::error_code ec, std::size_t length)
    {

         qDebug() << "收到包头类型: " << serverHeader->message_type;
        if(ec)
        {
            try
            {
                throw asio::system_error(ec);
            }
            catch(const std::exception& e)
            {
                qDebug() << "System error: " << e.what() << ", Error code: "<<ec.value() ;
            }
        }
        else if(serverHeader->message_type == SerarchUserResponse)
        {
            qDebug() << "接收到服务器发送的好友申请回应包";
            auto response = std::make_shared<serarchUserResponse>();
            qDebug() << "准备读取服务端信息" ;
            // 异步读取响应体
            asio::async_read(
                *server_socket,
                asio::buffer(response.get(), sizeof(serarchUserResponse)),
                [this,response](asio::error_code ec, size_t length) {
                    if (!ec) {
                        qDebug() << "好友申请响应状态码: " << response->status_code;
                        emit serachFriendRes(response->status_code);
                    }
                    acceptMessage();
                }
                );
        }
        else if(serverHeader->message_type == AddFriendsResponse)
        {
            qDebug() << "接收到服务器发送的好友申请回应包";
            auto response = std::make_shared<addFrirendResponse>();
            qDebug() << "准备读取服务端信息" ;
            asio::async_read(
                *server_socket,
                asio::buffer(response.get(), sizeof(addFrirendResponse)),
                [this,response](asio::error_code ec, size_t length) {
                    if (!ec) {
                        qDebug() << "好友申请响应状态码: " << response->state;
                        emit addFriendRes(response->state);
                    }
                    acceptMessage();
                }
                );
        }

        else if(serverHeader->message_type == ReturnAddReiend)
        {
            qDebug() << "接受到好友申请列表";
            FrirendRequest = new SendFriendRequestToClient ;
            FrirendRequest->status_code = -1;
            std::memset(FrirendRequest->ReceiverName , '\0' , sizeof(FrirendRequest->ReceiverName));
            std::memset(FrirendRequest->SenderName , '\0' , sizeof(FrirendRequest->SenderName));
            // asio::read(*server_socket,asio::buffer(FrirendRequest, sizeof(addFrirendReauest)));
            // qDebug() << "sender:" << FrirendRequest->SenderName << " receiver" << FrirendRequest->ReceiverName << "status" << FrirendRequest->status_code;
            asio::async_read(*server_socket,
                             asio::buffer(FrirendRequest, sizeof(SendFriendRequestToClient)),
                             [&](asio::error_code ec, size_t length){
                                 qDebug() << FrirendRequest->SenderName << "向：" << FrirendRequest->ReceiverName;
                                 qDebug() << "sender:" << FrirendRequest->SenderName << " receiver" << FrirendRequest->ReceiverName << "status" << FrirendRequest->status_code;
                                 emit sendFriendAddList(FrirendRequest->SenderName , FrirendRequest->ReceiverName , FrirendRequest->status_code);
                                 acceptMessage();
                             });
        }
        else if(serverHeader->message_type ==  FriendListsToClient)
        {
            //FriendsListData = new FriendListTTT{std::vector<char>(serverHeader.length)};
            rawData = new std::vector<char>(serverHeader->length);
            asio::async_read(*server_socket,
                             asio::buffer(*rawData , serverHeader->length),
                             [&](asio::error_code ec, size_t length){
                                 qDebug() << "接受好友列表" << serverHeader->length;
                                 const char* ptr = rawData->data();
                                 qDebug() << ptr;
                                 size_t  remaining = rawData->size();
                                 // 解析字符串列表
                                 while (remaining > 0) {
                                     size_t strLen = std::strlen(ptr);
                                     friendsList.emplace_back(ptr, strLen);
                                     ptr += strLen + 1; // 跳过 '\0'
                                     remaining -= strLen + 1;
                                 }
                                 for (const auto& name : friendsList) {
                                     emit newFriend(QString::fromStdString(name));
                                     qDebug() << name;
                                 }
                                 delete rawData;
                                 acceptMessage();
                             });
        }
        else if(serverHeader->message_type == TextMessageHistoryToClient)
        {
            qDebug() << "准备接受聊天记录" << "数据包长度" << serverHeader->length;
            std::memset(textmessage_packet->ReceiverName , '\0' , sizeof(textmessage_packet->ReceiverName));
            std::memset(textmessage_packet->SenderName , '\0' , sizeof(textmessage_packet->SenderName));
            std::memset(textmessage_packet->TextMessage , '\0' , sizeof(textmessage_packet->TextMessage));
            std::memset(textmessage_packet->messageTime , '\0' , sizeof(textmessage_packet->messageTime));
            asio::async_read(*server_socket,
                             asio::buffer(textmessage_packet, serverHeader->length),
                             [this](asio::error_code ec, size_t length){
                                 const size_t maxTextMessageSize = sizeof(*textmessage_packet);
                                 if (serverHeader->length > maxTextMessageSize) {
                                     qDebug() << "无效的聊天记录长度：" << serverHeader->length << "聊天记录的长度应该为:" << maxTextMessageSize;
                                     acceptMessage(); // 继续监听后续消息
                                     return;
                                 }
                                 if (ec) {
                                     qDebug() << "读取错误:" << ec.message().c_str();
                                     return;
                                 }
                                 qDebug() << "接收到聊天记录";
                                 MessageHistory tmp;
                                 tmp.messageType = 1;
                                 std::memset(tmp.ReceiverName , '\0' , sizeof(tmp.ReceiverName));
                                 std::memset(tmp.SenderName , '\0' , sizeof(tmp.SenderName));
                                 std::memset(tmp.messageTime , '\0' , sizeof(tmp.messageTime));
                                 strncpy(tmp.ReceiverName , textmessage_packet->ReceiverName , sizeof(tmp.ReceiverName) - 1);
                                 strncpy(tmp.SenderName , textmessage_packet->SenderName , sizeof(tmp.SenderName) - 1);
                                 tmp.Message = QString(textmessage_packet->TextMessage);
                                 strncpy(tmp.messageTime , textmessage_packet->messageTime , sizeof(tmp.messageTime) - 1);
                                 message.push_back(tmp);
                                 qDebug() << tmp.SenderName << "发送给" << tmp.ReceiverName <<"消息： " << tmp.Message << "发送时间为" << tmp.messageTime;
                                 int n = 7;
                                 int len = strlen(tmp.messageTime);

                                 if (len >= n) {
                                     tmp.messageTime[len - n] = '\0'; // 在新的末尾位置添加空字符
                                 }
                                 emit newTextMessage(tmp.SenderName,tmp.ReceiverName,tmp.Message,tmp.messageTime);
                                 acceptMessage();
                             });
        }
        else if(serverHeader->message_type == PictureMessageHeader)
        {
            qDebug() << "接收到带有图片的消息\n";
            auto* pictureMessagePacket_0 = new pictureMessagePacket_first;
            std::string* pictureMessage = new std::string ;
            std::memset(pictureMessagePacket_0->ReceiverName, 0, sizeof(pictureMessagePacket_0->ReceiverName));
            std::memset(pictureMessagePacket_0->SenderName, 0, sizeof(pictureMessagePacket_0->SenderName));
            std::memset(pictureMessagePacket_0->messageTime, 0, sizeof(pictureMessagePacket_0->messageTime));

            asio::async_read(*server_socket,
                             asio::buffer(pictureMessagePacket_0, sizeof(pictureMessagePacket_first)),
                             [this, pictureMessagePacket_0 , pictureMessage](asio::error_code ec, size_t length) { // 使用引用捕获 packets
                                 qDebug() << pictureMessagePacket_0->SenderName << "给" << pictureMessagePacket_0->ReceiverName << "接受带有图片的消息\n";
                                 qDebug() << "一共需要接受" << pictureMessagePacket_0->packetSize << "个包\n";
                                 qDebug() << pictureMessagePacket_0->messageTime;
                                 acceptPictureMessage(pictureMessage , pictureMessagePacket_0->ReceiverName , pictureMessagePacket_0->SenderName , pictureMessagePacket_0->messageTime);
                             });
        }
        else if(serverHeader->message_type == fileMessageHeaderToClient)
        {

            std::memset(textmessage_packet->ReceiverName , '\0' , sizeof(textmessage_packet->ReceiverName));
            std::memset(textmessage_packet->SenderName , '\0' , sizeof(textmessage_packet->SenderName));
            std::memset(textmessage_packet->TextMessage , '\0' , sizeof(textmessage_packet->TextMessage));
            std::memset(textmessage_packet->messageTime , '\0' , sizeof(textmessage_packet->messageTime));
            asio::async_read(*server_socket,
                             asio::buffer(textmessage_packet, serverHeader->length),
                             [this](asio::error_code ec, size_t length){
                                 const size_t maxTextMessageSize = sizeof(*textmessage_packet);
                                 if (serverHeader->length > maxTextMessageSize) {
                                     qDebug() << "无效的聊天记录长度：" << serverHeader->length << "聊天记录的长度应该为:" << maxTextMessageSize;
                                     acceptMessage(); // 继续监听后续消息
                                     return;
                                 }
                                 if (ec) {
                                     qDebug() << "读取错误:" << ec.message().c_str();
                                     return;
                                 }
                                 long long* fileSize = new long long ;
                                 asio::async_read(*server_socket, asio::buffer(fileSize, sizeof(long long)),
                                                  [this , fileSize](asio::error_code ec, size_t length){
                                                     qDebug() << "接收到文件消息";
                                                     qDebug() << textmessage_packet->SenderName << "发送给" << textmessage_packet->ReceiverName << "文件：" << textmessage_packet->TextMessage << "发送时间为" << textmessage_packet->messageTime << "文件大小为：" << *fileSize ;

                                                     MessageHistory tmp;
                                                     tmp.messageType = 1;
                                                     std::memset(tmp.ReceiverName , '\0' , sizeof(tmp.ReceiverName));
                                                     std::memset(tmp.SenderName , '\0' , sizeof(tmp.SenderName));
                                                     std::memset(tmp.messageTime , '\0' , sizeof(tmp.messageTime));
                                                     strncpy(tmp.ReceiverName , textmessage_packet->ReceiverName , sizeof(tmp.ReceiverName) - 1);
                                                     strncpy(tmp.SenderName , textmessage_packet->SenderName , sizeof(tmp.SenderName) - 1);
                                                     tmp.Message = QString(textmessage_packet->TextMessage);
                                                     strncpy(tmp.messageTime , textmessage_packet->messageTime , sizeof(tmp.messageTime) - 1);
                                                     message.push_back(tmp);
                                                     int n = 7;
                                                     int len = strlen(tmp.messageTime);

                                                     if (len >= n) {
                                                         tmp.messageTime[len - n] = '\0'; // 在新的末尾位置添加空字符
                                                     }

                                                     emit newFileMessage(tmp.SenderName,tmp.ReceiverName,tmp.Message,tmp.messageTime,*fileSize);
                                                     delete fileSize;
                                                     acceptMessage();
                                                });
                             });


        }
        else if(serverHeader->message_type == acceptChatRequestHeader){
            addFrirendReauest request;
            std::memset(request.ReceiverName , '\0' , sizeof(request.ReceiverName));
            std::memset(request.SenderName , '\0' , sizeof(request.SenderName));
            asio::read(*server_socket,asio::buffer(&request, sizeof(addFrirendReauest)));
            qDebug() << request.SenderName << "发来了通讯请求";
            emit acceptNewVoiceChat(request.SenderName , request.ReceiverName);
            acceptMessage();
        }
        else if(serverHeader->message_type == voiceChatRespone)
        {
            bool is;
            asio::read(*server_socket , asio::buffer(&is , sizeof(bool)));
            qDebug() << "是否接受了通讯请求" << is;
            emit isAcceptVoiceChat(is);
            if(is == true)
            {
                emit requestStartCapture();
            }
            else
            {
                emit requestStopCapture();
            }
            acceptMessage();
        }
        else{
            acceptMessage();
        }
    });
    // 启动事件循环线程
}

void chat_server::startAcceptMessage()
{
    acceptMessage();

    io_thread_ = std::thread([this] {
        qDebug() << "Network thread ID:" << QThread::currentThreadId();
        server_io_context->run();
        qDebug() << "Network thread exited";
    });
}

void chat_server::requestAccessFriendAdd()
{
    PacketHeader header{FriendAddRequest};
    std::vector<char> header_buffer(sizeof(PacketHeader));
    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
    asio::write(*server_socket , asio::buffer(header_buffer));
}

void chat_server::acceptPictureMessage(std::string *pictureMessage, std::string receiverName , std::string senderName , std::string messageTime)
{
    pictureMessagePacket* picture = new pictureMessagePacket;
    std::memset(picture->TextMessage , 0 , sizeof(pictureMessagePacket));
    asio::async_read(*server_socket ,
                     asio::buffer(picture , sizeof(pictureMessagePacket)) ,
                     [&,picture,pictureMessage , receiverName , senderName ,messageTime](asio::error_code ec, size_t length){
                         *pictureMessage += picture->TextMessage;
                         if(picture->isLastPacket)
                         {
                             //添加到历史记录
                             MessageHistory tmp;
                             tmp.messageType = 1;
                             std::memset(tmp.ReceiverName , '\0' , sizeof(tmp.ReceiverName));
                             std::memset(tmp.SenderName , '\0' , sizeof(tmp.SenderName));
                             std::memset(tmp.messageTime , '\0' , sizeof(tmp.messageTime));
                             strncpy(tmp.ReceiverName , receiverName.c_str() , sizeof(tmp.ReceiverName) - 1);
                             strncpy(tmp.SenderName , senderName.c_str() , sizeof(tmp.SenderName) - 1);
                             tmp.Message = QString::fromStdString(*pictureMessage);
                             strncpy(tmp.messageTime , messageTime.c_str() , sizeof(tmp.messageTime) - 1);
                             message.push_back(tmp);
                             int n = 7; // 要删除的字符数
                             int len = strlen(tmp.messageTime);

                             if (len >= n) {
                                 tmp.messageTime[len - n] = '\0'; // 在新的末尾位置添加空字符
                             }
                             emit newTextMessage(tmp.SenderName,tmp.ReceiverName,tmp.Message,tmp.messageTime);
                             delete pictureMessage;
                             delete picture;
                             acceptMessage();
                         }
                         else
                         {
                             delete picture;
                             acceptPictureMessage(pictureMessage , receiverName , senderName , messageTime);
                         }
                     });
}

Q_INVOKABLE  bool chat_server::searchUser(const QString& userNameQstr)
{

    if(!isConnect)
        connectToServer();
    if(!isConnect)
        return false;

    qDebug() << "正在搜索好友" << userNameQstr;
    //发送包头
    qDebug() << "发送搜索好友包头" << userNameQstr;
    PacketHeader header{SerarchUser_MESSAGE};
    std::vector<char> header_buffer(sizeof(PacketHeader));
    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
    asio::write(*server_socket , asio::buffer(header_buffer));

    //发送搜索好友包
    qDebug() << "发送搜索好友包" << userNameQstr;
    serarchUserRequest request;
    std::memset(request.username , '\0' , sizeof(request.username));
    std::string userName = userNameQstr.toUtf8().constData();
    std::strncpy(request.username , userName.c_str() , sizeof(request.username) - 1);
    std::vector<char> request_buffer(sizeof(serarchUserRequest));
    memset(request_buffer.data(),'\0',sizeof(serarchUserRequest));
    std::memcpy(request_buffer.data() , &request ,sizeof(serarchUserRequest));
    asio::write(*server_socket , asio::buffer(request_buffer, sizeof(serarchUserRequest)));
    qDebug() << "发送成功" << userNameQstr;

    return false;

}

Q_INVOKABLE  void chat_server::sendMessqge(const QString &MessqgejsonStr)
{

}



//发送请求添加好友
Q_INVOKABLE void chat_server::sendAddFriend(const QString &ReceiverName)
{
    PacketHeader header{AddFriendsRequest};
    std::vector<char> header_buffer(sizeof(PacketHeader));
    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
    asio::write(*server_socket , asio::buffer(header_buffer));

    addFrirendReauest request;
    std::memset(request.ReceiverName , '\0' , sizeof(request.ReceiverName));
    std::memset(request.SenderName , '\0' , sizeof(request.SenderName));
    strncpy(request.SenderName , username.c_str() , sizeof(request.SenderName) - 1);
    std::string receivername = ReceiverName.toUtf8().constData();
    strncpy(request.ReceiverName , receivername.c_str() , sizeof(request.ReceiverName) - 1);
    std::vector<char> request_buffer(sizeof(addFrirendReauest));
    memset(request_buffer.data(),'\0',sizeof(addFrirendReauest));
    std::memcpy(request_buffer.data() , &request ,sizeof(addFrirendReauest));
    qDebug() << "申请添加" << receivername << "为好友";
    asio::write(*server_socket , asio::buffer(request_buffer, sizeof(addFrirendReauest)));

}

void chat_server::respondFriendRequest(const QString &SenderName, int statue)
{
    PacketHeader header{ResponeFriendRequest};
    std::vector<char> header_buffer(sizeof(PacketHeader));


    SendFriendRequestToClient request;
    std::memset(request.ReceiverName , '\0' , sizeof(request.ReceiverName));
    std::memset(request.SenderName , '\0' , sizeof(request.SenderName));
    strncpy(request.ReceiverName , username.c_str() , sizeof(request.SenderName) - 1);
    std::string senderName = SenderName.toUtf8().constData();
    strncpy(request.SenderName , senderName.c_str() , sizeof(request.ReceiverName) - 1);
    request.status_code = statue;
    std::vector<char> request_buffer(sizeof(SendFriendRequestToClient));
    memset(request_buffer.data(),'\0',sizeof(SendFriendRequestToClient));
    std::memcpy(request_buffer.data() , &request ,sizeof(SendFriendRequestToClient));
    header.length = sizeof(request);
    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
    asio::write(*server_socket , asio::buffer(header_buffer));
    asio::write(*server_socket , asio::buffer(request_buffer, sizeof(SendFriendRequestToClient)));
    qDebug() << username << "回应" << SenderName << "的好友申请";
}

void chat_server::sendTextMessage(const QString &reveiverName, const QString &TextMessage)
{

    if(!isConnect)
        connectToServer();
    if(!isConnect)
        return;
    //将消息添加到消息列表
    std::string receiverName = reveiverName.toUtf8().constData();
    std::string text = TextMessage.toUtf8().constData();
    MessageHistory his;
    his.messageType = 1;
    strncpy(his.ReceiverName , receiverName.c_str() , sizeof(his.ReceiverName) - 1);
    strncpy(his.SenderName , username.c_str() , sizeof(his.SenderName) - 1);
    his.Message = QString(text.c_str());
    //strncpy(his.Message , text.c_str() , sizeof(his.Message) - 1);
    message.push_back(his);




    TextMessagePacket request;
    std::memset(request.ReceiverName , 0 , sizeof(request.ReceiverName));
    std::memset(request.SenderName , 0 , sizeof(request.SenderName));
    std::memset(request.TextMessage , 0 , sizeof(request.TextMessage));
    strncpy(request.SenderName , username.c_str() , sizeof(request.SenderName) - 1);
    const std::string reveiver = reveiverName.toUtf8().constData();
    const std::string Text = TextMessage.toUtf8().constData();
    strncpy(request.ReceiverName , reveiver.c_str() , sizeof(request.ReceiverName) - 1);
    strncpy(request.TextMessage , Text.c_str() , sizeof(request.TextMessage) - 1);
    std::vector<char> request_buffer(sizeof(TextMessagePacket));
    memset(request_buffer.data(),'\0',sizeof(TextMessagePacket));
    std::memcpy(request_buffer.data() , &request ,sizeof(TextMessagePacket));


    PacketHeader header{TextMessageHeader};
    std::vector<char> header_buffer(sizeof(PacketHeader));
    header.length = sizeof(request);
    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
    asio::write(*server_socket , asio::buffer(header_buffer , sizeof(PacketHeader)));
    asio::write(*server_socket , asio::buffer(request_buffer, header.length));
    qDebug() << "发送消息:" << TextMessage <<"到用户:" << reveiverName;

}

//发送带图片的消息
void chat_server::sendLargeMessage_pictureOrText(const QString &reveiverName, const QString &Message)
{
    qDebug() << "接受到" << reveiverName << "的图片消息 , 图片的大小字节数为" << Message.size();
    const std::string receiverName = reveiverName.toUtf8().constData();
    const std::string text = Message.toUtf8().constData();
    MessageHistory his;
    his.messageType = 1;
    strncpy(his.ReceiverName , receiverName.c_str() , sizeof(his.ReceiverName) - 1);
    strncpy(his.SenderName , username.c_str() , sizeof(his.SenderName) - 1);
    his.Message = Message;
    message.push_back(his);

    //发送包头
    PacketHeader header{PictureMessageHeader};
    std::vector<char> header_buffer(sizeof(PacketHeader));
    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
    asio::write(*server_socket , asio::buffer(header_buffer , sizeof(PacketHeader)));



    //计算一共发送多少个包,并存储进一个数组
    std::vector<pictureMessagePacket> packets;
    const int packetSize = 2046;
    int messageLength = Message.length(); // 获取QString 的字符个数
    int offset = 0;
    while (offset < messageLength) {
        pictureMessagePacket packet;
        std::memset(packet.TextMessage , 0 , sizeof(pictureMessagePacket));
        // 计算当前包的大小，确保不超过 packetSize
        int currentPacketSize = std::min(packetSize, messageLength - offset);
        // 从 QString 中提取子字符串
        QString subString = Message.mid(offset, currentPacketSize);
        QByteArray byteArray = subString.toUtf8();
        const char* charArray = byteArray.constData();
        std::memcpy(packet.TextMessage, charArray, byteArray.length());
        packet.isLastPacket = (offset + packetSize >= messageLength);
        packets.push_back(packet);
        offset += packetSize;
    }
    qDebug() << "一共有" << packets.size() << "个包" ;



    //发送第一个包，告知发送者和接受者的身份信息
    pictureMessagePacket_first  pictureMessagePacket_0;
    std::memset(pictureMessagePacket_0.ReceiverName , 0 , sizeof(pictureMessagePacket_0.ReceiverName));
    std::memset(pictureMessagePacket_0.SenderName , 0 , sizeof(pictureMessagePacket_0.SenderName));
    std::memset(pictureMessagePacket_0.messageTime , 0 , sizeof(pictureMessagePacket_0.messageTime));
    strncpy(pictureMessagePacket_0.ReceiverName , receiverName.c_str() , sizeof(pictureMessagePacket_0.ReceiverName));
    strncpy(pictureMessagePacket_0.SenderName , username.c_str() , sizeof(pictureMessagePacket_0.SenderName));
    pictureMessagePacket_0.packetSize = packets.size();
    std::vector<char> pictureMessagePacket_0_buffer(sizeof(pictureMessagePacket_first));
    memset(pictureMessagePacket_0_buffer.data(),'\0',sizeof(pictureMessagePacket_first));
    std::memcpy(pictureMessagePacket_0_buffer.data() , &pictureMessagePacket_0 ,sizeof(pictureMessagePacket_first));
    asio::write(*server_socket , asio::buffer(pictureMessagePacket_0_buffer , sizeof(pictureMessagePacket_first)));


    //开始发送图片消息，每个包发送2046字节的包和1字节的标注是否是最后一个包

    for(int i = 0 ; i < packets.size() ; i++)
    {
        std::vector<char> pictureMessagePacket_ll(sizeof(pictureMessagePacket));
        memset(pictureMessagePacket_ll.data() , '\0' , sizeof(pictureMessagePacket));
        std::memcpy(pictureMessagePacket_ll.data() , &packets[i] ,sizeof(pictureMessagePacket));
        asio::async_write(*server_socket,
                          asio::buffer(pictureMessagePacket_ll , sizeof(pictureMessagePacket)) ,
                          [& , packets , i](asio::error_code ec, std::size_t length){
                              //qDebug() << "发送第" << i << "个包，包大小:" << sizeof(pictureMessagePacket);
                          });
    }
    //qDebug() << Message;

}

bool chat_server::hasImage() const
{
    return QGuiApplication::clipboard()->mimeData()->hasImage();
}

QString chat_server::getImageBase64() const
{
    const QImage image = QGuiApplication::clipboard()->image();
    if(image.isNull()) return "";

    QByteArray byteArray; // 存储二进制数据
    QBuffer buffer(&byteArray); // 内存缓冲区
    buffer.open(QIODevice::WriteOnly); // 只写模式打开
    image.save(&buffer, "PNG");  // 转换为PNG格式
    return QString("data:image/png;base64," + byteArray.toBase64());
}

bool chat_server::hasText() const
{
    return QGuiApplication::clipboard()->mimeData()->hasText();
}

QString chat_server::getText() const
{
    return QGuiApplication::clipboard()->text();
}

void chat_server::openImageFileDialog()
{
    QString filePath = QFileDialog::getOpenFileName(nullptr,
                                                    tr("选择图片"),
                                                    QDir::homePath(),
                                                    tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)"));

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray imageData = file.readAll();
            file.close();

            // 获取文件MIME类型
            QMimeDatabase db;
            QMimeType mime = db.mimeTypeForFile(filePath);
            QString mimeType = mime.name();

            // 转换为Base64
            QByteArray base64Data = imageData.toBase64();
            QString base64String = "data:" + mimeType + ";base64," + QString::fromLatin1(base64Data);

            // 发送到QML
            emit newPictureMessage(base64String);
        }
    }
}

void chat_server::openFileDialog(const QString &receiverName, const QString &senderName)
{
    QFileDialog dialog(nullptr, tr("选择发送文件"));
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setLabelText(QFileDialog::Accept, tr("确认")); // 更改按钮文本

    while (dialog.exec() == QDialog::Accepted) {
        QStringList filePaths = dialog.selectedFiles();
        if (!filePaths.isEmpty()) {
            for (const QString &filePath : filePaths) {
                qDebug() << "选择的文件：" << filePath;
                QFileInfo fileInfo(filePath);
                QString fileName = fileInfo.fileName();
                bool is;
                for(auto tmp : message)
                {
                    if((tmp.Message == filePath && tmp.ReceiverName == receiverName) || (tmp.Message == fileName && tmp.ReceiverName==receiverName))
                    {
                        is = false;
                        QMessageBox::about(NULL , "失败" , "存在相同文件" + fileName);
                        break;
                    }
                    else{
                        is = true;
                    }

                }
                if(is)
                    sendFile(receiverName , senderName , filePath);

            }
            break;
        } else {
            qDebug() << "用户取消选择或未选择任何文件";
            break;
        }
    }

}

void chat_server::openFilePath(QString FileName, QString sender, QString receiver)
{
    qDebug() << "文件" << FileName << "发送者" << sender << "接受者" << receiver ;
    for(auto msg : message)
    {
        QFileInfo fileInfo(msg.Message);
        if(fileInfo.fileName() == FileName && msg.SenderName == sender && msg.ReceiverName == receiver)
        {
            QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(msg.Message)});
            break;
        }

    }
}

void chat_server::downLoadFile(const QString &receiverName, const QString &senderName, QString FileName, QString SavePath)
{
    qDebug() << senderName << "给" << receiverName << "发送了文件" << FileName << "保存在" << SavePath;
    QString filePath = SavePath + FileName;
    QFileInfo fileInfo(filePath);
    QDir dir(fileInfo.absolutePath());
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qDebug() << "无法创建目录:" << dir.path();
        }
    }

    // 打开文件（如果不存在则创建）
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::WriteOnly)) {
        qDebug() << "无法打开文件:" << file->errorString();
    }

    for(auto& msg : message)
    {
        QFileInfo fileInfo(filePath);
        if(fileInfo.fileName() == msg.Message && msg.SenderName == senderName && msg.ReceiverName == receiverName)
        {
            msg.Message = filePath;
        }
    }



    //连接传输文件的socket
    // 解析服务器地址和端口，得到服务器的端点（endpoint）
    asio::ip::tcp::resolver resolver(*server_io_context);
    auto endpoints = resolver.resolve(server_ip.toStdString(),std::to_string(server_port));
    //创建传输文件的socket
    asio::ip::tcp::socket* ReciverFileSocekt = new asio::ip::tcp::socket(*server_io_context);
    asio::connect(*ReciverFileSocekt,endpoints);

    //1、发送包头
    PacketHeader header{ReceiveFileMessageHeader};
    std::vector<char> header_buffer(sizeof(PacketHeader));
    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
    asio::write(*ReciverFileSocekt , asio::buffer(header_buffer , sizeof(PacketHeader)));

    // 2. 发送文件发送者信息
    QByteArray senderNameUtf8 = senderName.toUtf8();
    int senderNameLength = senderNameUtf8.length();
    asio::write(*ReciverFileSocekt, asio::buffer(&senderNameLength, sizeof(int))); // 发送长度
    asio::write(*ReciverFileSocekt, asio::buffer(senderNameUtf8.data(), senderNameLength)); // 发送数据

    //3.发送文件名
    QByteArray fileNameUtf8 = FileName.toUtf8(); // 转换为 QByteArray
    int fileNameLength = fileNameUtf8.length();
    asio::write(*ReciverFileSocekt, asio::buffer(&fileNameLength, sizeof(int))); // 发送长度
    asio::write(*ReciverFileSocekt, asio::buffer(fileNameUtf8.data(), fileNameLength)); // 发送数据

    //4、文件接受者
    QByteArray receiverNameUtf8 = receiverName.toUtf8();
    int receiverNameLength = receiverNameUtf8.length();
    asio::write(*ReciverFileSocekt, asio::buffer(&receiverNameLength, sizeof(int))); // 发送长度
    asio::write(*ReciverFileSocekt, asio::buffer(receiverNameUtf8.data(), receiverNameLength)); // 发送数据

    //5、接受文件大小
    long long* fileSize = new long long ;
    asio::read(*ReciverFileSocekt, asio::buffer(fileSize, sizeof(long long))); // 发送长度
    long long* size = new long long(*fileSize);
    qDebug() << "文件大小为：" << *fileSize;



    int fileIndex = 0;
    for(auto tmp : message)
    {
        if((tmp.ReceiverName == receiverName && tmp.SenderName == senderName) || (tmp.SenderName == receiverName && tmp.ReceiverName == senderName))
        {
            fileIndex++;
            QFileInfo fileInfo(tmp.Message);
            if(fileInfo.fileName() == FileName)
            {
                break;
            }
        }
    }

    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, [=](){
        float trans = 100.0 -  ((((double)(*size))/(*fileSize)) * 100.0);
        //qDebug() << "还剩下" << *size << "没传输，传输进度" << trans << "%";
        emit downloadFileTransProgress(FileName ,  senderName , receiverName , trans , fileIndex);

        if(*size <= 65535)
        {
            qDebug()  << "传输进度" << "100.00%";
            emit downloadFileTransProgress(FileName ,  senderName , receiverName , 100.00 , fileIndex);
            delete size;
            timer->stop();
        }
    });
    timer->start();

    receiverFile(size , file , FileName , ReciverFileSocekt);

}

void chat_server::selectDownLoadFilePath(const QString &receiverName, const QString &senderName, QString FileName)
{
    QFileDialog dialog(nullptr, tr("另存为"));
    dialog.setFileMode(QFileDialog::Directory);       // 设置为选择文件夹模式
    dialog.setOption(QFileDialog::ShowDirsOnly, true); // 仅显示文件夹
    dialog.setLabelText(QFileDialog::Accept, tr("确认")); // 更改确认按钮文本

    if(dialog.exec() == QDialog::Accepted)
    {
        QString folderPath = dialog.selectedFiles().value(0); // 取第一个路径（单选）
        if(!folderPath.isEmpty())
        {
            downLoadFile(receiverName , senderName , FileName , folderPath + '/');
        }
    }
}

QString chat_server::QmlHtmlPro(QString html)
{
    QTextDocument doc;
    doc.setHtml(html);

    for (QTextBlock block = doc.begin(); block != doc.end(); block = block.next()) {
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (fragment.isValid() && fragment.charFormat().isImageFormat()) {
                return "[图片]";
            }
        }
    }
    return doc.toPlainText();
}

//由于与申请好友的包类似，所以直接复制过来了
void chat_server::sendVoiceChat(QString senderName, QString receiverName)
{
    audioChatName = (senderName.toStdString() == username ? receiverName : senderName);
    qDebug() << senderName << "向" << receiverName << "发起通讯请求";
    PacketHeader header{voiceChatRequestHeader};
    std::vector<char> header_buffer(sizeof(PacketHeader));
    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
    asio::write(*server_socket , asio::buffer(header_buffer));

    addFrirendReauest request;
    std::memset(request.ReceiverName , '\0' , sizeof(request.ReceiverName));
    std::memset(request.SenderName , '\0' , sizeof(request.SenderName));
    strncpy(request.SenderName , username.c_str() , sizeof(request.SenderName) - 1);
    std::string receivername = receiverName.toUtf8().constData();
    strncpy(request.ReceiverName , receivername.c_str() , sizeof(request.ReceiverName) - 1);
    std::vector<char> request_buffer(sizeof(addFrirendReauest));
    memset(request_buffer.data(),'\0',sizeof(addFrirendReauest));
    std::memcpy(request_buffer.data() , &request ,sizeof(addFrirendReauest));
    asio::write(*server_socket , asio::buffer(request_buffer, sizeof(addFrirendReauest)));
}

void chat_server::responeVoicdChat(QString senderName, QString receiverName, uint16_t status_code)
{
    audioChatName = (senderName.toStdString() == username ? receiverName : senderName);
    if(status_code  == 2)
    {
        emit requestStopCapture();
        //return;
    }
    PacketHeaderServer FriendListHeader{responeVoiceChatHeader};
    std::vector<char> header_buffer(sizeof(PacketHeaderServer));
    std::memcpy(header_buffer.data() , &FriendListHeader ,sizeof(PacketHeaderServer));
    asio::write(*server_socket,asio::buffer(header_buffer, sizeof(PacketHeaderServer)));

    SendFriendRequestToClient request;
    std::memset(request.ReceiverName , '\0' , sizeof(request.ReceiverName));
    std::memset(request.SenderName , '\0' , sizeof(request.SenderName));
    std::strncpy(request.SenderName , senderName.toStdString().c_str() , sizeof(request.SenderName));
    std::strncpy(request.ReceiverName , receiverName.toStdString().c_str() , sizeof(request.ReceiverName));
    request.status_code = status_code;
    std::vector<char> request_buffer(sizeof(SendFriendRequestToClient));
    memset(request_buffer.data(),'\0',sizeof(SendFriendRequestToClient));
    std::memcpy(request_buffer.data() , &request ,sizeof(SendFriendRequestToClient));
    qDebug() << senderName << " " << receiverName << " " << request.status_code;
    asio::write(*server_socket , asio::buffer(request_buffer, sizeof(SendFriendRequestToClient)));
    if(status_code == 1)
    {
        emit requestStartCapture();
    }


}

void chat_server::sendFile_tuoru(QString filePath, const QString &receiverName, const QString &senderName)
{
    QString file;
    if (filePath.startsWith("file:///")) {
        file = filePath.mid(8);
    } else if (filePath.startsWith("file://")) {
        file = filePath.mid(7);
    } else if (filePath.startsWith("file:/")) {
        file = filePath.mid(6);
    }
    qDebug() << "文件路径：" << file << "接受者：" << receiverName << "发送者：" << senderName;
    sendFile(receiverName , senderName , file);

}


void chat_server::sendFileMessage(long long* remainingSize , QFile* inputFile , QString fileName)
{
    const size_t chunkSize = 65535;
    char* data = new char[chunkSize];

    size_t readLength = std::min(*remainingSize, static_cast<long long>(chunkSize));
    inputFile->read(data,readLength);
    //qDebug() << data ;


    asio::async_write(*fileSocekt[fileName.toStdString()] , asio::buffer(data , readLength),
                      [this, remainingSize, inputFile, data, readLength , fileName](asio::error_code ec, std::size_t length){
                            if (ec) {
                                // 处理错误
                                delete[] data;
                                return;
                            }

                            delete[] data;

                            if (readLength == chunkSize) {
                                // 递归发送下一个包
                                *remainingSize = (*remainingSize) - chunkSize;
                                sendFileMessage(remainingSize, inputFile , fileName);
                            } else {
                                // 全部读取完成后再关闭
                                fileSocekt[fileName.toStdString()]->close();
                                delete fileSocekt[fileName.toStdString()];
                                isTransFile = false;
                                inputFile->close();
                                delete inputFile;
                            }
                      });
}

void chat_server::sendFile(const QString &receiverName, const QString &senderName , QString filePath)
{
    qDebug() << filePath;
    QFile* file = new QFile(filePath);
    file->open(QIODevice::ReadOnly);
    //发送包头
    PacketHeader header{FileMessageHeader};
    std::vector<char> header_buffer(sizeof(PacketHeader));
    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
    asio::write(*server_socket , asio::buffer(header_buffer , sizeof(PacketHeader)));

    // 2. 发送接收者信息
    QByteArray receiverNameUtf8 = receiverName.toUtf8();
    int receiverNameLength = receiverNameUtf8.length();
    asio::write(*server_socket, asio::buffer(&receiverNameLength, sizeof(int))); // 发送长度
    asio::write(*server_socket, asio::buffer(receiverNameUtf8.data(), receiverNameLength)); // 发送数据

    //3.发送文件名
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName(); // 获取文件名，类型为 QString
    QByteArray fileNameUtf8 = fileName.toUtf8(); // 转换为 QByteArray
    int fileNameLength = fileNameUtf8.length();
    asio::write(*server_socket, asio::buffer(&fileNameLength, sizeof(int))); // 发送长度
    asio::write(*server_socket, asio::buffer(fileNameUtf8.data(), fileNameLength)); // 发送数据

    //4.发送文件大小
    long long fileSize = file->size();
    qDebug() << "文件大小：" << fileSize;
    asio::write(*server_socket, asio::buffer(&fileSize, sizeof(long long))); // 发送长度


    //连接传输文件的socket
    // 解析服务器地址和端口，得到服务器的端点（endpoint）
    asio::ip::tcp::resolver resolver(*server_io_context);
    auto endpoints = resolver.resolve(server_ip.toStdString(),std::to_string(server_port));
    //创建传输文件的socket
    fileSocekt[fileName.toStdString()] = new asio::ip::tcp::socket(*server_io_context);
    asio::connect(*fileSocekt[fileName.toStdString()],endpoints);

    //const char* name = username.c_str();
    char name[256];
    memset(name , '\0' , 256);
    strncpy(name, username.c_str(), 255); // 复制最多 255 个字符，留一个位置给 '\0'
    name[255] = '\0'; // 确保字符串以 null 结尾

    PacketHeader socket_header{FileSocket};
    header.length = username.length();
    qDebug() << header.length;
    std::vector<char> socket_header_buffer(sizeof(PacketHeader));
    std::memcpy(socket_header_buffer.data() , &socket_header ,sizeof(PacketHeader));
    asio::write(*fileSocekt[fileName.toStdString()] , asio::buffer(socket_header_buffer , sizeof(PacketHeader)));
    asio::write(*fileSocekt[fileName.toStdString()] , asio::buffer(name , 256));
    qDebug() << name;
    asio::write(*fileSocekt[fileName.toStdString()], asio::buffer(&fileNameLength, sizeof(int))); // 发送长度
    asio::write(*fileSocekt[fileName.toStdString()], asio::buffer(fileNameUtf8.data(), fileNameLength)); // 发送数据
    // char tmp[19];
    // asio::read(*fileSocekt[fileName.toStdString()], asio::buffer(tmp, 19));
    // qDebug() << tmp;


    emit newFileMessage_send(fileName , senderName , receiverName , fileSize);//通知qml发送文件

    MessageHistory his;
    his.messageType = 2;
    strncpy(his.ReceiverName , receiverName.toStdString().c_str() , sizeof(his.ReceiverName) - 1);
    strncpy(his.SenderName , username.c_str() , sizeof(his.SenderName) - 1);
    his.Message = filePath;
    message.push_back(his);
    int fileIndex = 0;
    for(auto tmp : message)
    {
        if((tmp.ReceiverName == receiverName && tmp.SenderName == username) || (tmp.SenderName == receiverName && tmp.ReceiverName == username))
            fileIndex++;
    }



    //发送数据包，每个包65535
    long lastPacketLength = fileSize % 65535;
    long packetSize;
    if(lastPacketLength != 0)
        packetSize = (fileSize / 65535) + 1 ;
    else
        packetSize = fileSize / 65535 ;

    qDebug() << "一共需要发送" << packetSize << "个包 ，最后一个包大小为: " << lastPacketLength;

    long long* size = new long long(fileSize);
    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, [=](){
        float trans = 100.0 -  ((((double)(*size))/((double)fileSize)) * 100.0);
        //qDebug() << "还剩下" << *size << "没传输，传输进度" << trans << "%";
        newFileMessage_send_TransProgress(fileName ,  senderName , receiverName , trans , fileIndex);

        if(*size <= 65535)
        {
            qDebug()  << "传输进度" << "100.00%";
            newFileMessage_send_TransProgress(fileName ,  senderName , receiverName , 100.00 , fileIndex);
            delete size;
            timer->stop();
        }
    });
    timer->start();
    sendFileMessage(size , file , fileName);

}

void chat_server::receiverFile(long long *remainingSize, QFile *inputFile, QString fileName , asio::ip::tcp::socket* receiverFileSocekt)
{
    const size_t chunkSize = 65535;
    char* data = new char[chunkSize];
    //std::memset(data , '\0' , 65535);
    size_t readLength = std::min(*remainingSize, static_cast<long long>(chunkSize));


    asio::async_read(*receiverFileSocekt , asio::buffer(data , readLength),
                      [this, remainingSize, inputFile, data, readLength , fileName , receiverFileSocekt](asio::error_code ec, std::size_t length){
                          if (ec) {
                              delete[] data;
                              return;
                          }

                          inputFile->write(data , readLength);

                          if (readLength == chunkSize) {
                              // 递归发送下一个包
                              delete[] data;
                              *remainingSize = (*remainingSize) - chunkSize;
                              receiverFile(remainingSize, inputFile , fileName , receiverFileSocekt);
                          } else {
                              // 全部读取完成后再关闭
                              delete[] data;
                              receiverFileSocekt->close();
                              delete receiverFileSocekt;
                              inputFile->close();
                              delete inputFile;
                          }
                      });
}

void chat_server::startAudioChat()
{
    if(!audioChat)
    {
        audioChat = new AudioHandler;
    }

    // QString filePath = "C:/Users/wumo/Desktop/新建文件夹 (3)/1.pcm"; //
    // QFileInfo fileInfo(filePath);
    // QDir dir(fileInfo.absolutePath());
    // if (!dir.exists()) {
    //     if (!dir.mkpath(".")) {
    //         qDebug() << "无法创建目录:" << dir.path();
    //     }
    // }

    // // 打开文件（如果不存在则创建）
    // QFile* AudioFile = new QFile(filePath);
    // if (!AudioFile->open(QIODevice::WriteOnly)) {
    //     qDebug() << "无法打开文件:" << AudioFile->errorString();
    // }

    if(!SendAudioSocekt)
    {
        asio::ip::tcp::resolver resolver(*server_io_context);
        auto endpoints = resolver.resolve(server_ip.toStdString(),std::to_string(server_port));
        //创建传输音频的socket
        SendAudioSocekt = new asio::ip::tcp::socket(*server_io_context);
        asio::connect(*SendAudioSocekt,endpoints);


        //发送包头
        PacketHeader header{transAudioToServer};
        std::vector<char> header_buffer(sizeof(PacketHeader));
        std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
        asio::write(*SendAudioSocekt , asio::buffer(header_buffer , sizeof(PacketHeader)));

        int length = audioChatName.length();
        asio::write(*SendAudioSocekt , asio::buffer(&length , sizeof(int)));
        asio::write(*SendAudioSocekt , asio::buffer(audioChatName.toStdString().c_str() , length));

        length = username.length();
        asio::write(*SendAudioSocekt , asio::buffer(&length , sizeof(int)));
        asio::write(*SendAudioSocekt , asio::buffer(username.c_str() , length));

        connect(audioChat , &AudioHandler::audioDataCaptured , this , [this](const QByteArray &data){
            int size = data.length();
            asio::write(*SendAudioSocekt , asio::buffer(&size , sizeof(int)));
            asio::write(*SendAudioSocekt , asio::buffer(data.data() , size));
            // AudioFile->write(data);
            // audioChat->receiveAudioData(data);
        });
    }

    audioChat->initialize();


    startAcceptAudio();
    audioChat->startCapture();
    audioChat->startPlayback();
}

void chat_server::stopAudioChat()
{
    if(audioChat) {
        //disconnect(audioChat, &AudioHandler::audioDataCaptured, this, nullptr); // 断开所有与 this 相关的 audioDataCaptured 信号的连接
        audioChat->stopCapture();
        audioChat->stopPlayback();
        int length = 0;
        asio::write(*SendAudioSocekt , asio::buffer(&length , sizeof(int)));
        // if(AudioFile) {
        //     AudioFile->close();
        //     delete AudioFile;
        //     AudioFile = nullptr;
        // }
    }
}

void chat_server::startAcceptAudio()
{
    int* size = new int;
    asio::async_read(*SendAudioSocekt , asio::buffer(size , sizeof(int)) , [this , size](asio::error_code ec, size_t length){
        if(ec)
        {
            return;
        }
        if(*size == 0)
        {
            emit stopAudio();
            return;
        }
        char* data = new char[(*size) + 1];
        memset(data , '\0' , *size + 1);
        asio::async_read(*SendAudioSocekt , asio::buffer(data , *size) , [this , size ,data](asio::error_code ec, size_t length){
            if(ec)
            {
                return;
            }
            qDebug() << "接受到音频" << *size;
            QByteArray audio(data, *size);
            audioChat->receiveAudioData(audio);
            startAcceptAudio();
        });
    });
}

void chat_server::stopvideo()
{
    qDebug() << "结束通讯";
    //disconnect(audioChat, &AudioHandler::audioDataCaptured, this, nullptr);
    audioChat->stopCapture();
    audioChat->stopPlayback();
    int length = 1;
    asio::write(*SendAudioSocekt , asio::buffer(&length , sizeof(int)));

}
