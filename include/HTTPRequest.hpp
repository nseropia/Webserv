#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <iostream>
#include <cstdlib>
#include <stdlib.h>
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
#include <algorithm>

#include "ConfigFile.hpp"
#include "Server.hpp"
#include "HTTPResponse.hpp"

class HTTPResponse;

class HTTPRequest
{
    private:
        std::string _requestStr;
        std::string _headerStr;
        // REQUEST LINE
        std::string _method;
        std::string _path; // request uri
        std::string _query; // ?created...
        std::string _protocolVersion;
        
        // HEADERS
        std::map<std::string, std::string> _headers;
        size_t _contentLength;
        std::string _contentType;
        std::string _contentTypeCGI; 
        std::string _transferEncoding;
        
        std::string _boundaryBegin;
        std::string _boundaryEnd;
        std::vector<std::pair<std::string, std::string> > _files;
        
        bool _hasHeader;
        bool _flag;

        // BODY
        std::string _bodyStr;
        std::string _unchunckedBodyStr;
        std::vector<std::pair<std::string, std::string> > _body;
        size_t _bodySize;
        bool _hasBody;
        std::string _fileName;
        std::string _fileContent;
        std::vector<std::pair<std::string, std::string> > _bodyPairs;

        std::string _reasonPhrase;
        int _statusCode;

        bool _requestComplete;
        std::map< std::string, std::string > _errorPageMap;
        std::map< std::string, std::vector<std::string> > _location;
        std::vector<std::string> _serverIndexVec;

        HTTPResponse *_response;

        void parseRequest();
        bool isHeaderValid();
        void populateHandyHeaders();
        bool isBodyValid();
        size_t getBodySize();
        void isValidPath();
        void doMultipart();
        void doUrlrencoded();

        bool isHex(const std::string& str);
        int hexToInt(const std::string& str);
        bool unchunkRequest(const std::string& chunked, std::string& unchunked);

    public:
        HTTPRequest();
        HTTPRequest(std::string str);
        HTTPRequest(const HTTPRequest &other);
        HTTPRequest &operator=(const HTTPRequest &other);
        ~HTTPRequest();

        std::string getMethod();
        std::string getPath();
        std::string getProtocolVersion();
        std::string getQuery();
        void getQueryString();
        std::map<std::string, std::string> getHeaders();
        std::string getBodyStr();
        std::string getBB();

        int getStatus();
        std::map< std::string, std::vector<std::string> > getLocation();
        std::map< std::string, std::string >& getErrorPageMap();
        std::string getRoot();
        bool getCGI();
        bool getRedirect();
        std::string getRedirectPath();
        std::vector<std::string> getAllowedMethods();
        std::vector<std::string> getIndices();
        std::string getUpload();
        bool getAutoIndex();
        std::string getFileName();
        std::string getFileContent();
        std::string getContentType();
        std::string getContentLength();
        std::string getcontentTypeCGI();
        std::string getErrorPagePath(int errorCode);

        void setMethod(std::string method);
        void setPath(std::string path);
        void setProtocolVersion(std::string protocolVersion);
        // void setQueryStrings(std::map<std::string, std::string> queryStrings);
        void setHeaders(std::map<std::string, std::string> headers);
        void setBodyStr(std::string body);
        void setBody(std::vector<std::pair<std::string, std::string> > body);
        std::vector<std::pair<std::string, std::string> > getBodyPairs();
        void setHeaders();

        bool isRequestComplete();

        void setRequestString(std::string str);
        std::string getRequestString();
        void appendRequestString(std::string str);
        void handleRequest(Server& server);
        void crossCheckLocationBlocks(Server& server);
};

#endif