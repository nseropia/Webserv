#include "../include/ServerBlock.hpp"

ServerBlock::ServerBlock() {}

ServerBlock::~ServerBlock() {}

void	ServerBlock::addServer(const std::string& key, const std::vector<std::string>& values)
{
	_server_data[key] = values;
}

void ServerBlock::addErrorPage(const std::string& errorNumber, const std::string& path)
{
	_errorPages[errorNumber] = path;
}

void	ServerBlock::addLocation(const std::string& path, const std::map< std::string, std::vector<std::string> >& loc_data)
{
	_locations[path] = loc_data;
}

void	ServerBlock::printData()
{
	std::map< std::string, std::vector<std::string> >::iterator it;

	for(it = _server_data.begin(); it != _server_data.end(); ++it)
	{
		std::cout << it->first << " = ";
		for(size_t i = 0; i < it->second.size(); ++i)
		{
			std::cout << it->second[i];
			if (i + 1 < it->second.size())
			{
				std::cout << ", ";
			}
		}
		std::cout << std::endl;
	}

	std::map< std::string, std::map< std::string, std::vector<std::string> > >::iterator loc_it;
	
	for (loc_it = _locations.begin(); loc_it != _locations.end(); ++loc_it)
	{
		std::cout << "Location path: " << loc_it->first << std::endl;
		std::map< std::string, std::vector<std::string> >::iterator loc_dir_it;
		for (loc_dir_it = loc_it->second.begin(); loc_dir_it != loc_it->second.end(); ++loc_dir_it)
		{
			std::cout << loc_dir_it->first << " = ";
			for (size_t i = 0; i < loc_dir_it->second.size(); ++i)
			{
				std::cout << loc_dir_it->second[i];
				if (i + 1 < loc_dir_it->second.size())
				{
					std::cout << ", ";
				}
			}
			std::cout << std::endl;
		}
	}
}
     
std::map< std::string, std::vector<std::string> > ServerBlock::getServerData()
{
    return _server_data;
}

std::map< std::string, std::map< std::string, std::vector<std::string> > >& ServerBlock::getServerLocation()
{
    return _locations;
}

std::vector<std::string> ServerBlock::getPortVec()
{
    return _server_data["listen"];
}

int ServerBlock::getPortSize()
{
    return _server_data["listen"].size();
}

std::vector<std::string> ServerBlock::getServerNameVec()
{
    return _server_data["host"];
}

std::vector<std::string> ServerBlock::getRootVec()
{
    return _server_data["root"];
}

std::vector<std::string> ServerBlock::getIndexVec()
{
    return _server_data["index"];
}

std::vector<std::string> ServerBlock::getClientMaxBodySizeVec()
{
    return _server_data["client_max_body_size"];
}

std::map< std::string, std::string > ServerBlock::getErrorPagePath()
{
	return _errorPages;
}