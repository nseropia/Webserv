#ifndef CGI_HPP
# define CGI_HPP

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
#include <map>
#include <sys/stat.h>
#include <dirent.h>

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class HTTPRequest;

class CGI
{
    private:
        char** createEnvVars(HTTPRequest* request, std::string& path);
    public:
        CGI();
        ~CGI();
        bool execute(HTTPRequest* request, std::string& content, std::string& path);
};

#endif