#include "../include/Server.hpp"

Server::Server(
    ServerBlock& serverBlock,
    unsigned int& portIndex,
    size_t&       index
)
    : _serverBlock(serverBlock),
      _portIndex(portIndex),
      _root(_serverBlock.getRootVec().at(0)),
      _index(serverBlock.getIndexVec()),
      _serverName(serverBlock.getServerNameVec()),
      _serverIndex(index),
      _serverFd(-1)

{
    std::istringstream iss(serverBlock.getPortVec().at(portIndex));
    iss >> _port;
    std::istringstream isss(serverBlock.getClientMaxBodySizeVec().at(0));
    isss >> _clientMaxBodySize;
}

Server::~Server()
{
}

void Server::getListenerSocket()
{
    int reuse = 1;  // For setsockopt() SO_REUSEADDR, below
    int status;

    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // Fill in my IP for me

    status = getaddrinfo(_serverName.at(0).c_str(), \
    this->getServerBlock().getPortVec().at(_portIndex).c_str(), &hints, &servinfo);
    if (status != 0)
    {
        freeaddrinfo(servinfo);
        std::cerr << "get_listener_socket: " << gai_strerror(status) << std::endl;
        throw ServerException("Failed to get address info");
    }

    this->_serverFd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (_serverFd == -1)
    {
        freeaddrinfo(servinfo);
        throw ServerException("Failed to create socket");
    }

    int flag = fcntl(_serverFd, F_GETFL, 0);
    if (flag == -1)
    {
        freeaddrinfo(servinfo);
        throw ServerException("Failed GETFL");
    }
    if (fcntl(_serverFd, F_SETFL, flag | O_NONBLOCK) == -1)
    {
        freeaddrinfo(servinfo);
        throw ServerException("Failed SETFL");
    }
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
    {
        freeaddrinfo(servinfo);
        throw ServerException("Failed to set socket option REUSEADDR");
    }
    if (setsockopt(_serverFd, SOL_SOCKET, SO_KEEPALIVE, &reuse, sizeof(int)) == -1)
    {
        freeaddrinfo(servinfo);
        throw ServerException("Failed to set socket option KEEPALIVE");
    }

    _serverAddr = *(struct sockaddr_in *)servinfo->ai_addr;
    _serverAddr.sin_port = htons(_port);

    if (bind(_serverFd, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
    {
        freeaddrinfo(servinfo);
        throw ServerException("Failed to bind socket");
    }
    freeaddrinfo(servinfo);
    if (listen(_serverFd, 126) == -1)
        throw ServerException("Failed to listen the socket");
}

ServerBlock& Server::getServerBlock()
{
    return _serverBlock;
}

Server::ServerException::ServerException(const char* msg): _error_msg(msg) {}

const char* Server::ServerException::what() const throw()
{
	return _error_msg;
}

int Server::getServerFd()
{
    return _serverFd;
}

sockaddr_in Server::getServerAddress()
{
    return _serverAddr;
}

unsigned int Server::getServerIndex()
{
    return _serverIndex;
}

std::vector<std::string> Server::getIndex()
{
    return _index;
}