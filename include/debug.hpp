#include <iostream>
#include <sstream>

#define COLOR  "\033[1;92m"
#define RESET "\033[0m"

#ifdef NDEBUG
#define debug(...)
#define header(...)
#define newLine()
#else
#define debug(msg) \
    std::cerr << COLOR << __FILE__ << ":" << __LINE__ << ": " << msg << RESET << std::endl;
#define header(msg) \
    std::cerr << COLOR << "======= " << msg << " =======" << RESET << std::endl;
#define newLine() \
    std::cerr << std::endl;
#endif