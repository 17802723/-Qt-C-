#include <mysql/mysql.h>
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;
class userDB 
{
private:
    string host_;     // 数据库主机地址
    string user_;     // 数据库用户名
    string passwd_;   // 数据库密码
    string dbname_;   // 数据库名称
    MYSQL* conn_;     // MySQL连接对象
    MYSQL_RES* res;   //mysql查询结果
public:
    //参数为主机地址，数据库用户名，数据库密码，数据库名称
    userDB(const string& host, const string& user, const string& passwd, const string& dbname);
    ~userDB();
    //连接至数据库
    bool connect();
    //执行增、删、改的语句
    bool executeQuery(const string& query);
    //获取查询结果
    MYSQL_RES* fetchResults(const string& query);
    //判断用户是否存在
    bool userExists(const string& username);
    //添加新用户
    bool insertUser(const string& username, const string& password);
    //用户登录检查用户密码是否正确
    int checkLogin(const string& username, const string& inputPassword);
    //请求添加好友
    bool sendFriendRequest(const string& sender_name, const string& receiver_name);
    //根据用户名查询用户id
    std::string serachUserId(const string& userName);
    //根据用户id查询用户名
    std::string serachUserName(const string& id);
    //好友申请表遍历插入相应用户
    void insertFrienaRequestTo();
    //客户端回应好友申请，修改好友申请表的状态值
    void changeFriendRequestState(const string& senderName , const string& receiveName , const int& state);
    //添加好友
    void addFriend(const string& friendName , const string& userName );
    //获取好友列表
    std::vector<std::string> getFriendList(const string& userName);
    //发送消息到客户端
    std::vector<std::unordered_map<int , std::string>> sendMessage(std::string username, std::vector<std::string>& friednName , std::vector<std::string>& messageTime);
    //添加文本消息
    void addTextMessage(std::string senderName , std::string receiverName , std::string textMessage);
    //添加图片或文件消息
    void addPictureOrFileMessage(std::string senderName , std::string receiverName , std::string fileOrPicture);
    //添加文件消息
    void addFileMessage(std::string senderName , std::string receiverName , std::string fileOrPicture);
};

extern userDB usersDataBases;