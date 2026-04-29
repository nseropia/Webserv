#include "../include/Cgi.hpp"

static pid_t cgi_child_pid = -1;

void sigalrm_handler(int signum)
{
    (void)signum;
    // If the child is still running, kill it
    if (cgi_child_pid > 0) {
        kill(cgi_child_pid, SIGKILL);
    }
}

CGI::CGI() {}

CGI::~CGI()
{

}

template <typename T>
std::string toString(const T &value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

char** CGI::createEnvVars(HTTPRequest* request, std::string& path)
{
    std::map<std::string, std::string> envMap;

    envMap["REQUEST_METHOD"] = request->getMethod();
    envMap["SCRIPT_NAME"] = path;
    envMap["SCRIPT_FILENAME"] = path;
    envMap["REDIRECT_STATUS"] = "200";
    envMap["SERVER_PROTOCOL"] = request->getProtocolVersion();
    envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
    if (request->getMethod() == "GET")
        envMap["QUERY_STRING"] = request->getQuery();
    else if (request->getMethod() == "POST")
    {
        envMap["CONTENT_TYPE"] = request->getcontentTypeCGI();
        envMap["CONTENT_LENGTH"] = toString(request->getBodyStr().length());
    }
    char** env = new char*[envMap.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::iterator it = envMap.begin(); it != envMap.end(); ++it)
	{
        std::string var = it->first + "=" + it->second;
        env[i] = new char[var.size() + 1];
        std::strcpy(env[i], var.c_str());
        i++;
    }
    env[i] = NULL;
    return env;
}

bool CGI::execute(HTTPRequest* request, std::string& content, std::string& path)
{
    char** env = createEnvVars(request, path);
    std::string interpreter = "/usr/bin/php-cgi";

    char* argv[3];
    argv[0] = const_cast<char*>(interpreter.c_str());
    argv[1] = const_cast<char*>(path.c_str());
    argv[2] = NULL;

    int webToCGI[2]; // stdin
    int CGItoWeb[2]; // stdout

    if (pipe(webToCGI) == -1 || pipe(CGItoWeb) == -1)
    {
        perror("pipe()");
        for (int i = 0; env[i]; i++)
            delete[] env[i];
        delete[] env;
        return false;
    }
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork()");
        for (int i = 0; env[i]; i++)
            delete[] env[i];
        delete[] env;
        return false;
    }
    if (pid == 0)
    {
        close(CGItoWeb[0]);
        close(webToCGI[1]);
        dup2(webToCGI[0], STDIN_FILENO);
        dup2(CGItoWeb[1], STDOUT_FILENO);
        dup2(CGItoWeb[1], STDERR_FILENO);
        close(webToCGI[0]);
        close(CGItoWeb[1]);
        if (execve(argv[0], argv, env) == -1)
        {
            perror("execve()");
            exit(1);
        }
    }
    else
    {
        cgi_child_pid = pid; // store globally or as static variable so handler can access it

        // Set up the signal handler for SIGALRM
        struct sigaction sa;
        sa.sa_handler = sigalrm_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGALRM, &sa, NULL);

        // Set a timeout of, say, 5 seconds
        alarm(3);

        close(webToCGI[0]);
        close(CGItoWeb[1]);
        std::string req = request->getBodyStr();
            
        write(webToCGI[1], req.c_str(), req.size());
        close(webToCGI[1]);
        int s;
        waitpid(pid, &s, 0);

        alarm(0);

        std::string resp;
        char buff[128];
        size_t br;
        while ((br = read(CGItoWeb[0], buff, sizeof(buff) -1)) > 0)
        {
            buff[br] = '\0';
            resp += buff;
        }
        close(CGItoWeb[0]);


        std::ostringstream	responseStream;
        responseStream << "HTTP/1.1 200 OK\r\n";
        responseStream << "Content-Length: " << resp.size() << "\r\n";
        responseStream << "Connection: keep-alive" << "\r\n";
        responseStream << "keep-alive: timeout=5, max=1000" << "\r\n";
        responseStream << "Content-Type: " << "text/html" << "\r\n";
        responseStream << "\r\n";
        responseStream << resp;
        content = responseStream.str();

        for (int i = 0; env[i]; i++)
            delete[] env[i];
        delete[] env;

        if (WIFEXITED(s) && WEXITSTATUS(s) != 0)
        {
            std::cerr << "ERROR: CGI" << WEXITSTATUS(s) << '\n';
            return false;
        }
    }
    return true;
}