#include "../include/webserv.hpp"

bool g_sigint_flag = false;

void    handle_sigint(int sig)
{
    (void)sig;
    g_sigint_flag = true;
    std::cout << MAGENTA << "\r[ALERT]: STOPPING THE SERVER...\n" << RESET;
}