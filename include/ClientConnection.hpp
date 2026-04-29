#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <sstream>

// class ConfigFile;
// class HTTPRequest;
// class HTTPResponse;
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "ConfigFile.hpp"

// #define SEND_BUFFER_SIZE 1024 * 100 // 100 KB
#define BUFFER_SIZE 1025

/* example of a GET request

    GET /path/file.html HTTP/1.1 \r\n
    Host: www.host1.com \r\n
    \r\n
    BODY
*/

class ClientConnection
{
    private:
        int _client_sockfd;
        Server& _server;
        std::stringstream _sendBuffer;
        // std::string _requestStr;
        bool _requestLineIsParsed;
        bool _headersIsParsed;
        int _contentLength;
        int _bytesReceivedBody;
        HTTPRequest* _request;
    
    public:
        // ClientConnection();
        ClientConnection(Server& server);
        ClientConnection(const ClientConnection &src);
        ClientConnection &operator=(const ClientConnection &rhs);
        ~ClientConnection();

        HTTPRequest* getHTTPRequest();
        Server& getServer();

        // int getSockFD();

        void resetHTTPRequest();
        std::string	createResponse();
        void setRequestString(std::string str);
        std::string getRequestString();
        void appendRequestString(std::string str);
        void createHTTPRequest(std::string str);

        std::string _receivedBuffer;
};

#endif