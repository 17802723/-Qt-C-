#include <iostream>
#include <asio.hpp>
#include <string>
#include <vector>
#include <unordered_map>
enum MessageType {
    Register_REQUEST = 0,//注册请求
    LOGIN_REQUEST = 1,//登录请求
    LOGIN_RESPONSE = 2,//
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
    ReceiveFileMessageHeader = 19 , //从服务器下载文件
    voiceChatRequestHeader = 20 , //发起通讯请求
    acceptChatRequestHeader = 21 , //接收到通讯请求
    responeVoiceChatHeader = 22 , //回应电话通讯
    voiceChatRespone = 23 , //服务器给客户端回应语音消息
    transAudioToServer = 24 //客户端传输音频给服务器
};

//客户端给服务端发的包头
#pragma pack(push, 1)  // 1字节对齐
struct PacketHeader {
    uint16_t message_type;  // 消息类型（Login, Text, File, etc.）
    size_t length; //包长度
};
#pragma pack(pop)

//服务端发给客户端的包头
#pragma pack(push, 1)
struct PacketHeaderServer
{
    uint16_t message_type;  // 消息类型
    size_t length; //包长度
};
#pragma pack(pop)

//注册请求包
#pragma pack(push, 1)
struct RegisterRequest {
    char username[256];     // 用户名
    char password[256];     // 密码
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
    char TextMessage[2046] = {};   //消息   
    bool isLastPacket; //是否是最后一个包
};
#pragma pack(pop)

class ChatClient
{
private: 
    std::string userPassword;
    std::string userIp;
    std::string userPort;
    std::shared_ptr<asio::ip::tcp::socket> clientSocket;
    std::unordered_map<std::string , std::shared_ptr<asio::ip::tcp::socket>> fileSocekt;
    //std::shared_ptr<asio::ip::tcp::socket> fileSocekt;//用于传输文件的socket
    bool isTransFile = false;//定位fileSocket是否正在被使用
    bool isAccept = false;
    RegisterRequest* request;
    serarchUserRequest* serachUser ;
    addFrirendReauest* FrirendRequest;
    std::vector<std::string> friendsList;
    //消息队列，1/2为文本消息，3/4为文件消息，1/3代表当前用户为发送者，2/4代表当前用户为接受者
    std::vector<std::unordered_map<int , std::string>> message;
    TextMessagePacket textMessage;
    TextMessagePacket* testMessageToClient;
    TextMessagePacket* textMsg ;
    std::unordered_map<std::string , std::ofstream*> outFile;
    std::unordered_map<std::string , long long> fileSizeAll;
    std::unordered_map<std::string , std::string> fileReceiver;
    std::string audioChatCurrentName;//语音通话时用于标记当前socket是哪个用户的


    
public:
    std::string userName;
    //绑定到一个新客户
    void Session(std::shared_ptr<asio::ip::tcp::socket> socket);
    //接受客户端发送的信息
    void acceptMessage();
    //客户端申请注册用户
    void RegisterUser( RegisterRequest* request);
    //客户端申请登录
    void LoginUser(RegisterRequest* request);
    //发送数据库中的好友申请表到客户端
    void returnAddfriend(std::string SenderName , std::string ReceiverName ,uint16_t status_code);
    //刷新好友列表并发送给客户端
    void sendFriendList();
    //向客户端发送聊天记录
    void sendMessageHistory();
    //接受带图片的消息
    void acceptPictureMessage(std::string* pictureMessage , std::string receiverName);
    //接受文件消息
    void acceptFileMessage(long long remainingSize , std::ofstream* outputFile  , std::string fileName );
    //客户端下载文件
    void sendFile(long long remainingSize , std::ifstream * outputFile  , std::string fileName);
    //申请电话通讯
    void sendVoiceChatRequest(std::string SenderName , std::string ReceiverName);
    //回应电话通讯
    void responeVoice(bool status);
    //接受到客户端的音频数据
    void acceptAudioMessage(std::string receiveName);
    ChatClient();
    ~ChatClient();

};
extern std::vector<ChatClient*> client;



