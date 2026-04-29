#include "../include/HTTPRequest.hpp"

template <typename T>
std::string toString(const T &value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

HTTPRequest::HTTPRequest()
{
    _requestStr = "";

    _method = "";
    _path = "";
    _query = "";
    _protocolVersion = "";

    _headers.clear();
    _hasHeader = false;
    _boundaryBegin = "";
    _boundaryEnd = "";
    _flag = false;

    _bodyStr = "";
    _unchunckedBodyStr = "";
    _body.clear();
    _bodySize = 0;
    _hasBody = false;

    _requestComplete = false;
    _statusCode = 0;
}

HTTPRequest::HTTPRequest(std::string str)
{
    _requestStr = str;

    _method = "";
    _path = "";
    _query = "";
    _protocolVersion = "";

    _headers.clear();
    _hasHeader = false;
    _boundaryBegin = "";
    _boundaryEnd = "";
    _flag = false;

    _bodyStr = "";
    _unchunckedBodyStr = "";
    _body.clear();
    _bodySize = 0;
    _hasBody = false;

    _requestComplete = false;
    _statusCode = 0;
}

HTTPRequest::HTTPRequest(const HTTPRequest &other){ (void)other; }

HTTPRequest &HTTPRequest::operator=(const HTTPRequest &other)
{
    if (this != &other){}
    return (*this);
}

HTTPRequest::~HTTPRequest() {}

void HTTPRequest::setMethod(std::string method)
{
    _method = method;
}

void HTTPRequest::setPath(std::string path)
{
    _path = path;
}

void HTTPRequest::setProtocolVersion(std::string protocolVersion)
{
    _protocolVersion = protocolVersion;
}

void HTTPRequest::setHeaders()
{
    std::istringstream request(_headerStr);
    std::string line;

    while( std::getline( request, line ) && !line.empty() )   
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        std::istringstream iss( line );
        std::string key, value;
        if( std::getline( iss, key, ':' ) )
        {
            if (std::getline(iss >> std::ws, value))
            {
                _headers[key] = value;
                // std::cout << "Header: " << key << " = " << value << std::endl;
            }
        }
    }
}

void HTTPRequest::setHeaders(std::map<std::string, std::string> headers)
{
    _headers = headers;
}

void HTTPRequest::setBodyStr(std::string body)
{
    _bodyStr = body;
}

void HTTPRequest::setBody(std::vector<std::pair<std::string, std::string> > body)
{
    _body = body;
}

std::vector<std::pair<std::string, std::string> > HTTPRequest::getBodyPairs()
{
    return _bodyPairs;
}

std::string HTTPRequest::getMethod()
{
    return _method;
}

std::string HTTPRequest::getPath()
{
    return _path;
}

std::string HTTPRequest::getProtocolVersion()
{
    return _protocolVersion;
}

std::string HTTPRequest::getQuery()
{
    return _query;
}

std::map<std::string, std::string> HTTPRequest::getHeaders()
{
    return _headers;
}

std::string HTTPRequest::getContentType()
{
    return _contentType;
}

std::string HTTPRequest::getContentLength()
{
    return toString(_contentLength);
}

std::string HTTPRequest::getcontentTypeCGI()
{
    return _contentTypeCGI;
}

std::string HTTPRequest::getBodyStr()
{
    return _bodyStr;
}

std::string HTTPRequest::getBB()
{
    return (_boundaryBegin);
}

void HTTPRequest::setRequestString(std::string str)
{
    _requestStr = str;
}

std::string HTTPRequest::getRequestString()
{
    return _requestStr;
}

std::string HTTPRequest::getFileName()
{
    return _fileName;
}

std::string HTTPRequest::getFileContent()
{
    return _fileContent;
}

void HTTPRequest::appendRequestString(std::string str)
{
    _requestStr += str;
}

void HTTPRequest::doMultipart()
{
    size_t boundaryStart = _bodyStr.find(_boundaryBegin);
    size_t filenameStart = _bodyStr.find("filename=\"", boundaryStart);
    filenameStart += 10;
    size_t filenameEnd = _bodyStr.find("\"", filenameStart);
    _fileName = _bodyStr.substr(filenameStart, filenameEnd - filenameStart);

    size_t contentStart = _bodyStr.find("\r\n\r\n", filenameEnd);
    contentStart += 4;
    size_t boundaryEndPos = _bodyStr.find(_boundaryEnd, contentStart);

    // _fileContent.resize(boundaryEndPos - contentStart);
    // std::copy(_bodyStr.begin() + contentStart, _bodyStr.begin() + boundaryEndPos, _fileContent.begin());

    _fileContent.assign(_bodyStr.begin() + contentStart, _bodyStr.begin() + boundaryEndPos);
    // std::cout << _fileName << '\n';
}

void HTTPRequest::doUrlrencoded()
{
    std::istringstream bodyStream(_bodyStr);
    std::string keyValuePair;
    while (getline(bodyStream, keyValuePair, '&'))
    {
        std::cout << "getline &" << '\n';
        std::istringstream keyValueStream(keyValuePair);
        std::string key, value;
        std::string token;
        bool isKey = true;
        while (getline(keyValueStream, token, '='))
        {
            std::cout << "getline =" << '\n';
            if (isKey)
            {
                key = token;
                isKey = false;
            }
            else
                value = token;
        }
        _bodyPairs.push_back(std::make_pair(key, value));
    }
    setBody(_bodyPairs);
}

void HTTPRequest::handleRequest(Server& server)
{
    parseRequest();
    if (!_requestComplete)
        return;
    // std::cout << "request complete" << std::endl;
    if (_contentType == "multipart/form-data")
        doMultipart();
    else if (_contentType == "application/x-www-form-urlencoded")
        doUrlrencoded();
    crossCheckLocationBlocks(server);
}

void HTTPRequest::crossCheckLocationBlocks(Server& server)
{
    size_t l = 0;
    _errorPageMap = server.getServerBlock().getErrorPagePath();
    std::map< std::string, std::map< std::string, std::vector<std::string> > >& locations =
        server.getServerBlock().getServerLocation();
    itMm locIt;
    for (locIt = locations.begin(); locIt != locations.end(); ++locIt)
    {
        std::string locationPath = locIt->first;
        if (locationPath.size() > _path.size()) continue;
        size_t	tmpLength = 0;
		for (size_t j = 0; j < _path.size() && locationPath[j] == _path[j]; j++)
			tmpLength++;
		if (tmpLength > l)
        {
            l = tmpLength;
			_location = locIt->second;
        }
    }
    _serverIndexVec = server.getIndex();
    if (_method == "POST" && _contentType.empty() && _transferEncoding.empty())
        _statusCode = NOT_IMPLEMENTED;
    if (_method != "POST" && _method != "GET" && _method != "DELETE")
        _statusCode = NOT_ALLOWED;
    else if (_bodySize > static_cast<size_t>(strtod(server.getServerBlock().getClientMaxBodySizeVec().at(0).c_str(), NULL)))
        _statusCode = TOO_LARGE;
}

bool isSpecChar(char c)
{
    std::string sc;

    sc = "-._~:/?#[]@!$&'()*+,;=%";
    for (size_t i = 0; i < sc.size(); i++)
    {
        if (c == sc[i])
            return true;
    }
    return false;
}

void HTTPRequest::isValidPath()
{
    if (_path.size() > 2048)
    {
        _statusCode = URI_TOO_LONG;
        _reasonPhrase = "uri too long";
    }
    for (size_t i = 0; i < _path.size(); i++)
    {
        if (!isalpha(_path[i]) && !isdigit(_path[i]) && !isSpecChar(_path[i]))
        {
            _statusCode = BAD_REQUEST;
            _reasonPhrase = "bad request";
        }
    }
}

void HTTPRequest::parseRequest()
{
    if (!_hasHeader)
    {
        if (isHeaderValid())
        {
            std::istringstream request(_headerStr);
            std::string line;

            if ( std::getline( request, line ) ) 
            {
                std::istringstream iss( line );
                iss >> _method >> _path >> _protocolVersion;
            }
            this->getQueryString();
            this->setHeaders();
            populateHandyHeaders();
            isValidPath();
            if (_method == "GET" || _method == "DELETE")
                _requestComplete = true;
            _hasHeader = true;
        }
    }
    if (!_hasBody && _hasHeader && !_requestComplete)
    {
        if (isBodyValid())
        {
            if (_contentLength != 0)
            {
                if (getBodySize() == _contentLength)
                    _hasBody = true;
            }
            else if (_contentLength == 0 && _transferEncoding == "chunked")
            {
                // std::cout << "body:\n" << _bodyStr << '\n';
                if (unchunkRequest(_bodyStr, _unchunckedBodyStr))
                    _hasBody = true;
                // std::cout << "body:\n" << _unchunckedBodyStr << '\n';
            }
            else if (_contentLength == 0 && _transferEncoding == "")
            {
                _statusCode = LENGTH_REQUIRED;
                _reasonPhrase = "length requires";
                _requestComplete = true;
            }
            else if (_transferEncoding != "" && _transferEncoding != "chunked")
            {
                _statusCode = NOT_IMPLEMENTED;
                _reasonPhrase = "not implemented";
                _requestComplete = true;
            }
        }
    }
    if (_hasHeader && _hasBody)
        _requestComplete = true;
}

void HTTPRequest::getQueryString()
{
    std::string path;
    size_t pathEnd;
    size_t questionmarkPos = _path.find("?");
    if (questionmarkPos == std::string::npos)
        pathEnd = _path.size();
    else
        pathEnd = questionmarkPos;
    for (size_t i = 0; i < pathEnd; i++)
        path += _path[i];
    if (questionmarkPos <= _path.size())
    {
        for (size_t i = questionmarkPos + 1; i < _path.size(); i++)
            _query += _path[i];
    }
    _path = path;
}

void HTTPRequest::populateHandyHeaders()
{
    if (_headers.empty())
        return;
    if (_headers["Content-Length"] != "")
        _contentLength = static_cast<int>(strtod(_headers["Content-Length"].c_str(), NULL));
    if (_headers["Transfer-Encoding"] != "")
        _transferEncoding = _headers["Transfer-Encoding"];
    if (_headers["Content-Type"] != "")
    {
        _contentTypeCGI = _headers["Content-Type"];
        if (_headers["Content-Type"].find("multipart/form-data") <= _headers["Content-Type"].size())
        {
            size_t semicolonPos = _headers["Content-Type"].find(';');
            if (semicolonPos != std::string::npos)
                _contentType = _headers["Content-Type"].substr(0, semicolonPos);
            size_t equalsPos = _headers["Content-Type"].find('=');
            if (equalsPos != std::string::npos)
            {
                _boundaryBegin += "--";
                _boundaryBegin += _headers["Content-Type"].substr(equalsPos + 1, _headers["Content-Type"].size());
                _boundaryEnd = _boundaryBegin + "--";
            }
            _flag = true;
        }
        if (!_flag)
            _contentType = _headers["Content-Type"];
    }
}

bool HTTPRequest::isHeaderValid()
{
    size_t pos = _requestStr.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
        std::string buff = _requestStr.substr(0, pos + 2);
        _headerStr += buff;
        return true;
    }
    return false;
}

bool HTTPRequest::isBodyValid()
{
    size_t pos = _requestStr.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
        _bodyStr = _requestStr.substr(pos + 4, _requestStr.size());
        return true;
    }
    return false;
}

size_t HTTPRequest::getBodySize()
{
    _bodySize = _bodyStr.size();
    return _bodySize;
}

std::map< std::string, std::string >& HTTPRequest::getErrorPageMap()
{
    return _errorPageMap;
}

std::map< std::string, std::vector<std::string> > HTTPRequest::getLocation()
{
    return _location;
}

std::string HTTPRequest::getRedirectPath()
{
    std::map< std::string, std::vector<std::string> > location;
    std::map< std::string, std::vector<std::string> >::iterator it;
    location = getLocation();

    for (it = location.begin(); it != location.end(); it++)  
    {
        if (it->first == "return")
        {
            for (size_t i = 0; i < it->second.size(); i++)
                return it->second[i];
        }
    }
    return ("");
}

std::string HTTPRequest::getRoot()
{
    std::map< std::string, std::vector<std::string> > location;
    std::map< std::string, std::vector<std::string> >::iterator it;
    location = getLocation();

    for (it = location.begin(); it != location.end(); it++)  
    {
        if (it->first == "root")
        {
            for (size_t i = 0; i < it->second.size(); i++)  
            {
                return it->second[i];
            }    
        }
    }
    return ("");
}

std::string HTTPRequest::getUpload()
{
    std::map< std::string, std::vector<std::string> > location;
    std::map< std::string, std::vector<std::string> >::iterator it;
    location = getLocation();

    for (it = location.begin(); it != location.end(); it++)  
    {
        if (it->first == "upload_dir")
        {
            for (size_t i = 0; i < it->second.size(); i++)  
            {
                return it->second[i];
            }    
        }
    }
    return ("");
}

bool HTTPRequest::getRedirect()
{
    std::map< std::string, std::vector<std::string> > location;
    std::map< std::string, std::vector<std::string> >::iterator it;
    location = getLocation();

    for (it = location.begin(); it != location.end(); it++)  
    {
        if (it->first == "return")
        {
            return true;
        }
    }
    return (false);
}

std::vector<std::string> HTTPRequest::getAllowedMethods()
{
    std::map<std::string, std::vector<std::string> > location = getLocation();
    std::vector<std::string> allowedMethodsVec;

    for (std::map<std::string, std::vector<std::string> >::iterator it = location.begin(); it != location.end(); ++it)  
    {
        if (it->first == "allow_methods")
        {
            for (size_t i = 0; i < it->second.size(); i++)  
            {
                allowedMethodsVec.push_back(it->second.at(i));
            }
            return allowedMethodsVec;
        }
    }
    return allowedMethodsVec;
}

bool HTTPRequest::getCGI()
{
    std::map<std::string, std::vector<std::string> > location = getLocation();

    for (std::map<std::string, std::vector<std::string> >::iterator it = location.begin(); it != location.end(); ++it)  
    {
        if (it->first == "cgi")
        {
            if (it->second[0] == ".php")
                return true;
        }
    }
    return false;
}

std::vector<std::string> HTTPRequest::getIndices()
{
    std::map<std::string, std::vector<std::string> > location = getLocation();
    std::vector<std::string> indicesVec;

    for (std::map<std::string, std::vector<std::string> >::iterator it = location.begin(); it != location.end(); ++it)  
    {
        if (it->first == "index")
        {
            for (size_t i = 0; i < it->second.size(); i++)  
            {
                indicesVec.push_back(it->second.at(i));
            }
            // for (size_t i = 0; i < indicesVec.size(); i++)
            //     std::cout << indicesVec.at(i) << '\n';
            return indicesVec;
        }
    }
    if (indicesVec.empty())
    {
        // for (size_t i = 0; i < _serverIndexVec.size(); i++)
        //     std::cout << _serverIndexVec.at(i) << '\n';
        return _serverIndexVec;
    }
    return indicesVec;
}

bool HTTPRequest::getAutoIndex()
{
    std::map< std::string, std::vector<std::string> > location = getLocation();
    std::map< std::string, std::vector<std::string> >::iterator it;

    for (it = location.begin(); it != location.end(); it++)  
    {
        if (it->first == "autoindex")
        {
            if (it->second[0] == "on")
                return true;
        }
    }
    return (false);
}

std::string HTTPRequest::getErrorPagePath(int errorCode)
{
    std::map< std::string, std::string > errorMap = getErrorPageMap();
    std::map< std::string, std::string >::iterator it;

    for (it = errorMap.begin(); it != errorMap.end(); it++)  
    {
        if (it->first == toString(errorCode))
            return it->second;
    }
    return ("");
}

bool	HTTPRequest::isHex(const std::string& str)
{
	for (size_t i = 0; i < str.size(); ++i)
	{
		char c = str[i];
		
		if (!(c >= '0' && c <= '9') && (c >= 'A' && c <= 'F') && (c >= 'a' && c <= 'f'))
		{
			return (false);
		}
	}
	
	return (true);
}

int		HTTPRequest::hexToInt(const std::string& str)
{
	if (!isHex(str))
	{
		throw std::invalid_argument("Error: invalid hexadecimal string.");
	}
	
	std::stringstream ss;
	int int_val;
	ss << std::hex << str;
	ss >> int_val;

	return (int_val);
}

bool	HTTPRequest::unchunkRequest(const std::string& chunked, std::string& unchunked)
{
	std::istringstream iss(chunked);
	std::string line;
	int chunk_size;

	while(std::getline(iss, line))
	{
		bool is_empty = true;
		
		for (size_t i = 0; i < line.size(); ++i)
		{
			if (!isspace(line[i]))
			{
				is_empty = false;
				break ;
			}
		}
		
		if (is_empty)
		{
			continue ;
		}
		
		if (!isHex(line))
		{
			std::cerr << "Error: invalid hexadecimal string." << std::endl;
			return (false);
		}

		chunk_size = hexToInt(line);
		
		if (chunk_size == -1)
		{
			std::cerr << "Error: unchunkedRequest failed." << std::endl;
			return (false);
		}
		
		if (chunk_size == 0)
		{
			break ;
		}

		std::vector<char> buffer(chunk_size);
		iss.read(buffer.data(), chunk_size);
		unchunked.append(buffer.data(), chunk_size);

		// std::cout << unchunked << std::endl; // for testing only!
	
		std::getline(iss, line);
	}
	return (!unchunked.empty());
}

int HTTPRequest::getStatus()
{
    return _statusCode;
}

bool HTTPRequest::isRequestComplete()
{
    return _requestComplete;
}