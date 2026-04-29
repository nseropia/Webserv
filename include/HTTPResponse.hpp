#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

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
#include "Cgi.hpp"
#include "webserv.hpp"

class HTTPRequest;

enum e_asset_type
{
	ISFILE,
	ISDIRECTORY
};

enum e_status_code
{
	SUCCESS = 200,
	CREATED = 201,
	NO_CONTENT = 204,
	MOVED = 301,
	BAD_REQUEST = 400,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	NOT_ALLOWED = 405,
	CONFLICT = 409,
    LENGTH_REQUIRED = 411,
	TOO_LARGE = 413,
    URI_TOO_LONG = 414,
	INTERNAL_ERR = 500,
    NOT_IMPLEMENTED = 501,
	GATEWAY_TIMEOUT = 504,
	NOT_SUPPORTED = 505
};

class HTTPResponse
{
    private:
        HTTPRequest* _request;
        std::string _statusLine;
        std::map<std::string, std::string> _headers;
        std::string _body;
        int  _statusCode;
        bool _responseComplete;
        std::string _content;
        std::string _asset;
        enum e_asset_type _assetType;
        std::string _index;

        void isResponseComplete(enum e_status_code code);
        std::string getStatusMessage();
        void handleMethods();
        void handleAsset();
        void handleLocation();
        void handleRedirect();
        void handleCgi();
        void doCGI(std::string& path);
        void handleError();
        void handleFile(std::string asset);
        bool hasIndex(std::vector<std::string> indices);
        void checkAutoIndex();
        std::string getMime(std::string asset);
        void uploadFileToDir();
        std::string getErrorPath();

    public:
        HTTPResponse(HTTPRequest* request);
        ~HTTPResponse();

        void setStatusCode(int code);
        void setReasonPhrase(std::string reason);
        void setHeader(std::string key, std::string value);
        void setBody(std::string body);

        std::string getContent();
        int getStatusCode();
        std::string getReasonPhrase();
        std::map<std::string, std::string> getHeaders();
        std::string getBody();
};

#endif