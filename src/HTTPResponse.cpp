#include "../include/HTTPResponse.hpp"

HTTPResponse::HTTPResponse(HTTPRequest* request)
    : _request(request), _statusCode(0), _responseComplete(false)
{
    if (request->getStatus())
        isResponseComplete(static_cast<enum e_status_code>(request->getStatus()));
    handleLocation();
    handleAsset();
    handleCgi();
    handleMethods();
    handleError();
}

HTTPResponse::~HTTPResponse() {}

void HTTPResponse::setStatusCode(int code)
{
    _statusCode = code;
}

void HTTPResponse::setReasonPhrase(std::string reason)
{
    _statusLine = reason;
}

void HTTPResponse::setHeader(std::string key, std::string value)
{
    _headers[key] = value;
}

void HTTPResponse::setBody(std::string body)
{
    _body = body;
}

int HTTPResponse::getStatusCode()
{
    return _statusCode;
}

std::string HTTPResponse::getReasonPhrase()
{
    return _statusLine;
}

std::map<std::string, std::string> HTTPResponse::getHeaders()
{
    return  _headers;
}

std::string HTTPResponse::getBody()
{
    return _body;
}

std::string HTTPResponse::getContent()
{
    return _content;
}

void HTTPResponse::isResponseComplete(enum e_status_code code)
{
    _statusCode = code;
    if (_statusCode == CREATED || _statusCode == NO_CONTENT || _statusCode == MOVED)
    {
        std::ostringstream responseStream;
        responseStream << "HTTP/1.1" << " " << _statusCode << " " << getStatusMessage() << "\r\n";
        if (_statusCode == MOVED)
            responseStream << "Location: " << _asset + "/" << "\r\n";
        responseStream << "Content-Length: 0\r\n";
        responseStream << "Connection: keep-alive" << "\r\n";
        responseStream << "keep-alive: timeout=5, max=1000" << "\r\n";
        responseStream << "\r\n";
        _content = responseStream.str();
    }
    if (_statusCode >= 400 && _statusCode < 600)
        std::cout << RED << "[ERROR]: " << _statusCode << " " << getStatusMessage() << RESET << "\n";
    else
        std::cout << GREEN << "[RESPONSE]: " << _statusCode << " " << getStatusMessage() << RESET << "\n";
    _responseComplete = true;
}

std::string HTTPResponse::getStatusMessage()
{
    if (_statusCode == 200)
        return "OK";
    else if (_statusCode == 201)
        return "Created";
    else if (_statusCode == 204)
        return "No Content";
    else if (_statusCode == 301)
        return "Moved Permanently";
    else if (_statusCode == 400)
        return "Bad Request";
    else if (_statusCode == 403)
        return "Forbidden";
    else if (_statusCode == 404)
        return "Not Found";
    else if (_statusCode == 405)
        return "Method Not Allowed";
    else if (_statusCode == 409)
        return "Conflict";
    else if (_statusCode == 413)
        return "Content Too Large";
    else if (_statusCode == 414)
        return "URI Too Long";
    else if (_statusCode == 500)
        return "Internal Server Error";
    else
        return "Not Implemented";
}

void HTTPResponse::handleAsset()
{
    struct stat file;

    if (_responseComplete) return;
    _asset = _request->getRoot() + _request->getPath();
    if (stat(_asset.c_str(), &file) != 0)
        isResponseComplete(NOT_FOUND);
    else if (S_ISDIR(file.st_mode))
        _assetType = ISDIRECTORY;
    else
        _assetType = ISFILE;
}

void HTTPResponse::handleCgi()
{
    if (_responseComplete)
        return;
    if (_request->getCGI())
    {
        if (_assetType == ISFILE)
        {
            size_t i = _asset.find_last_of('.');
            if (i != std::string::npos && _asset.substr(i) == ".php")
                doCGI(_asset);
        }
        else if (hasIndex(_request->getIndices()))
        {
            std::string tmp = _asset + _index;
            size_t i = tmp.find_last_of('.');
            if (i != std::string::npos && tmp.substr(i) == ".php")
                doCGI(tmp);
        }
    }
}

void HTTPResponse::doCGI(std::string& path)
{
    CGI cgi;
    if (!cgi.execute(_request, _content, path))
        isResponseComplete(INTERNAL_ERR);
    else
        isResponseComplete(SUCCESS);
}

void HTTPResponse::handleMethods()
{
    std::string method = _request->getMethod();
    if (_responseComplete) return;

    if (method == "GET")
    {
        if (_responseComplete) return;
        if (_assetType == ISFILE)
            handleFile(_asset);
        else
        {
            std::string path = _request->getPath();
            if (path[path.size() - 1] != '/')
                isResponseComplete(MOVED);
            if (!_responseComplete)
            {
                if (hasIndex(_request->getIndices()))
                    handleFile(_asset + _index);
                else
                    checkAutoIndex();
            }
        }
    }
    else if (method == "POST")
    {
        // std::cout << "reaching POST\n";
        if (_responseComplete) return;
        if (_request->getUpload().empty())
            isResponseComplete(FORBIDDEN);
        else
            uploadFileToDir();
    }
    else if ( (method == "DELETE"))
    {
        if (_responseComplete) return;
        if (_assetType == ISFILE)
        {
            int res = std::remove(_asset.c_str());
            if (res == 0)
                isResponseComplete(NO_CONTENT);
            else
                isResponseComplete(INTERNAL_ERR);
        }
        else
        {
            std::string path = _request->getPath();
            if (path[path.size() - 1] != '/')
                isResponseComplete(CONFLICT);
            if (!_responseComplete)
            {
                if (access(_asset.c_str(), W_OK) != 0)
                    isResponseComplete(FORBIDDEN);
                else
                {
                    std::string cmd = "rm -rf " + _asset;
                    int resSys = std::system(cmd.c_str());
                    if (resSys == 0)
                        isResponseComplete(NO_CONTENT);
                    else
                        isResponseComplete(INTERNAL_ERR);
                }
            }
        }
    }
}

void HTTPResponse::uploadFileToDir()
{
    if (!_request->getFileName().size())
    {
        isResponseComplete(INTERNAL_ERR);
        return;
    }
    std::string fileName = _request->getUpload() + "/" + _request->getFileName();
    std::ofstream file(fileName.c_str());
    if (file.is_open())
    {
        file << _request->getFileContent();
        file.close();
        isResponseComplete(CREATED);
    }
    else
        isResponseComplete(INTERNAL_ERR);
}

void HTTPResponse::handleLocation()
{
    std::map< std::string, std::vector<std::string> > location;
    if (_responseComplete) return;
    location = _request->getLocation();
    if (location.size() == 0)
        isResponseComplete(NOT_FOUND);
    else if (_request->getRedirect())
        handleRedirect();
    else
    {
        std::vector<std::string> allowedMethods = _request->getAllowedMethods();
        if (std::find(allowedMethods.begin(), allowedMethods.end(), _request->getMethod()) \
            == allowedMethods.end() || allowedMethods.empty())
                isResponseComplete(NOT_ALLOWED);
    }
}

std::string HTTPResponse::getMime(std::string asset)
{
    static std::map<std::string, std::string> mimeTypes;

    if (mimeTypes.empty())
    {
        mimeTypes[".html"] = "text/html";
        mimeTypes[".htm"] = "text/html";
        mimeTypes[".css"] = "text/css";
        mimeTypes[".js"] = "application/javascript";
        mimeTypes[".json"] = "application/json";
        mimeTypes[".png"] = "image/png";
        mimeTypes[".jpg"] = "image/jpeg";
        mimeTypes[".jpeg"] = "image/jpeg";
        mimeTypes[".gif"] = "image/gif";
        mimeTypes[".txt"] = "text/plain";
        mimeTypes[".php"] = "application/x-httpd-php";
        mimeTypes[".ico"] = "image/vnd.microsoft.icon";
    }
    size_t dotIndex = asset.find_last_of('.');
    if (dotIndex == std::string::npos || dotIndex == asset.length() - 1)
        return "application/octet-stream";

    // Extract the file extension
    std::string extension = asset.substr(dotIndex);

    // Convert the extension to lowercase
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // Look up the extension in the MIME types map
    std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);
    if (it != mimeTypes.end())
        return it->second;
    else
        return "application/octet-stream"; // Default MIME type if not found
}

void HTTPResponse::handleFile(std::string asset)
{
    std::ifstream file(asset.c_str(), std::ios::binary);
    if (file)
    {
        std::ostringstream	responseStream;
        std::ostringstream	fileContent;
        fileContent << file.rdbuf();
        responseStream << "HTTP/1.1 200 OK\r\n";
        responseStream << "Content-Length: " << fileContent.str().length() << "\r\n";
        responseStream << "Connection: keep-alive" << "\r\n";
        responseStream << "keep-alive: timeout=5, max=1000" << "\r\n";
        responseStream << "Content-Type: "<< getMime(asset) << "\r\n";
        responseStream << "\r\n";
        responseStream << fileContent.str();
        file.close();
        _content = responseStream.str();
        isResponseComplete(SUCCESS);
    }
}

bool HTTPResponse::hasIndex(std::vector<std::string> indices)
{
    // for (size_t i = 0; i < indices.size(); i++)
    //     std::cout << "in has index: " << indices.at(i) << '\n';
    DIR											*ptr;
	struct dirent								*next;
	std::vector<std::string>					directoryF;
	std::vector<std::string>::const_iterator	it;

	ptr = opendir(_asset.c_str());
	if (!ptr)
		return (false);
	while ((next = readdir(ptr)))
    {
		directoryF.push_back(next->d_name);
    }
    
    // for (size_t i = 0; i < directoryF.size(); i++)
    // {
    //     std::cout << "directory F: " << directoryF.at(i) << '\n';
    // }

	for (size_t i = 0; i < indices.size(); i++)
	{
		if ((it = std::find(directoryF.begin(), directoryF.end(), indices[i]))
			!= directoryF.end())
		{
			_index = *it;
			closedir(ptr);
			return (true);
		}
	}
	closedir(ptr);
	return (false);
}


void HTTPResponse::checkAutoIndex()
{
    if (_request->getAutoIndex())
    {
        DIR*			ptr;
        struct dirent*	next;

        ptr = opendir(_asset.c_str());
        if (ptr != NULL)
        {
            std::ostringstream	responseStream;
            std::ostringstream	oss;

            oss << "<!DOCTYPE html><html><head><link rel=\"icon\" href=\"/mushroom.png\" sizes=\"48x48\"><title>Directory Index</title></head><body>";
            while ((next = readdir(ptr)) != NULL)
                oss << "<li><a href=\"" << next->d_name << "\">" << next->d_name << "</a></li>";
            oss << "</body></html>";
            closedir(ptr);

            responseStream << "HTTP/1.1 200 OK\r\n";
            responseStream << "Content-Length: " << oss.str().length() << "\r\n";
            responseStream << "Content-Type: text/html\r\n";
            responseStream << "\r\n";
            responseStream << oss.str();
            _content = responseStream.str();
            isResponseComplete(SUCCESS);
        }
    }
    else
        isResponseComplete(FORBIDDEN);
}

void HTTPResponse::handleRedirect()
{
    std::ostringstream	oSS;

	_statusCode = MOVED;
	oSS << "HTTP/1.1" << " " << _statusCode << " " << getStatusMessage() << "\r\n";
	oSS << "Location: " << _request->getRedirectPath() << "\r\n";
	oSS << "Content-Length: 0\r\n";
	oSS << "\r\n";
	_content = oSS.str();
	std::cout << GREEN << "[REDIRECTION]: " << _statusCode << " " << getStatusMessage() << RESET << "\n";
	_responseComplete = true;
}

std::string HTTPResponse::getErrorPath()
{
    std::string path;
    std::string errorPagePath = _request->getErrorPagePath(_statusCode);
    std::ifstream file(errorPagePath.c_str(), std::ios::binary);
    if (!errorPagePath.empty() && file)
        return errorPagePath;
    else
    {
        std::ostringstream oss;
        oss << "www/errors/" << _statusCode << ".html";
        path = oss.str();
        return path;
    }
}

void HTTPResponse::handleError()
{
    if (_statusCode >= 400 && _statusCode < 600)
    {
        std::string path = getErrorPath();
        std::ifstream file(path.c_str(), std::ios::binary);
        if (file)
        {
            std::ostringstream	responseStream;
            std::ostringstream	fileContent;
            fileContent << file.rdbuf();
            responseStream << "HTTP/1.1" << " " << _statusCode << " " << getStatusMessage() << "\r\n";
            responseStream << "Content-Length: " << fileContent.str().length() << "\r\n";
            responseStream << "Content-Type: "<< "text/html" << "\r\n";
            responseStream << "\r\n";
            responseStream << fileContent.str();
            file.close();
            _content = responseStream.str();
        }
    }
}