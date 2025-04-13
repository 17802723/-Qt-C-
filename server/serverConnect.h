#include <asio.hpp>
#include <iostream>

#include "chatClient.h"
class server
{
private:
    static int port;
    asio::io_context* serverIoc;
    asio::ip::tcp::acceptor* acceptor;  
public:
    void listen();
    void DoAccept();
    server();
};