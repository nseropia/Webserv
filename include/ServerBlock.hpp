#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

# include <iostream>
# include <vector>
# include <map>
# include <string>

class ServerBlock
{
	private:
        std::map< std::string, std::vector<std::string> > _server_data;
        std::map< std::string, std::string> _errorPages;
        std::map< std::string, std::map< std::string, std::vector<std::string> > > _locations;

        //ServerBlock(const ServerBlock& copy);
        //ServerBlock&	operator=(const ServerBlock& other);

	public:
        ServerBlock();
        ~ServerBlock();

        void	addServer(const std::string& key, const std::vector<std::string>& values);
        void    addErrorPage(const std::string& errorNumber, const std::string& path);
        void	addLocation(const std::string& path, const std::map< std::string, std::vector<std::string> >& loc_data);

        void	printData();

        std::map< std::string, std::vector<std::string> > getServerData();
        std::map< std::string, std::map< std::string, std::vector<std::string> > >& getServerLocation();

        int getPortSize();
        std::vector<std::string> getPortVec();
        std::vector<std::string> getServerNameVec();
        std::vector<std::string> getRootVec();
        std::vector<std::string> getIndexVec();
        std::vector<std::string> getClientMaxBodySizeVec();
        std::map< std::string, std::string > getErrorPagePath();
};

#endif