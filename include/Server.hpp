#ifndef SERVER_HPP
# define SERVER_HPP

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

#include "ConfigFile.hpp"

class Server
{
    public:
        Server(ServerBlock& serverBlock, unsigned int& index, size_t& portIndex);
        ~Server();

        void getListenerSocket();        
        ServerBlock& getServerBlock();
        int getServerFd();
        sockaddr_in getServerAddress();
        // std::string getPath();
        unsigned int getServerIndex();
        std::vector<std::string> getIndex();

        class ServerException: public std::exception
        {
            public:
                ServerException(const char* msg);
		        const char* what() const throw();
            private:
                const char* _error_msg;
        };

    private:
        ServerBlock _serverBlock;
        // std::vector<int> _serverSockets;

        unsigned int _portIndex;
        unsigned short _port;
        unsigned long _clientMaxBodySize;
        std::string _root;
        std::vector<std::string> _index;
        std::vector<std::string> _serverName;

        unsigned int	 _serverIndex;
	    int				 _serverFd;
	    sockaddr_in		 _serverAddr;
};

#endif