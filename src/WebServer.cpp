#include "../include/WebServer.hpp"
#include "../include/Server.hpp"
#include "../include/ConfigFile.hpp"
#include "../include/ClientConnection.hpp"
#include "../include/webserv.hpp"

WebServer::WebServer(ConfigFile& config)
{
    setNumberOfServers(config.getServerVecSize());
    size_t globalSocketCounter(0);
    for (_serverIndex=0; _serverIndex < _numberOfServers; ++_serverIndex)
    {
        initServer(config.getServerBlockVec().at(_serverIndex), globalSocketCounter);
    }
    _totalSockets = globalSocketCounter;
}

void WebServer::initServer(ServerBlock& serverBlock, size_t& globalSocketCounter)
{
    unsigned int socketNumber = serverBlock.getPortSize();

    for (unsigned int portIndex=0; portIndex < socketNumber; ++portIndex)
    {
        this->_serverVec.push_back(Server(serverBlock, portIndex, globalSocketCounter));
        this->_serverVec[globalSocketCounter].getListenerSocket();
        ++globalSocketCounter;
    }
}

WebServer::~WebServer()
{
    for (std::map<int, ClientConnection*>::iterator it = _clientConn.begin(); it != _clientConn.end(); ++it)
    {
        delete it->second;
    }
}

void WebServer::setNumberOfServers(unsigned int num)
{
    _numberOfServers = num;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void add_to_pfds(std::vector<pollfd>& pfds, int newfd)
{
    struct pollfd pfd;
    pfd.fd = newfd;
    pfd.events = POLLIN | POLLOUT;
    pfd.revents = 0;
    pfds.push_back(pfd);
}

// Remove an index from the set
void del_from_pfds(std::vector<pollfd>& pfds, int i)
{
    pfds.erase(pfds.begin() + i);
}

bool WebServer::isPollFdServer(int &pollFD)
{
	for (int i = 0; i < this->_totalSockets; ++i)
	{
		if (pollFD == this->_serverVec[i].getServerFd())
			return true;
	}
	return false;
}

int WebServer::initPoll()
{
    int ret = poll(_pollfds.data(), _pollfds.size(), 0.5);
    if (ret == -1)
    {
        if (g_sigint_flag)
            return ret;
        std::cerr << "poll error: " << strerror(errno) << std::endl;
        return ret;
    }
    return ret;
}

void WebServer::startPollEventLoop()
{
    for (int i = 0; i < this->_totalSockets; ++i)
    {
        struct pollfd pfd;
        pfd.fd = _serverVec[i].getServerFd();
        pfd.events = POLLIN;
        pfd.revents = 0;
        _pollfds.push_back(pfd);
    }

    // Main loop
    while (!g_sigint_flag)
    {
        int poll_events = initPoll();
        if (poll_events > 0)
        {
            // Run through the existing connections looking for data to read
            for (_pollIndex = 0; _pollIndex < _pollfds.size(); _pollIndex++)
            {
                // std::cout << RED << "_pollfds size: " << _pollfds.size() << '\n' << RESET;
                if (_pollfds[_pollIndex].revents & (POLLERR | POLLHUP))
                {
                    close(_pollfds[_pollIndex].fd);
                    if (_clientConn[_pollfds[_pollIndex].fd])
                    {
                        delete _clientConn[_pollfds[_pollIndex].fd];
                        _clientConn.erase(_pollfds[_pollIndex].fd);
                    }   
                    if (this->isPollFdServer(_pollfds[_pollIndex].fd))
                        _pollfds.erase(_pollfds.begin() + _pollIndex);
                    _pollIndex--;
                    std::cerr << "Error handle the poll" << std::endl;
                }
                // Check if someone's ready to read
                if (_pollfds[_pollIndex].revents & POLLIN && !_clientConn[_pollfds[_pollIndex].fd])
                {
                     // Accept new connections if it's a listening socket    
                    if (this->isPollFdServer(_pollfds[_pollIndex].fd))
                        acceptNewConnection(_pollIndex);
                }
                if (_pollfds[_pollIndex].revents & POLLIN && _clientConn[_pollfds[_pollIndex].fd])
                { // Read data from socket
                        readClientConnection(_pollIndex);
                }
                else if (_pollfds[_pollIndex].revents & POLLOUT)
                {
                    respondToClient(_pollIndex);
                }
            } // END looping through file descriptors
        } // END for(;;)--and you thought it would never end!
    }
}

void WebServer::acceptNewConnection(size_t& pollIndex)
{
    std::cout << CYAN << "pollserver: new connection on server[" << pollIndex << ']' << RESET << std::endl;
    sockaddr_in clientAddress = this->_serverVec[pollIndex].getServerAddress();
    socklen_t addrlen = sizeof(clientAddress);
    
    int new_fd = accept(_serverVec[pollIndex].getServerFd(),\
        (struct sockaddr*)&clientAddress, &addrlen);
    if (new_fd == -1)
        perror("accept error");
    int flag = fcntl(new_fd, F_GETFL, 0);
    if (flag == -1)
        close(new_fd);
    if (fcntl(new_fd, F_SETFL, flag | O_NONBLOCK) == -1)
        close(new_fd);
    // std::cout << RED << "new fd: " << new_fd << '\n' << RESET;
    add_to_pfds(_pollfds, new_fd);
    _clientConn.insert(std::make_pair(new_fd, new ClientConnection(_serverVec[pollIndex])));
}

void WebServer::readClientConnection(size_t& pollIndex)
{
    char		buffer[4096];
    std::string	strToSend;

    memset(buffer, 0, 4096);
    int nbytes = recv(_pollfds[pollIndex].fd, buffer,\
     4096, 0);

    if (nbytes <= 0) // handle disconnection (0) and error (-1)
    {
        std::cout << ORANGE << "pollserver: socket " << _pollfds[pollIndex].fd << " hung up" << RESET << std::endl;
        
        // if (_clientConn.find(_pollfds[pollIndex].fd) == _clientConn.end())
        close(_pollfds[pollIndex].fd);
        delete _clientConn[_pollfds[pollIndex].fd];
        _clientConn.erase(_pollfds[pollIndex].fd);

        _pollfds.erase(_pollfds.begin() + pollIndex);
        // del_from_pfds(_pollfds, pollIndex);
        
        pollIndex--;
        
        // del_from_pfds(_pollfds[pollIndex].fd);
        return;
    }
    strToSend = "";
    strToSend.append(buffer, nbytes);
    
    // std::cout << strToSend;
    
    int fd = _pollfds[pollIndex].fd;

    // Check if fd exists in the map
    if (_clientConn.find(fd) != _clientConn.end())
    {
        // std::cout << "1\n" << fd << std::endl;
        ClientConnection* clientConn = _clientConn[fd];
        if (clientConn->getHTTPRequest())
        {
            if (clientConn->getHTTPRequest()->getRequestString().empty())
            {
                // std::cout << "1.a\n" << fd << std::endl;
                clientConn->getHTTPRequest()->setRequestString(strToSend);
            }
            else
            {
                // std::cout << "1.b\n" << fd << std::endl;
                clientConn->getHTTPRequest()->appendRequestString(strToSend);
            }
            // std::cout << "2\n" << fd << std::endl;
        }
        else
        {
            // std::cout << "3\n";
            clientConn->createHTTPRequest(strToSend);
        }
        clientConn->getHTTPRequest()->handleRequest(clientConn->getServer());
    }
    else
    {
        std::cerr << "Client connection not found for fd " << fd << std::endl;
    }
}

void WebServer::respondToClient(size_t& pollIndex)
{
    int fd = _pollfds[pollIndex].fd;

    if (_clientConn.find(fd) == _clientConn.end())
        return;

    ClientConnection* clientConn = _clientConn[fd];
    if (clientConn && clientConn->getHTTPRequest() && clientConn->getHTTPRequest()->isRequestComplete())
    {
        std::string response = clientConn->createResponse();
        int ret = send(fd, response.c_str(), response.size(), 0);
        clientConn->resetHTTPRequest();
        if (ret <= 0)
        {
            if (ret == 0)
                std::cout << "Warning: No data was sent to client on fd: " << fd << '\n';
            else
                std::cerr << "Error sending data to client" << '\n';
            close(_pollfds[pollIndex].fd);
            delete _clientConn[_pollfds[pollIndex].fd];
            _clientConn.erase(_pollfds[pollIndex].fd);
            _pollfds.erase(_pollfds.begin() + pollIndex);
            pollIndex--;
        }
    }
}

std::vector<Server> WebServer::getServerVec()
{
    return _serverVec;
}
