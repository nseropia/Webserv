#include "../include/WebServer.hpp"
#include "../include/Server.hpp"
#include "../include/ClientConnection.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/ServerBlock.hpp"
#include "../include/ConfigFile.hpp"

#include "../include/webserv.hpp"
#include "../include/Signals.hpp"

int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
            throw std::invalid_argument("usage: ./webserv *.config");
        std::string conf_file = argv[1];
        ConfigFile config;
        config.parsConfFile(conf_file);
        std::signal(SIGINT, handle_sigint);
        WebServer webServer(config);
        webServer.startPollEventLoop();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error | " << e.what() << std::endl;
    }
    return 0;
}
