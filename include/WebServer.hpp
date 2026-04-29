#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

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
#include <map>
#include <exception>

class Server;
class ClientConnection;
class ServerBlock;
class ConfigFile;

// #include "ConfigFile.hpp"

class WebServer
{
    private:
        std::vector<Server> _serverVec;
        std::vector<struct pollfd> _pollfds;
        size_t _pollIndex;
        std::map<int, ClientConnection*> _clientConn;
        int _numberOfServers;
        int _totalSockets;
        int _clientIndex;
        int _serverIndex;

        // void processClientRequest_(size_t &pollIndex_);
        int initPoll();
        void acceptNewConnection(size_t& pollIndex);
        void readClientConnection(size_t& pollIndex);
        void respondToClient(size_t& pollIndex);
        // void del_from_pfds(int i);

    public:
        WebServer(ConfigFile& config);
        ~WebServer();

        void startPollEventLoop();
        void setNumberOfServers(unsigned int num);
        void initServer(ServerBlock& serverBlock, size_t& globalSocketCounter);

        std::vector<Server> getServerVec();
        bool isPollFdServer(int &pollFD); // change name;
};

#endif