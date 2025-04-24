#include "userMysql.h"
#include "chatClient.h"
userDB usersDataBases( "localhost" , "root" , "762300" , "user");

userDB::userDB(const string& host, const string& user, const string& passwd, const string& dbname)
                :host_(host), user_(user), passwd_(passwd), dbname_(dbname), conn_(nullptr) 
{
    connect();
}



userDB::~userDB()
{
    if (conn_ != nullptr) {
        mysql_close(conn_);  // 关闭数据库连接
    }
}

bool userDB::connect()
{
    // 初始化MySQL对象
    conn_ = mysql_init(nullptr);
    if (conn_ == nullptr) {
        cerr << "mysql_init() failed" << endl;  // 如果初始化失败，输出错误信息
        return false;  // 返回连接失败
    }

    // 使用提供的用户名、密码、数据库名等信息进行连接
    if (mysql_real_connect(conn_, host_.c_str(), user_.c_str(), passwd_.c_str(), dbname_.c_str(), 0, nullptr, 0) == nullptr) {
        cerr << "mysql_real_connect() failed: " << mysql_error(conn_) << endl;  // 如果连接失败，输出错误信息
        mysql_close(conn_);  // 关闭连接
        return false;  // 返回连接失败
    }

    return true;  // 连接成功
}

bool userDB::executeQuery(const string& query) {
    if (mysql_query(conn_, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn_) << endl;
        return false;
    }
    return true;
}

MYSQL_RES* userDB::fetchResults(const string& query) {
    // 执行查询
    if (mysql_query(conn_, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn_) << endl;
        return nullptr;  // 如果查询失败，返回nullptr
    }
     // 获取查询结果
     res = mysql_store_result(conn_);
     if (res == nullptr) {
         cerr << "mysql_store_result() failed: " << mysql_error(conn_) << endl;
         return nullptr;  // 如果获取结果失败，返回nullptr
     }

     return res;  // 返回查询结果
}


bool userDB::userExists(const string& username) {
    // 查询用户表，检查用户名是否存在
    string query = "SELECT COUNT(*) FROM users WHERE username = '" + username + "'";
    res = fetchResults(query);  // 执行查询并获取结果
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row) {
            // 获取查询结果中的计数值
            int count = atoi(row[0]);  // 将结果转换为整数
            mysql_free_result(res);  // 释放查询结果
            return count > 0;  // 如果计数大于0，说明用户存在
        }
        mysql_free_result(res);  // 如果没有查询到数据，也释放结果
    }
    return false;  // 如果查询失败或没有找到用户，返回false
}

bool userDB::insertUser(const string& username, const string& password)
{
    if(userExists(username))
    {
        std::cout << "用户" << username << "存在\n";
        return false;
    }
    else
    {
        // 为"friends"字段手动指定空数组的默认值 "[]"
        string query = "INSERT INTO users (username, password, friends) VALUES ('" +
        username + "', '" + password + "', '[]')";
        bool isInsert = executeQuery(query);  // 执行插入操作
        if(isInsert)
            std::cout << "成功创建用户:" << username << "密码为:" << password << std::endl;
        return isInsert;
    }
}

int userDB::checkLogin(const string& username, const string& inputPassword)
{
    if(!userExists(username))
    {
        std::cout << username << "用户不存在\n" ;
        return 1;//用户不存在
    }
    else
    {
        // 构建查询SQL语句，查询指定用户名的用户信息
        string query = "SELECT password FROM users WHERE username = '" + username + "'";  
        res = fetchResults(query);
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                // 获取查询结果中的密码
                string pwd = row[0];
                if(inputPassword == pwd)
                {
                    //std::cout<<"用户密码正确\n";                   
                    mysql_free_result(res);  // 释放查询结果
                    std::cout << "用户密码正确\n";
                    return 0;//用户密码正确
                }
                else
                {
                    std::cout << "密码错误\n";
                    mysql_free_result(res);
                    return 2;//密码错误
                }
            }
            mysql_free_result(res);
            return 3;
              
        }

    }
    return 3;
}


std::string userDB::serachUserId(const string& userName)
{
    if(userName.empty())
    {
        std::cout << "用户名为空\n";
        return "";
    }
    string query = "SELECT id FROM users WHERE username = '" + userName + "'" ;  
    res = fetchResults(query);
    if (res)
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        std::cout <<row[0] << '\0';
        mysql_free_result(res);  // 释放查询结果
        return row[0];
    }
    else
    {
        mysql_free_result(res);  // 释放查询结果
        return NULL;
    }

}

std::string userDB::serachUserName(const string& id)
{
    std::string query = "SELECT username  FROM users WHERE id = " + id  ; 
    res = fetchResults(query);
    if (res)
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        std::cout <<row[0] << '\0';
        mysql_free_result(res);  // 释放查询结果
        return row[0];
    }
    else
    {
        mysql_free_result(res);  // 释放查询结果
        return NULL;
    }
}

bool userDB::sendFriendRequest(const string& sender_name, const string& receiver_name)
{
    std::string sender_id = serachUserId(sender_name);
    std::string receiver_id = serachUserId(receiver_name);
    
    string query = "INSERT INTO friend_requests (sender_id, receiver_id, status) VALUES (" +
    sender_id + ", " + receiver_id + ", 'pending')"; //插入语句
    string que = "SELECT COUNT(*) FROM friend_requests WHERE sender_id = " + sender_id + " AND receiver_id = " + receiver_id;

    //先判断表中是否存在
    res = fetchResults(que);
    if (!res) {
        return false;  // 如果查询失败，返回 false
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        // 获取 COUNT(*) 的结果值，假设第一个字段是 COUNT(*) 的结果
        int count = atoi(row[0]);  // 将字符串转换为整数
        mysql_free_result(res);  // 释放查询结果
        if(count>0)
        {
            std::cout << "表中已经存在" << sender_name << "添加" << receiver_name <<"的记录\n";
            return false;  // 如果返回值大于 0，说明记录存在
        }
    }
    else 
    {
        mysql_free_result(res);  // 释放查询结果
    }



    bool isInsert = executeQuery(query);  // 执行插入操作

    if(isInsert)
        std::cout << "用户:" << sender_name << "向用户:" << receiver_name << "申请添加为好友" << std::endl;
    return isInsert;
}

void userDB::insertFrienaRequestTo()
{
    std::string query = "SELECT sender_id, receiver_id, status FROM friend_requests";
    MYSQL_RES* res_1 = fetchResults(query);
    MYSQL_ROW row;
    int j = 0;
    while((row = mysql_fetch_row(res_1) ))
    {
        for(int i = 0 ; i < client.size()-1 ; i++)
        {
            int status = -1;
            std::string id = serachUserId(client[i]->userName);
            if(id.empty())
            {
                std::cout << "用户不存在\n";
                continue;
            }
            std::string sender = row[0];
            std::string receiver = row[1];
            std::string sta = row[2];
            std::string senderName = serachUserName(sender);
            std::string receiverName = serachUserName(receiver);
            if(id == sender || id == receiver)
            {
                if(sta == "pending")
                    status = 0;
                else if(sta == "accepted")
                    status = 1;
                else
                    status = 2;
                client[i]->returnAddfriend(senderName , receiverName , status);   
            }
        }
    }
    mysql_free_result(res_1);  // 释放查询结果
}


void userDB::changeFriendRequestState(const string& senderName , const string& receiveName , const int& state)
{
    std::string senderId = serachUserId(senderName);
    std::string receiverId = serachUserId(receiveName);
    if(senderId.empty() || receiverId.empty())
    {
        std::cout << "用户不存在\n";
        return;
    }
    std::string status = (state == 1) ? "accepted" : "rejected" ;
    std::string query = "update friend_requests set status = '" + status +"' where sender_id = " + senderId + " and receiver_id = " + receiverId;
    bool isInsert = executeQuery(query);  // 执行插入操作
    std::cout << "用户:" << senderName << "向用户:" << receiveName << "发送的好友申请是否被接受:" << status << '\n';
    usersDataBases.insertFrienaRequestTo();
    if(state == 1)
        addFriend(senderName , receiveName);
}


void userDB::addFriend(const string& friendName, const string& userName)
{
    std::string query = "UPDATE users SET friends = CASE WHEN friends IS NULL THEN '" 
    + friendName + "' WHEN FIND_IN_SET('" + friendName +
     "', friends) = 0 THEN CONCAT_WS(',', friends, '" + friendName +
      "') ELSE friends END WHERE username = '" + userName + "'";

    bool isInsert = executeQuery(query);
    query = "UPDATE users SET friends = CASE WHEN friends IS NULL THEN '" 
    + userName + "' WHEN FIND_IN_SET('" + userName +
     "', friends) = 0 THEN CONCAT_WS(',', friends, '" + userName +
      "') ELSE friends END WHERE username = '" + friendName + "'";
    isInsert = executeQuery(query);
    std::cout << friendName << "和" << userName << "成为好友" << '\n';
    for(int i = 0 ; i < client.size() ; i++)
    {
        if(client[i]->userName == friendName || client[i]->userName == userName)
            client[i]->sendFriendList();
    }
}

std::vector<std::string> userDB::getFriendList(const string& userName)
{
    std::string query = "select friends from users where username = '" + userName + "'" ;
    MYSQL_RES* res_1 = fetchResults(query);
    if (res_1 == nullptr) {
        //fetchResults 失败，返回一个空的vector。
        return std::vector<std::string>();
    }
    MYSQL_ROW row;
    row = mysql_fetch_row(res_1);
    if (row == NULL) {
        // 没有找到匹配的行，返回空向量
        mysql_free_result(res_1);
        return std::vector<std::string>();
    }
    std::string Friend = std::string(row[0]);
    std::vector<std::string> friendsList ;
    Friend.erase(std::remove(Friend.begin() , Friend.end() , '[') , Friend.end());
    Friend.erase(std::remove(Friend.begin() , Friend.end() , ']') , Friend.end());
    std::stringstream tokenStream(Friend);
    std::string token;
    while (std::getline(tokenStream, token, ',')) {
        if (!token.empty()) {
            friendsList.push_back(token);
            std::cout << friendsList.back() << ' ';
        }
    }
    mysql_free_result(res_1);
    return friendsList;

}

std::vector<std::unordered_map<int , std::string>> userDB::sendMessage(std::string username, std::vector<std::string>& friednName , std::vector<std::string>& messageTime)
{
    std::string query = "select sender_name , receiver_name , message_content , file_type ,sent_at from message_queue where sender_name = '" 
    + username + "' OR receiver_name = '" + username + "' ";
    std::vector<std::unordered_map<int , std::string>> messageHistory;

    MYSQL_RES* res_1 = fetchResults(query);
    std::cout << query;
    if (res_1) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res_1))) {
            std::string sender_name ;
            std::string receiver_name ;
            std::string message_content ;
            std::string file_type ;
            std::string time; 
            
            if (row[0]) sender_name =  row[0];
            if (row[1]) receiver_name = row[1];
            if (row[2]) message_content = row[2];
            if (row[3]) file_type = row[3];
            if (row[4]) time = row[4];

            std::unordered_map<int, std::string> message;
            int key = 0;

            if (sender_name == username) {
                friednName.push_back(receiver_name);
                messageTime.push_back(time);
                // 当前用户是发送者
                if (file_type == "text") {
                    key = 1; // 文本消息
                } else {
                    key = 3; // 文件消息
                }
            } else {
                friednName.push_back(sender_name);
                messageTime.push_back(time);
                // 当前用户是接收者
                if (file_type == "text") {
                    key = 2; // 文本消息
                } else {
                    key = 4; // 文件消息
                }
            }

            message[key] = message_content;
            //std::cout << key << ":" << message_content;
            messageHistory.push_back(message);
        }
        mysql_free_result(res_1); // 释放结果集
    }
    // for(std::unordered_map<int , std::string> mess : messageHistory)
    // {
    //     std::cout << mess[1] << mess[2] << '\n';
    // }
    return messageHistory;

}


void  userDB::addPictureOrFileMessage(std::string senderName , std::string receiverName , std::string fileOrPicture)
{
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[3];
    const char *query = "INSERT INTO message_queue (sender_name, receiver_name, message_content, file_type) VALUES (?, ?, ?, 'file')";

        // 创建预处理语句
        stmt = mysql_stmt_init(conn_);
        if (!stmt) {
            std::cerr << "mysql_stmt_init() 失败: " << mysql_error(conn_) << std::endl;
            return;
        }
        
        // 准备SQL语句
        if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
            std::cerr << "mysql_stmt_prepare() 失败: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            return;
        }
        
        // 清空并初始化绑定结构
        memset(bind, 0, sizeof(bind));
        
        // 绑定参数
        unsigned long sender_len = senderName.length();
        unsigned long receiver_len = receiverName.length();
        unsigned long message_len = fileOrPicture.length();
        
        // 设置第一个参数 - sender_name
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (void*)senderName.c_str();
        bind[0].buffer_length = sender_len;
        bind[0].length = &sender_len;
        bind[0].is_null = 0;
        
        // 设置第二个参数 - receiver_name
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (void*)receiverName.c_str();
        bind[1].buffer_length = receiver_len;
        bind[1].length = &receiver_len;
        bind[1].is_null = 0;
        
        // 设置第三个参数 - message_content
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (void*)fileOrPicture.c_str();
        bind[2].buffer_length = message_len;
        bind[2].length = &message_len;
        bind[2].is_null = 0;
        
        // 绑定参数到语句
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::cerr << "mysql_stmt_bind_param() 失败: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            return;
        }
        
        // 执行预处理语句
        if (mysql_stmt_execute(stmt) != 0) {
            std::cerr << "mysql_stmt_execute() 失败: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            return;
        }
        
        std::cout << "成功添加文件图片消息: 从 " << senderName << " 到 " << receiverName << std::endl;
        
        // 关闭语句
        mysql_stmt_close(stmt);
        
        // 通知接收者有新消息 
        for(auto& cli : client)
        {
            if(cli->userName == receiverName)
            {
                cli->sendMessageHistory();
            }
        }
}


void userDB::addTextMessage(std::string senderName, std::string receiverName , std::string textMessage)
{
    // std::string query = "INSERT INTO message_queue (sender_name, receiver_name, message_content, file_type) VALUES ( '" +
    // senderName + "' , '" + receiverName + "' , '" + textMessage + "' , 'text' )";
    // std::cout << "执行添加文本消息语句 :" << query << '\n';
    // bool is = executeQuery(query);
    
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[3];
    const char *query = "INSERT INTO message_queue (sender_name, receiver_name, message_content, file_type) VALUES (?, ?, ?, 'text')";
    
    // 创建预处理语句
    stmt = mysql_stmt_init(conn_);
    if (!stmt) {
        std::cerr << "mysql_stmt_init() 失败: " << mysql_error(conn_) << std::endl;
        return;
    }
    
    // 准备SQL语句
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        std::cerr << "mysql_stmt_prepare() 失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return;
    }
    
    // 清空并初始化绑定结构
    memset(bind, 0, sizeof(bind));
    
    // 绑定参数
    unsigned long sender_len = senderName.length();
    unsigned long receiver_len = receiverName.length();
    unsigned long message_len = textMessage.length();
    
    // 设置第一个参数 - sender_name
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)senderName.c_str();
    bind[0].buffer_length = sender_len;
    bind[0].length = &sender_len;
    bind[0].is_null = 0;
    
    // 设置第二个参数 - receiver_name
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)receiverName.c_str();
    bind[1].buffer_length = receiver_len;
    bind[1].length = &receiver_len;
    bind[1].is_null = 0;
    
    // 设置第三个参数 - message_content
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void*)textMessage.c_str();
    bind[2].buffer_length = message_len;
    bind[2].length = &message_len;
    bind[2].is_null = 0;
    
    // 绑定参数到语句
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        std::cerr << "mysql_stmt_bind_param() 失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return;
    }
    
    // 执行预处理语句
    if (mysql_stmt_execute(stmt) != 0) {
        std::cerr << "mysql_stmt_execute() 失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return;
    }
    
    std::cout << "成功添加文本消息: 从 " << senderName << " 到 " << receiverName << std::endl;
    
    // 关闭语句
    mysql_stmt_close(stmt);
    
    // 通知接收者有新消息 
    for(auto& cli : client)
    {
        if(cli->userName == receiverName)
        {
            cli->sendMessageHistory();
        }
    }
    
}

void userDB::addFileMessage(std::string senderName , std::string receiverName , std::string fileName)
{
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[3];
    const char *query = "INSERT INTO message_queue (sender_name, receiver_name, message_content, file_type) VALUES (?, ?, ?, 'file')";
    
    // 创建预处理语句
    stmt = mysql_stmt_init(conn_);
    if (!stmt) {
        std::cerr << "mysql_stmt_init() 失败: " << mysql_error(conn_) << std::endl;
        return;
    }
    
    // 准备SQL语句
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        std::cerr << "mysql_stmt_prepare() 失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return;
    }
    
    // 清空并初始化绑定结构
    memset(bind, 0, sizeof(bind));
    
    // 绑定参数
    unsigned long sender_len = senderName.length();
    unsigned long receiver_len = receiverName.length();
    unsigned long message_len = fileName.length();
    
    // 设置第一个参数 - sender_name
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)senderName.c_str();
    bind[0].buffer_length = sender_len;
    bind[0].length = &sender_len;
    bind[0].is_null = 0;
    
    // 设置第二个参数 - receiver_name
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)receiverName.c_str();
    bind[1].buffer_length = receiver_len;
    bind[1].length = &receiver_len;
    bind[1].is_null = 0;
    
    // 设置第三个参数 - message_content
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void*)fileName.c_str();
    bind[2].buffer_length = message_len;
    bind[2].length = &message_len;
    bind[2].is_null = 0;
    
    // 绑定参数到语句
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        std::cerr << "mysql_stmt_bind_param() 失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return;
    }
    
    // 执行预处理语句
    if (mysql_stmt_execute(stmt) != 0) {
        std::cerr << "mysql_stmt_execute() 失败: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return;
    }
    
    std::cout << "成功添加文件消息:" << fileName <<"  从 " << senderName << " 到 " << receiverName << std::endl;
    
    // 关闭语句
    mysql_stmt_close(stmt);
    
    // 通知接收者有新消息 
    for(auto& cli : client)
    {
        if(cli->userName == receiverName)
        {
            cli->sendMessageHistory();
        }
    }
}


