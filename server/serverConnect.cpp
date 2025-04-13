#include "serverConnect.h"
int server::port = 1145;//监听端口


server::server()
{
    serverIoc = new asio::io_context;
    acceptor = new asio::ip::tcp::acceptor(*serverIoc , 
        asio::ip::tcp::endpoint(asio::ip::tcp::v4() , port));
}

//开始监听
void server::listen()
{

    try
    {               
        DoAccept();       
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    serverIoc->run();
}

//等待用户接入
void server::DoAccept()
{   
    client.push_back(new ChatClient());
        acceptor->async_accept([&](const asio::error_code ec ,asio::ip::tcp::socket socket){
            asio::ip::tcp::endpoint remote_endpoint = socket.remote_endpoint();
            std::string client_ip = remote_endpoint.address().to_string();
            std::cout << "新socket接入"  << std::endl;
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
                }                    
            }
            else
            {
                std::cout << "正在将该用户添加至对话" << std::endl ;
                auto socket_ptr = std::make_shared<asio::ip::tcp::socket>(std::move(socket));
                client.back()->Session(socket_ptr);  // 传递这个变量的地址
                DoAccept();
            }
        });

}
