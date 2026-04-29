#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

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
#include <sstream>
#include <fstream>
# include <set>

#include "ServerBlock.hpp"

#define DEFAULT_PORT 8080
#define DEFAULT_IP "0.0.0.0"
# define WHITESPACE " \t\n\v\f\r"

typedef std::vector<std::string>::iterator itVec;
typedef std::map< std::string, std::vector<std::string> >::iterator itMap;
typedef std::map< std::string, std::map< std::string, std::vector<std::string> > >::iterator itMm;

class ConfigFile
{
	private:
        //std::map< std::string, std::vector<std::string> > _server_data;
        //std::map< std::string, std::map< std::string, std::vector<std::string> > > _locations;
        std::vector<ServerBlock> _servers;
        ServerBlock _current_server;

	public:
        ConfigFile();
        ~ConfigFile();

        // void	error_exit(const std::string& message);
        class ConfigException: public std::exception
        {
            public:
                ConfigException(const char* msg);
		const char* what() const throw();
            private:
                const char* _error_msg;
        };

        std::string	trimString(const std::string& str);
        std::vector<std::string>	splitLine(const std::string& line);
        bool	isSkip(const std::string& token);
        bool	isValid(const std::string& token);
        bool	checkSemicolon(std::string& token);
        void	checkBrackets(std::string& conf_file);
        void	checkConfFile(std::string& conf_file);

        void	parseLocationDirectives(std::string& loc_token, std::vector<std::string>& loc_tokens, std::map< std::string, std::vector<std::string> >& loc_data);
        void	parseLocationBlock(itVec& it, std::vector<std::string>& tokens, std::ifstream& file);
        void	parseServerDirectives(itVec& it, std::vector<std::string>& tokens);
        void	parseTokens(std::vector<std::string>& tokens, std::ifstream& file, bool& in_server_block);
        void	parsConfFile(std::string& conf_file);

        void	printServers();

        size_t  getServerVecSize();
        std::vector<ServerBlock> getServerBlockVec();
};

#endif
