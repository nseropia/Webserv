NAME        := webserv

# --- DIRECTORIES ---

SRCDIR      := src
INCDIR      := include
OBJDIR      := obj

# --- SOURCES ---

SRCS        := $(wildcard $(SRCDIR)/*.cpp)
OBJS        := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# --- FLAGS ---

CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -g -std=c++98 -I$(INCDIR)

# --- COMMANDS ---

RM          := rm -rf
MKDIR       := mkdir -p

# --- RULES ---

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@$(MKDIR) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJDIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all