# Couleurs pour le Makefile
CLEAN_COLOR     =   \033[38;5;198m
REMOVE_COLOR    =   \033[38;5;201m
COMPILE_COLOR   =   \033[38;5;207m
BOX_COLOR       =   \033[38;5;219m
SUCCESS_COLOR   =   \033[38;5;157m
TITLE_COLOR     =   \033[38;5;39m
RESET           =   \033[0m

NAME = ircserv

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -I.

SRCS = src/main.cpp \
       src/Server.cpp \
       src/Client.cpp \
       src/Channel.cpp \
       src/Command.cpp \
       src/CommandHandler.cpp \
       src/FileTransfer.cpp \
       src/Bot.cpp \
       src/Utils.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(BOX_COLOR)╔═════════════════════════════════════════════════════════╗$(RESET)"
	@echo "$(BOX_COLOR)║                $(TITLE_COLOR)Compiling IRC Server...$(BOX_COLOR)                 ║$(RESET)"
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "$(BOX_COLOR)║      $(SUCCESS_COLOR)$(NAME) has been created successfully!$(BOX_COLOR)        ║$(RESET)"
	@echo "$(BOX_COLOR)╚═════════════════════════════════════════════════════════╝$(RESET)"

%.o: %.cpp
	@echo "$(COMPILE_COLOR)▶ Compiling: $<$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(CLEAN_COLOR)🧹 Cleaning object files...$(RESET)"
	@rm -f $(OBJS)

fclean: clean
	@echo "$(REMOVE_COLOR)🗑️  Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re