#include "chatClient.h"
#include "userMysql.h"
#include <map>
#include <filesystem>
#include <fstream>

std::vector<ChatClient*> client;
ChatClient::ChatClient()
{

}


ChatClient::~ChatClient()
{
    
}

void ChatClient::Session(std::shared_ptr<asio::ip::tcp::socket> socket)
{
    std::cout << "成功接入对话\n" ;
    try
    {
        asio::ip::tcp::endpoint remote_endpoint = socket->remote_endpoint();
        std::string client_ip = remote_endpoint.address().to_string();
        userIp = client_ip;
        clientSocket = socket;
        acceptMessage();
        
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
    }
    
}


void ChatClient::acceptMessage()
{
    std::cout << "正在等待" << userName << "的信息\n" ; 
    // if(header != nullptr)
    //     delete header;
    auto* header = new PacketHeader;
    asio::async_read(*clientSocket, asio::buffer(header, sizeof(PacketHeader)),
        [this,header](asio::error_code ec, std::size_t length) 
    {         
        std::cout << "包头" << header->message_type << '\n';              
         if (ec) 
        {
            // 其他错误
            try
            {
                throw asio::system_error(ec);
            }
            catch(const std::exception& e)
            {
                std::cerr << "System error: " << e.what() << ", Error code: "<<ec.value()  << std::endl;
                std::cout << "用户:"<<userName << "断开了连接" << std::endl;
                auto it = std::find(client.begin(), client.end(), this);
                client.erase(it);       
                delete this;
            }
            
            return;                   
        }
        else
        {
            //注册请求
            if(header->message_type == Register_REQUEST)
            {
                delete header;
                std::cout <<"注册请求\n";  
                request = new RegisterRequest;
                std::memset(request->username , '\0' , sizeof(request->username));
                std::memset(request->password , '\0' , sizeof(request->password));
                asio::async_read(*clientSocket,
                    asio::buffer(request, sizeof(RegisterRequest)),
                    [=](asio::error_code ec, size_t length) {
                        if (ec || length != sizeof(RegisterRequest)) {
                            std::cerr << "注册请求错误: " << ec.message() << "\n";
                            return;
                        }
                        std::cout << "用户:" << request->username << "密码:" << request->password <<" 尝试注册" << std::endl;
                        // 处理注册逻辑
                        
                        RegisterUser(request);
                    }
                );             
            }

            //登录请求，由于登录包与注册包类似，所以用同一个
            else if(header->message_type == LOGIN_REQUEST)
            {
                delete header;
                std::cout <<"登录请求\n";  
                request = new RegisterRequest;
                std::memset(request->username , '\0' , sizeof(request->username));
                std::memset(request->password , '\0' , sizeof(request->password));
                asio::async_read(*clientSocket,
                    asio::buffer(request, sizeof(RegisterRequest)),
                    [=](asio::error_code ec, size_t length) {
                        if (ec || length != sizeof(RegisterRequest)) {
                            std::cerr << "登录请求错误: " << ec.message() << "\n";
                            return;
                        }
                        std::cout << "用户:" << request->username << "密码:" << request->password << "尝试登录" << std::endl;
                        // 处理登录逻辑
                        
                        LoginUser(request);
                    }
                );
            }

            else if(header->message_type == SerarchUser_MESSAGE)
            {
                serachUser = new serarchUserRequest  ;
                std::memset(serachUser->username , '\0' ,sizeof(serachUser->username));
                delete header;
                asio::async_read(*clientSocket,
                    asio::buffer(serachUser, sizeof(serarchUserRequest)),
                    [&](asio::error_code ec, size_t length) {
                        if (ec || length != sizeof(serarchUserRequest)) {
                            std::cerr << "登录请求错误: " << ec.message() << "\n";
                            return;
                        }
                        std::cout << "正在搜索用户" << serachUser->username << "是否存在\n";

                        //发送包头
                        PacketHeaderServer header{SerarchUserResponse};
                        std::vector<char> header_buffer(sizeof(PacketHeaderServer));
                        std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeaderServer));
                        asio::async_write(*clientSocket,
                            asio::buffer(header_buffer, sizeof(PacketHeaderServer)),[&](asio::error_code ec, std::size_t length) {
                                if (ec) {
                                    std::cerr << "发送查询回应包错误: " << ec.message() << "\n";
                                }
                                std::cout << "成功发送好友查询回应包包头\n";
                                // 处理查询逻辑
                                serarchUserResponse respon;
                                respon.status_code = usersDataBases.userExists(serachUser->username);
                                std::vector<char> respon_buffer(sizeof(serarchUserResponse));
                                std::memcpy(respon_buffer.data() , &respon , sizeof(serarchUserResponse));
                                asio::async_write(*clientSocket,
                                    asio::buffer(respon_buffer, sizeof(serarchUserResponse)),[&](asio::error_code ec, std::size_t length) {
                                        if (ec) {
                                            std::cerr << "发送查询回应包错误: " << ec.message() << "\n";
                                        }

                                        std::cout << "成功发送好友查询回应包\n";
                                        acceptMessage();
                                    });
                                
                                delete  serachUser;
                              });                       
                    });
             }

             else if(header->message_type == AddFriendsRequest)
             {
                delete header;
                FrirendRequest = new addFrirendReauest  ;
                std::memset(FrirendRequest->ReceiverName , '\0' , sizeof(FrirendRequest->ReceiverName));
                std::memset(FrirendRequest->SenderName , '\0' , sizeof(FrirendRequest->SenderName));
                asio::async_read(*clientSocket,
                    asio::buffer(FrirendRequest, sizeof(addFrirendReauest)),
                    [&](asio::error_code ec, size_t length)
                    { 
                        std::cout << "用户:" << FrirendRequest->SenderName << "申请添加用户" << FrirendRequest->ReceiverName << "为好友\n";
                        bool isInsertAccept = usersDataBases.sendFriendRequest(FrirendRequest->SenderName ,  FrirendRequest->ReceiverName);

                        //发送包头
                        PacketHeaderServer header{AddFriendsResponse};
                        std::vector<char> header_buffer(sizeof(PacketHeaderServer));
                        std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeaderServer));
                        asio::async_write(*clientSocket,
                            asio::buffer(header_buffer, sizeof(PacketHeaderServer)),[=](asio::error_code ec, std::size_t length) {
                                if (ec) {
                                    std::cerr << "发送s申请回应包错误: " << ec.message() << "\n";
                                }
                                std::cout << "成功发送好友申请回应包包头\n";
                                // 处理查询逻辑
                                addFrirendResponse respon;
                                respon.state = isInsertAccept;
                                std::vector<char> respon_buffer(sizeof(addFrirendResponse));
                                std::memcpy(respon_buffer.data() , &respon , sizeof(addFrirendResponse));
                                asio::async_write(*clientSocket,
                                    asio::buffer(respon_buffer, sizeof(addFrirendResponse)),[&](asio::error_code ec, std::size_t length) {
                                        if (ec) {
                                            std::cerr << "发送查询回应包错误: " << ec.message() << "\n";
                                        }
                                        std::cout << "成功发送好友申请回应包\n";
                                        usersDataBases.insertFrienaRequestTo();
                                        acceptMessage();
                                       
                                    });
                              });  
                        
                    }
                );
             }
             else if(header->message_type == FriendAddRequest)
             {
                usersDataBases.insertFrienaRequestTo();
                delete header;
                acceptMessage();
             }
             else if(header->message_type == ResponeFriendRequest )
             {
                std::cout << "包头" << header->message_type << '\n';   
                std::cout << "回应好友请求";           
                SendFriendRequestToClient* FrirendRequest = new SendFriendRequestToClient ;
                FrirendRequest->status_code = -1;
                std::memset(FrirendRequest->ReceiverName , '\0' , 256);
                std::memset(FrirendRequest->SenderName , '\0' , 256);
                // asio::read(*server_socket,asio::buffer(FrirendRequest, sizeof(addFrirendReauest)));
                // qDebug() << "sender:" << FrirendRequest->SenderName << " receiver" << FrirendRequest->ReceiverName << "status" << FrirendRequest->status_code;
                asio::async_read(*clientSocket,
                                asio::buffer(FrirendRequest, header->length),
                                [& , FrirendRequest](asio::error_code ec, size_t length){
                                    if (ec) {
                                        std::cerr << "Async read error: " << ec.message() << std::endl;
                                        // 处理错误，例如关闭 socket 或重新发起读取
                                    } else {
                                        std::cout << "用户:" << FrirendRequest->SenderName << "向用户:" << FrirendRequest->ReceiverName << "发送的好友申请是否被接受:" << FrirendRequest->status_code << '\n';
                                        usersDataBases.changeFriendRequestState(FrirendRequest->SenderName , FrirendRequest->ReceiverName , FrirendRequest->status_code);
                                    }
                                    acceptMessage();
                                });
             }
             else if(header->message_type == TextMessageHeader)
             {
                 std::cout << "用户发送信息\n";
                 auto* textMsg = new TextMessagePacket;
                 memset(textMsg->ReceiverName, '\0', sizeof(textMsg->ReceiverName));
                 memset(textMsg->SenderName, '\0', sizeof(textMsg->SenderName));
                 memset(textMsg->TextMessage, '\0', sizeof(textMsg->TextMessage));
                 
                 asio::async_read(*clientSocket,
                                  asio::buffer(textMsg, header->length),
                                  [this, textMsg,header](asio::error_code ec, size_t length) {
                     
                     std::cout << "用户: " << textMsg->SenderName 
                               << " 发送消息: " << textMsg->TextMessage 
                               << " 给用户: " << textMsg->ReceiverName << '\n';
                     usersDataBases.addTextMessage(textMsg->SenderName, 
                                                  textMsg->ReceiverName, 
                                                  textMsg->TextMessage);
                     std::unordered_map<int, std::string> tempMap;
                     tempMap[1] = textMsg->TextMessage;
                     message.push_back(tempMap);
                     delete textMsg;
                     delete header;
                     acceptMessage(); // 继续接收下一条消息
                 });
             }
             else if(header->message_type == PictureMessageHeader)
             {
                 std::cout << "接收到带有图片的消息\n";
                 auto* pictureMessagePacket_0 = new pictureMessagePacket_first;
                 std::string* pictureMessage = new std::string ;
                 // 将 packets 移动到外部作用域
                 std::memset(pictureMessagePacket_0->ReceiverName, 0, sizeof(pictureMessagePacket_0->ReceiverName));
                 std::memset(pictureMessagePacket_0->SenderName, 0, sizeof(pictureMessagePacket_0->SenderName));
                 std::memset(pictureMessagePacket_0->messageTime, 0, sizeof(pictureMessagePacket_0->messageTime));
             
                 asio::async_read(*clientSocket,
                     asio::buffer(pictureMessagePacket_0, sizeof(pictureMessagePacket_first)),
                     [this, pictureMessagePacket_0 , pictureMessage](asio::error_code ec, size_t length) { // 使用引用捕获 packets
                         std::cout << pictureMessagePacket_0->SenderName << "给" << pictureMessagePacket_0->ReceiverName << "发送带有图片的消息\n";
                         std::cout << "一共需要接受" << pictureMessagePacket_0->packetSize << "个包\n";
                         acceptPictureMessage(pictureMessage , pictureMessagePacket_0->ReceiverName);
                     });
             }
             else if(header->message_type == FileMessageHeader)
             {
                // 2. 接收接收者名字
                std::cout << "接受到文件消息: ";
                int receiverNameLength;
                asio::read(*clientSocket, asio::buffer(&receiverNameLength, sizeof(int)));
                std::vector<char> receiverNameBuffer(receiverNameLength);
                asio::read(*clientSocket, asio::buffer(receiverNameBuffer.data(), receiverNameLength));
                std::string receiverName(receiverNameBuffer.begin(), receiverNameBuffer.end());
                std::cout << "接收者名字: " << receiverName << std::endl;

                //3.接受文件名
                int fileNameLength;
                asio::read(*clientSocket, asio::buffer(&fileNameLength, sizeof(int)));
                std::vector<char> fileNameBuffer(fileNameLength);
                asio::read(*clientSocket, asio::buffer(fileNameBuffer.data(), fileNameLength));
                std::string fileName(fileNameBuffer.begin(), fileNameBuffer.end());
                std::cout << "文件名字: " << fileName << std::endl;
                fileReceiver[fileName] = receiverName;


                
                //5.获取文件大小
                long long fileSize;
                asio::read(*clientSocket, asio::buffer(&fileSize, sizeof(long long)));
                std::cout << "文件大小：" << fileSize;
                fileSizeAll[fileName] = fileSize;

                //4.接受文件
                mkdir(("/home/wumo/c++/chat_server/chat_server/build/userFile/" + userName).c_str() , S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                std::ofstream* outputFile = new std::ofstream("/home/wumo/c++/chat_server/chat_server/build/userFile/" 
                    + userName + '/' + fileName , std::ios::binary);
                outFile[fileName] = outputFile;
                int i = 1;
                isAccept = false;

                //acceptFileMessage(fileSize , outputFile , fileName);
           
             }
             else if(header->message_type == FileSocket)
             {
                char name[256] ;
                std::memset(name , '\0' , 256);
                asio::read(*clientSocket , asio::buffer(name , 256));
                std::cout << '\n' << name <<"的文件socket接入\n";
                int length;
                asio::read(*clientSocket , asio::buffer(&length , sizeof(int)));
                std::cout << "文件长度" << length << '\n';
                char fileName[length + 1];
                std::memset(fileName , '\0' , length + 1);
                asio::read(*clientSocket , asio::buffer(fileName , length));
                std::cout << "文件名" << fileName << '\n';

                // 将文件socket接入用户
                for(auto& cli : client)
                {
                    if(cli->userName == name)
                    {
                        std::cout << '\n' << name <<"的文件socket接入\n";
                        cli->fileSocekt[fileName] = clientSocket;
                        cli->acceptFileMessage(cli->fileSizeAll[fileName] , cli->outFile[fileName] , fileName);
                        auto it = std::find(client.begin(), client.end(), this);
                        client.erase(it);       
                        delete this;
                    }
                }
             }
             else if(header->message_type == ReceiveFileMessageHeader)
             {
                // 2. 接收发送者者名字
                std::cout << "接受到申请下载文件消息: ";
                int senderNameLength;
                asio::read(*clientSocket, asio::buffer(&senderNameLength, sizeof(int)));
                std::vector<char> senderNameBuffer(senderNameLength);
                asio::read(*clientSocket, asio::buffer(senderNameBuffer.data(), senderNameLength));
                std::string senderName(senderNameBuffer.begin(), senderNameBuffer.end());
                std::cout << "发送者名字: " << senderName << std::endl;
                //3.接受文件名
                int fileNameLength;
                asio::read(*clientSocket, asio::buffer(&fileNameLength, sizeof(int)));
                std::vector<char> fileNameBuffer(fileNameLength);
                asio::read(*clientSocket, asio::buffer(fileNameBuffer.data(), fileNameLength));
                std::string fileName(fileNameBuffer.begin(), fileNameBuffer.end());
                std::cout << "文件名字: " << fileName << std::endl;
                //4.接受发送者名字
                int receiverNameLength;
                asio::read(*clientSocket, asio::buffer(&receiverNameLength, sizeof(int)));
                std::vector<char> receiverNameBuffer(receiverNameLength);
                asio::read(*clientSocket, asio::buffer(receiverNameBuffer.data(), receiverNameLength));
                std::string receiverName(receiverNameBuffer.begin(), receiverNameBuffer.end());
                std::cout << "接收者名字: " << receiverName << std::endl;

                std::ifstream * inputFile = new std::ifstream ("/home/wumo/c++/chat_server/chat_server/build/userFile/" 
                    + senderName + '/' + fileName , std::ios::binary | std::ios::ate);

                
                inputFile->seekg(0, std::ios::end);
                long long fileSize = inputFile->tellg();
                inputFile->seekg(0, std::ios::beg);

                std::cout << "文件大小：" << fileSize << '\n';
                asio::write(*clientSocket , asio::buffer(&fileSize , sizeof(long long)));
                
                sendFile(fileSize , inputFile , fileName);

             }

             else if(header->message_type == voiceChatRequestHeader)
             {
                delete header;
                FrirendRequest = new addFrirendReauest  ;
                std::memset(FrirendRequest->ReceiverName , '\0' , sizeof(FrirendRequest->ReceiverName));
                std::memset(FrirendRequest->SenderName , '\0' , sizeof(FrirendRequest->SenderName));
                asio::async_read(*clientSocket,
                    asio::buffer(FrirendRequest, sizeof(addFrirendReauest)),
                    [&](asio::error_code ec, size_t length)
                    { 
                        std::cout << "用户:" << FrirendRequest->SenderName << "向" << FrirendRequest->ReceiverName << "发起通讯请求\n"; 
                        for(auto& cli : client)
                        {
                            if(cli->userName == FrirendRequest->ReceiverName)
                            {
                                cli->sendVoiceChatRequest(FrirendRequest->SenderName , FrirendRequest->ReceiverName);
                            }
                        }
                        for(auto& cli : client)
                        {
                            if(cli->audioChatCurrentName == userName)
                            {
                                cli->audioChatName = (FrirendRequest->ReceiverName == audioChatCurrentName ? FrirendRequest->SenderName : FrirendRequest->ReceiverName);
                            }
                        }
                        acceptMessage();
                    }
                );
             }

             else if(header->message_type == responeVoiceChatHeader)
             {
                SendFriendRequestToClient request;
                asio::read(*clientSocket , asio::buffer(&request , sizeof(SendFriendRequestToClient)));
                std::cout << request.SenderName << " " << request.ReceiverName << " " << request.status_code << '\n';
                for(auto& cli : client)
                {
                    if(cli->userName ==  (request.SenderName == userName ? request.ReceiverName : request.SenderName) )
                    {
                        cli->responeVoice((request.status_code == 1 ? true : false));
                        break;
                    }
                }
                for(auto& cli : client)
                {
                    if(cli->audioChatCurrentName == userName)
                    {
                        cli->audioChatName = (request.SenderName == audioChatCurrentName ? request.ReceiverName : request.SenderName);
                    }
                }
                acceptMessage();
             }
             else if(header->message_type == transAudioToServer)
             {
                int length ;
                asio::read(*clientSocket , asio::buffer(&length , sizeof(int)));
                char receiveDataName[length + 1];
                memset(receiveDataName , '\0' , length + 1);
                asio::read(*clientSocket , asio::buffer(receiveDataName , length));
                std::string ReceiveName = receiveDataName;
                std::cout << "要将数据发送给" << ReceiveName << '\n';

                asio::read(*clientSocket , asio::buffer(&length , sizeof(int)));
                memset(receiveDataName , '\0' , length + 1);
                asio::read(*clientSocket , asio::buffer(receiveDataName , length));
                audioChatCurrentName = receiveDataName;
                audioChatName = ReceiveName;

                acceptAudioMessage();
             }

             else 
             {
                delete header;
                acceptMessage();
             }
        }
    });
}

void ChatClient::acceptAudioMessage()
{
    int* size = new int;
    asio::async_read(*clientSocket , asio::buffer(size , sizeof(int)) , [this , size](asio::error_code ec, size_t length){
        if(ec)
            return;
        if(*size == 0)
        {
            std::cout << "结束音频传输";
            for(auto& tmp : client)
            {
                if(tmp->audioChatCurrentName == audioChatName)
                {
                    asio::write(*(tmp->clientSocket) , asio::buffer(size , sizeof(int)));
                }
            }
            acceptAudioMessage();
            return;   
        }
        if(*size == 1)
        {
            std::cout << "结束音频传输";
            acceptAudioMessage();   
            return;
        }
        char* data = new char[(*size) + 1];
        memset(data , '\0' , *size + 1);
        asio::async_read(*clientSocket , asio::buffer(data , *size) , [this , data  , size](asio::error_code ec, size_t length){
            if(ec)
                return;
            std::cout << "接收到：" <<audioChatCurrentName << "发给" << audioChatName << "的音频：" << *size << "音频数据\n";
            for(auto& tmp : client)
            {
                if(tmp->audioChatCurrentName == audioChatName)
                {
                    asio::write(*(tmp->clientSocket) , asio::buffer(size , sizeof(int)));
                    asio::write(*(tmp->clientSocket) , asio::buffer(data , *size));
                }
            }
            delete size;
            acceptAudioMessage();
            
        });
    });
}

void ChatClient::responeVoice(bool status)
{
    bool is = status;
    PacketHeaderServer FriendListHeader{voiceChatRespone};
    std::vector<char> header_buffer(sizeof(PacketHeaderServer));
    std::memcpy(header_buffer.data() , &FriendListHeader ,sizeof(PacketHeaderServer));
    asio::write(*clientSocket,asio::buffer(header_buffer, sizeof(PacketHeaderServer)));

    asio::write(*clientSocket,asio::buffer(&is, sizeof(bool)));

}

void ChatClient::sendVoiceChatRequest(std::string SenderName , std::string ReceiverName)
{
    //发送包头
    PacketHeaderServer AddfriendHeader{acceptChatRequestHeader};
    std::vector<char> header_buffer(sizeof(PacketHeaderServer));
    std::memcpy(header_buffer.data() , &AddfriendHeader ,sizeof(PacketHeaderServer));
    asio::write(*clientSocket,asio::buffer(header_buffer, sizeof(PacketHeaderServer)));
    // 处理查询逻辑
    addFrirendReauest request;
    std::memset(request.ReceiverName , '\0' , sizeof(request.ReceiverName));
    std::memset(request.SenderName , '\0' , sizeof(request.SenderName));
    strncpy(request.SenderName , SenderName.c_str() , sizeof(request.SenderName) - 1);
    strncpy(request.ReceiverName , ReceiverName.c_str() , sizeof(request.ReceiverName) - 1);
    std::vector<char> request_buffer(sizeof(addFrirendReauest));
    memset(request_buffer.data(),'\0',sizeof(addFrirendReauest));
    std::memcpy(request_buffer.data() , &request ,sizeof(addFrirendReauest));
    asio::write(*clientSocket,asio::buffer(request_buffer, sizeof(addFrirendReauest)));
}


//发送文件
void ChatClient::sendFile(long long remainingSize , std::ifstream * inputFile  , std::string fileName)
{
    const size_t chunkSize = 65535;
    char* data = new char[chunkSize];
    
    size_t readLength = std::min(remainingSize, static_cast<long long>(chunkSize));
    //std::memset(data , '\0' , 65535);

    if(!inputFile->read(data, readLength)) {
        std::cerr << "文件读取错误" << std::endl;
        delete inputFile;
        return;
    }
    
    asio::async_write(*clientSocket , asio::buffer(data, readLength),
        [this, remainingSize, inputFile, data, readLength , fileName](asio::error_code ec, size_t length) {
            if (ec) {
                // 处理错误
                delete[] data;
                return;
            }
            
            std::cout << " \n还剩下 " << remainingSize << "bit"  << std::endl;
            
            delete[] data; // 正确释放
            
            if (readLength == chunkSize) {
                // 递归读取下一个包
                sendFile(remainingSize - chunkSize, inputFile , fileName);
            } else {
                // 全部发送完成后再关闭
                inputFile->close();
                auto it = std::find(client.begin(), client.end(), this);
                client.erase(it);
                delete this;       
                delete inputFile;
            }
        });

}



//接受文件数据包
void ChatClient::acceptFileMessage(long long remainingSize , std::ofstream* outputFile , std::string fileName )
{
    if(!isAccept)
    {
        acceptMessage();
        isAccept = true;
    }
    const size_t chunkSize = 65535;
    char* data = new char[chunkSize];
    
    size_t readLength = std::min(remainingSize, static_cast<long long>(chunkSize));
    
    asio::async_read(*(fileSocekt[fileName]), asio::buffer(data, readLength),
        [this, remainingSize, outputFile, data, readLength , fileName](asio::error_code ec, size_t length) {
            if (ec) {
                // 处理错误
                delete[] data;
                return;
            }
            
            // 写入文件
            outputFile->write(data, readLength);
            
            //std::cout.write(data, length);
            std::cout << " \n还剩下 " << remainingSize << "bit"  << std::endl;
            
            delete[] data; // 正确释放
            
            if (readLength == chunkSize) {
                // 递归读取下一个包
                acceptFileMessage(remainingSize - chunkSize, outputFile , fileName);
            } else {
                // 全部读取完成后再关闭
                usersDataBases.addFileMessage(userName, 
                    fileReceiver[fileName], 
                    fileName);
                std::unordered_map<int, std::string> tempMap;
                tempMap[3] = fileName;
                message.push_back(tempMap);
                fileReceiver[fileName].erase();
                outputFile->close();
                delete outputFile;
            }
        });

}

void ChatClient::acceptPictureMessage(std::string* pictureMessage , std::string receiverName)
{
    pictureMessagePacket* picture = new pictureMessagePacket;
    std::memset(picture->TextMessage , 0 , sizeof(pictureMessagePacket));
    asio::async_read(*clientSocket , 
        asio::buffer(picture , sizeof(pictureMessagePacket)) , 
        [&,picture,pictureMessage , receiverName](asio::error_code ec, size_t length){
            *pictureMessage += picture->TextMessage;
            if(picture->isLastPacket)
            {
                usersDataBases.addPictureOrFileMessage(userName, receiverName, *pictureMessage);
                std::unordered_map<int, std::string> tempMap;
                tempMap[3] = *pictureMessage;
                message.push_back(tempMap);
                delete pictureMessage;
                delete picture;
                acceptMessage();
            }
            else
            {
                delete picture;
                acceptPictureMessage(pictureMessage , receiverName);
            }
        });   
}


void ChatClient::RegisterUser(RegisterRequest* request)
{
    // 插入数据库
    bool isInsert = usersDataBases.insertUser(request->username, request->password);
    RegisterResponse respon;
    if(isInsert)
        respon.status_code = 0;
    else
        respon.status_code = 1;

    std::vector<char> respon_buffer(sizeof(RegisterResponse));
    std::memcpy(respon_buffer.data() , &respon , sizeof(RegisterResponse));
    asio::async_write(*clientSocket,
        asio::buffer(respon_buffer, sizeof(RegisterResponse)),[this](asio::error_code ec, std::size_t length) {
            if (ec) {
                std::cerr << "发送注册回应包错误: " << ec.message() << "\n";
            }

            acceptMessage();
          });
    std::cout << "创建用户返回码" << respon.status_code << std::endl;
    delete request;
}


void ChatClient::LoginUser(RegisterRequest* request)
{
    int isLogin = usersDataBases.checkLogin(request->username, request->password);
    if(isLogin == 0)
    {
        this->userName = request->username;
        this->userPassword = request->password;
    }
    LoginResponse respon;
    respon.status_code = isLogin;
    std::vector<char> respon_buffer(sizeof(LoginResponse));
    std::memcpy(respon_buffer.data() , &respon , sizeof(LoginResponse));
    asio::async_write(*clientSocket,
        asio::buffer(respon_buffer, sizeof(LoginResponse)),[this,isLogin](asio::error_code ec, std::size_t length) {
            if (ec) {
                std::cerr << "发送登录回应包错误: " << ec.message() << "\n";
            }

            if(isLogin == 0)
                {
                    sendFriendList();
                }
            acceptMessage();
            sendMessageHistory();
          });
    std::cout << "登录用户返回码" << respon.status_code << std::endl;
    
    delete request;
}

void ChatClient::returnAddfriend(std::string SenderName , std::string ReceiverName , uint16_t status_code)
{
        //发送包头
        PacketHeaderServer AddfriendHeader{ReturnAddReiend};
        std::vector<char> header_buffer(sizeof(PacketHeaderServer));
        std::memcpy(header_buffer.data() , &AddfriendHeader ,sizeof(PacketHeaderServer));
        asio::write(*clientSocket,asio::buffer(header_buffer, sizeof(PacketHeaderServer)));
        std::cout << "成功发送好友申请列表包包头:" << AddfriendHeader .message_type <<"\n";
        // 处理查询逻辑
        SendFriendRequestToClient request;
        std::memset(request.ReceiverName , '\0' , sizeof(request.ReceiverName));
        std::memset(request.SenderName , '\0' , sizeof(request.SenderName));
        strncpy(request.SenderName , SenderName.c_str() , sizeof(request.SenderName) - 1);
        strncpy(request.ReceiverName , ReceiverName.c_str() , sizeof(request.ReceiverName) - 1);
        request.status_code = status_code;
        std::vector<char> request_buffer(sizeof(SendFriendRequestToClient));
        memset(request_buffer.data(),'\0',sizeof(SendFriendRequestToClient));
        std::memcpy(request_buffer.data() , &request ,sizeof(SendFriendRequestToClient));
        asio::write(*clientSocket,asio::buffer(request_buffer, sizeof(SendFriendRequestToClient)));
        std::cout << "成功发送好友申请列表\n";
}


void ChatClient::sendFriendList()
{
    friendsList = usersDataBases.getFriendList(userName);
    std::vector<char> request_buffer;
    for (const auto& name : friendsList) {
        std::cout << name;
        request_buffer.insert(request_buffer.end(), name.begin(), name.end());
        request_buffer.push_back('\0'); // 使用 '\0' 分隔每个字符串
    }
    size_t totalLength = request_buffer.size();
    PacketHeaderServer FriendListHeader{FriendListsToClient};
    FriendListHeader.length = totalLength;
    std::vector<char> header_buffer(sizeof(PacketHeaderServer));
    std::memcpy(header_buffer.data() , &FriendListHeader ,sizeof(PacketHeaderServer));
    asio::write(*clientSocket,asio::buffer(header_buffer, sizeof(PacketHeaderServer)));
    std::cout << "成功发送好友列表包包头:" << FriendListHeader.message_type <<"\n";   
    asio::write(*clientSocket,asio::buffer(request_buffer , totalLength));
    std::cout << "成功发送好友列表"  <<"\n";  

}

void ChatClient::sendMessageHistory()
{
    std::vector<std::unordered_map<int , std::string>> tmp;
    std::vector<std::string> friednName;
    std::vector<std::string> messageTime;
    tmp = usersDataBases.sendMessage(userName , friednName , messageTime);
    std::cout << "tmp的个数为" << tmp.size() << '\n';
    std::cout << "friednName的个数为" << friednName.size() << '\n';

    int index = tmp.size() - message.size();
    if(index == 0)
        return;    
    for(int i = message.size() ; i < tmp.size() ; i++)
    {
        message = tmp;
        //文本消息且当前用户为发送者
        if(message[i].count(1) > 0 )
        {
            std::cout << "文本消息且当前用户为发送者\n";
            PacketHeader MessageHistoryHeader{TextMessageHistoryToClient};
            std::vector<char> header_buffer(sizeof(PacketHeader));
            int index_tmp = i;
            this->testMessageToClient = new TextMessagePacket;
            std::memset(testMessageToClient->ReceiverName , '\0' , sizeof(testMessageToClient->ReceiverName));
            std::memset(testMessageToClient->SenderName , '\0' , sizeof(testMessageToClient->SenderName));
            std::memset(testMessageToClient->TextMessage , '\0' , sizeof(testMessageToClient->TextMessage));
            std::memset(testMessageToClient->messageTime , '\0' , sizeof(testMessageToClient->messageTime));
            strncpy(testMessageToClient->ReceiverName , friednName[index_tmp].c_str() , sizeof(testMessageToClient->ReceiverName) - 1);
            strncpy(testMessageToClient->SenderName, userName.c_str() , sizeof(testMessageToClient->SenderName) - 1);
            strncpy(testMessageToClient->TextMessage, message[index_tmp][1].c_str() , sizeof(testMessageToClient->TextMessage) - 1);            
            strncpy(testMessageToClient->messageTime, messageTime[index_tmp].c_str() , sizeof(testMessageToClient->messageTime) - 1);            
            MessageHistoryHeader.length = sizeof(*testMessageToClient);
            std::memcpy(header_buffer.data() , &MessageHistoryHeader ,sizeof(PacketHeader));
            asio::write(*clientSocket,asio::buffer(header_buffer, sizeof(PacketHeader)));
            std::cout << "发送聊天记录包头\n";
            std::vector<char> request_buffer(sizeof(TextMessagePacket));
            std::memcpy(request_buffer.data() , this->testMessageToClient ,sizeof(TextMessagePacket));
            asio::write(*clientSocket , asio::buffer(request_buffer, MessageHistoryHeader.length));
            std::cout << "发送聊天记录\n";
        }
        //文本消息且当前用户为接受者
        if( message[i].count(2) > 0)
        {
            std::cout << "文本消息且当前用户为接受者\n";
            PacketHeader MessageHistoryHeader{TextMessageHistoryToClient};
            std::vector<char> header_buffer(sizeof(PacketHeader));
            std::memcpy(header_buffer.data() , &MessageHistoryHeader ,sizeof(PacketHeader));
            int index_tmp = i;
            this->testMessageToClient = new TextMessagePacket;
            std::memset(testMessageToClient->ReceiverName , '\0' , sizeof(testMessageToClient->ReceiverName));
            std::memset(testMessageToClient->SenderName , '\0' , sizeof(testMessageToClient->SenderName));
            std::memset(testMessageToClient->TextMessage , '\0' , sizeof(testMessageToClient->TextMessage));
            std::memset(testMessageToClient->messageTime , '\0' , sizeof(testMessageToClient->messageTime));
            strncpy(testMessageToClient->ReceiverName , userName.c_str() , sizeof(testMessageToClient->ReceiverName) - 1);
            strncpy(testMessageToClient->SenderName, friednName[index_tmp].c_str() , sizeof(testMessageToClient->SenderName) - 1);
            strncpy(testMessageToClient->TextMessage, message[index_tmp][2].c_str() , sizeof(testMessageToClient->TextMessage) - 1);            
            strncpy(testMessageToClient->messageTime, messageTime[index_tmp].c_str() , sizeof(testMessageToClient->messageTime) - 1);            
            MessageHistoryHeader.length = sizeof(*testMessageToClient);
            std::memcpy(header_buffer.data() , &MessageHistoryHeader ,sizeof(PacketHeader));
            asio::write(*clientSocket,asio::buffer(header_buffer, sizeof(PacketHeader)));
            std::cout << "发送聊天记录包头\n";
            std::vector<char> request_buffer(sizeof(TextMessagePacket));
            std::memcpy(request_buffer.data() , this->testMessageToClient ,sizeof(TextMessagePacket));
            asio::write(*clientSocket , asio::buffer(request_buffer, MessageHistoryHeader.length));
            std::cout << "发送聊天记录\n";
        }
        //图片文件消息且当前用户为发送者
        if( message[i].count(3) > 0)
        {
            if(message[i][3].length() < 1000)
            {
                std::string filePath = "/home/wumo/c++/chat_server/chat_server/build/userFile/" +
                userName + '/' + message[i][3];
                std::ifstream file(filePath, std::ios::binary | std::ios::ate); // 以二进制模式打开，指针定位到文件末尾
                //存储的是文件
                if(file.is_open())
                {


                    //由于要发送的包与文本消息包类似，故这里使用文本消息包
                    TextMessagePacket fileMessage;
                    std::memset(fileMessage.ReceiverName , '\0' , sizeof(fileMessage.ReceiverName));
                    std::memset(fileMessage.SenderName , '\0' , sizeof(fileMessage.SenderName));
                    std::memset(fileMessage.TextMessage , '\0' , sizeof(fileMessage.TextMessage));
                    std::memset(fileMessage.messageTime , '\0' , sizeof(fileMessage.messageTime));
                    strncpy(fileMessage.ReceiverName , friednName[i].c_str()  , sizeof(fileMessage.ReceiverName) - 1);
                    strncpy(fileMessage.SenderName, userName.c_str(), sizeof(fileMessage.SenderName) - 1);
                    strncpy(fileMessage.TextMessage, message[i][3].c_str() , sizeof(fileMessage.TextMessage) - 1);            
                    strncpy(fileMessage.messageTime, messageTime[i].c_str() , sizeof(fileMessage.messageTime) - 1);            



                    PacketHeader header{fileMessageHeaderToClient};
                    std::vector<char> header_buffer(sizeof(PacketHeader));
                    header.length = sizeof(fileMessage);
                    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
                    std::vector<char> request_buffer(sizeof(TextMessagePacket));
                    std::memcpy(request_buffer.data() , &fileMessage ,sizeof(TextMessagePacket));
                    asio::write(*clientSocket , asio::buffer(header_buffer , sizeof(PacketHeader)));
                    asio::write(*clientSocket , asio::buffer(request_buffer, header.length));
                    std::cout << "文件消息且当前用户为发送者,文件为:" << filePath << "\n";

                    long long fileSize = file.tellg();
                    std::cout << "文件大小：" << fileSize;
                    asio::write(*clientSocket, asio::buffer(&fileSize, sizeof(long long)));
                    continue;
                }
            }
            //存储的是图片
            
            
                std::cout << "图片消息且当前用户为发送者\n";
                PacketHeader header{PictureMessageHeader};
                std::vector<char> header_buffer(sizeof(PacketHeader));
                std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
                asio::write(*clientSocket , asio::buffer(header_buffer , sizeof(PacketHeader)));

                //计算一共发送多少个包,并存储进一个数组
                std::vector<pictureMessagePacket> packets;
                const int packetSize = 2046;  // 保留 2 字节给 null 终止符，或者用于其他控制信息
                int messageLength = message[i][3].length();
                int offset = 0;
                while (offset < messageLength) {
                    pictureMessagePacket packet;
                    std::memset(packet.TextMessage, 0, sizeof(packet.TextMessage)); // 使用 packet.TextMessage，更清晰
                    // 计算当前包的大小
                    int currentPacketSize = std::min(packetSize, messageLength - offset);
                    // 从 std::string 中提取子字符串
                    std::string subString = message[i][3].substr(offset, currentPacketSize);
                    // 复制子字符串到 packet 中
                    std::memcpy(packet.TextMessage, subString.c_str(), subString.length());
                    packet.TextMessage[subString.length()] = '\0'; // 显式添加 null 终止符，确保 C 风格字符串的正确性
                    packet.isLastPacket = (offset + packetSize >= messageLength);
                    packets.push_back(packet);
                    offset += packetSize;
                }

                //发送第一个包，告知发送者和接受者的身份信息
                pictureMessagePacket_first  pictureMessagePacket_0;
                std::memset(pictureMessagePacket_0.ReceiverName , 0 , sizeof(pictureMessagePacket_0.ReceiverName));
                std::memset(pictureMessagePacket_0.SenderName , 0 , sizeof(pictureMessagePacket_0.SenderName));
                std::memset(pictureMessagePacket_0.messageTime , 0 , sizeof(pictureMessagePacket_0.messageTime));
                strncpy(pictureMessagePacket_0.ReceiverName , friednName[i].c_str() , sizeof(pictureMessagePacket_0.ReceiverName));
                strncpy(pictureMessagePacket_0.SenderName , userName.c_str() , sizeof(pictureMessagePacket_0.SenderName));
                strncpy(pictureMessagePacket_0.messageTime, messageTime[i].c_str() , sizeof(pictureMessagePacket_0.messageTime) - 1);            
                pictureMessagePacket_0.packetSize = packets.size();
                std::cout << testMessageToClient->messageTime;
                std::vector<char> pictureMessagePacket_0_buffer(sizeof(pictureMessagePacket_first));
                memset(pictureMessagePacket_0_buffer.data(),0,sizeof(pictureMessagePacket_first));
                std::memcpy(pictureMessagePacket_0_buffer.data() , &pictureMessagePacket_0 ,sizeof(pictureMessagePacket_first));
                asio::write(*clientSocket , asio::buffer(pictureMessagePacket_0_buffer , sizeof(pictureMessagePacket_first)));

                //开始发送图片消息，每个包发送2046字节的包和1字节的标注是否是最后一个包

                for(int i = 0 ; i < packets.size() ; i++)
                {
                    std::vector<char> pictureMessagePacket_ll(sizeof(pictureMessagePacket));
                    memset(pictureMessagePacket_ll.data() , '\0' , sizeof(pictureMessagePacket));
                    std::memcpy(pictureMessagePacket_ll.data() , &packets[i] ,sizeof(pictureMessagePacket));
                    asio::async_write(*clientSocket,
                                    asio::buffer(pictureMessagePacket_ll , sizeof(pictureMessagePacket)) ,
                                    [& , packets , i](asio::error_code ec, std::size_t length){
                                        //std::cout << "发送第" << i << "个包，包大小:" << sizeof(pictureMessagePacket);
                                    });
                }
                       
        }


        //图片文件消息且当前用户为接受者
        if( message[i].count(4) > 0)
        {
            if(message[i][4].length() < 1000)
            {
                std::string filePath = "/home/wumo/c++/chat_server/chat_server/build/userFile/" +
                                        friednName[i] + '/' + message[i][4];
                std::ifstream file(filePath, std::ios::binary | std::ios::ate); // 以二进制模式打开，指针定位到文件末尾
                if(file.is_open())
                {

                    //由于要发送的包与文本消息包类似，故这里使用文本消息包
                    TextMessagePacket fileMessage;
                    std::memset(fileMessage.ReceiverName , '\0' , sizeof(fileMessage.ReceiverName));
                    std::memset(fileMessage.SenderName , '\0' , sizeof(fileMessage.SenderName));
                    std::memset(fileMessage.TextMessage , '\0' , sizeof(fileMessage.TextMessage));
                    std::memset(fileMessage.messageTime , '\0' , sizeof(fileMessage.messageTime));
                    strncpy(fileMessage.ReceiverName , userName.c_str()  , sizeof(fileMessage.ReceiverName) - 1);
                    strncpy(fileMessage.SenderName, friednName[i].c_str() , sizeof(fileMessage.SenderName) - 1);
                    strncpy(fileMessage.TextMessage, message[i][4].c_str() , sizeof(fileMessage.TextMessage) - 1);            
                    strncpy(fileMessage.messageTime, messageTime[i].c_str() , sizeof(fileMessage.messageTime) - 1);            



                    PacketHeader header{fileMessageHeaderToClient};
                    std::vector<char> header_buffer(sizeof(PacketHeader));
                    header.length = sizeof(fileMessage);
                    std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
                    std::vector<char> request_buffer(sizeof(TextMessagePacket));
                    std::memcpy(request_buffer.data() , &fileMessage ,sizeof(TextMessagePacket));
                    asio::write(*clientSocket , asio::buffer(header_buffer , sizeof(PacketHeader)));
                    asio::write(*clientSocket , asio::buffer(request_buffer, header.length));
                    std::cout << "文件消息且当前用户为接受者,文件为:" << filePath << "\n";

                    long long fileSize = file.tellg();
                    std::cout << "文件大小：" << fileSize;
                    asio::write(*clientSocket, asio::buffer(&fileSize, sizeof(long long))); // 发送长度
                    continue;
                }
            }
            //存储的是图片
                std::cout << "文件图片消息且当前用户为发送者\n";
                PacketHeader header{PictureMessageHeader};
                std::vector<char> header_buffer(sizeof(PacketHeader));
                std::memcpy(header_buffer.data() , &header ,sizeof(PacketHeader));
                asio::write(*clientSocket , asio::buffer(header_buffer , sizeof(PacketHeader)));

                //计算一共发送多少个包,并存储进一个数组
                std::vector<pictureMessagePacket> packets;
                const int packetSize = 2046;  // 保留 2 字节给 null 终止符，或者用于其他控制信息
                int messageLength = message[i][4].length();
                int offset = 0;
                while (offset < messageLength) {
                    pictureMessagePacket packet;
                    std::memset(packet.TextMessage, 0, sizeof(packet.TextMessage)); 
                    // 计算当前包的大小
                    int currentPacketSize = std::min(packetSize, messageLength - offset);
                    // 从 std::string 中提取子字符串
                    std::string subString = message[i][4].substr(offset, currentPacketSize);
                    // 复制子字符串到 packet 中
                    std::memcpy(packet.TextMessage, subString.c_str(), subString.length());
                    packet.TextMessage[subString.length()] = '\0'; // 显式添加 null 终止符，确保 C 风格字符串的正确性
                    packet.isLastPacket = (offset + packetSize >= messageLength);
                    packets.push_back(packet);
                    offset += packetSize;
                }

                //发送第一个包，告知发送者和接受者的身份信息
                pictureMessagePacket_first  pictureMessagePacket_0;
                std::memset(pictureMessagePacket_0.ReceiverName , 0 , sizeof(pictureMessagePacket_0.ReceiverName));
                std::memset(pictureMessagePacket_0.SenderName , 0 , sizeof(pictureMessagePacket_0.SenderName));
                std::memset(pictureMessagePacket_0.messageTime , 0 , sizeof(pictureMessagePacket_0.messageTime));
                strncpy(pictureMessagePacket_0.ReceiverName , userName.c_str() , sizeof(pictureMessagePacket_0.ReceiverName));
                strncpy(pictureMessagePacket_0.SenderName , friednName[i].c_str() , sizeof(pictureMessagePacket_0.SenderName));
                strncpy(pictureMessagePacket_0.messageTime, messageTime[i].c_str() , sizeof(pictureMessagePacket_0.messageTime) - 1);            
                pictureMessagePacket_0.packetSize = packets.size();
                std::vector<char> pictureMessagePacket_0_buffer(sizeof(pictureMessagePacket_first));
                std::cout << testMessageToClient->messageTime;
                memset(pictureMessagePacket_0_buffer.data(),'\0',sizeof(pictureMessagePacket_first));
                std::memcpy(pictureMessagePacket_0_buffer.data() , &pictureMessagePacket_0 ,sizeof(pictureMessagePacket_first));
                asio::write(*clientSocket , asio::buffer(pictureMessagePacket_0_buffer , sizeof(pictureMessagePacket_first)));

                //开始发送图片消息，每个包发送2046字节的包和1字节的标注是否是最后一个包

                for(int i = 0 ; i < packets.size() ; i++)
                {
                    std::vector<char> pictureMessagePacket_ll(sizeof(pictureMessagePacket));
                    memset(pictureMessagePacket_ll.data() , '\0' , sizeof(pictureMessagePacket));
                    std::memcpy(pictureMessagePacket_ll.data() , &packets[i] ,sizeof(pictureMessagePacket));
                    asio::async_write(*clientSocket,
                                    asio::buffer(pictureMessagePacket_ll , sizeof(pictureMessagePacket)) ,
                                    [& , packets , i](asio::error_code ec, std::size_t length){
                                        //std::cout << "发送第" << i << "个包，包大小:" << sizeof(pictureMessagePacket);
                                    });
                }           
        }
    }
}

