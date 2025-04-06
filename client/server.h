#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QString>

class server
{

private:

public:
    static QString server_ip ;
    server();
};

QString server::server_ip = "47.122.48.34";

#endif // SERVER_H
