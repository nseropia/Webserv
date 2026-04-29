#include "../include/ClientConnection.hpp"
#include <algorithm>

// ClientConnection::ClientConnection()
//     : _request(new HTTPRequest())
// {
// }

ClientConnection::ClientConnection(Server& server)
    : _server(server)
{
    _request = new HTTPRequest();
}

ClientConnection::ClientConnection(const ClientConnection &other)
    : _server(other._server)
{
    *this = other;
}

ClientConnection &ClientConnection::operator=(const ClientConnection &rhs)
{
    _server = rhs._server;
    _sendBuffer.str("");
    _sendBuffer.clear();
    _sendBuffer << rhs._sendBuffer.str();

    return *this;
}

ClientConnection::~ClientConnection()
{
    delete _request;
}

// template <typename T>
// std::string toString(const T &value)
// {
// 	std::ostringstream oss;
// 	oss << value;
// 	return oss.str();
// }

HTTPRequest* ClientConnection::getHTTPRequest()
{
    return _request;
}

std::string	ClientConnection::createResponse()
{
    std::string str;
    HTTPResponse* response = new HTTPResponse(_request);
    str = response->getContent();
    delete response;
    return str;
}

void ClientConnection::createHTTPRequest(std::string str)
{
    _request = new HTTPRequest(str);
}

void ClientConnection::resetHTTPRequest()
{
    delete _request;
    _request = NULL;
}

Server& ClientConnection::getServer()
{
    return _server;
}