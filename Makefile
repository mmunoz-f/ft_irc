#		( ͡° ʖ̯ ͡°)		#

SHELL			=	/bin/bash

NOCOL			=	\033[0m
ORG				=	\033[0;33m
GRN				=	\033[1;32m
CYN				=	\033[0;36m

NAME			=	ircserv

SRC_DIR			=	src/
OBJ_DIR			=	obj/
INC_DIR			=	inc/

SRCS			=	main.cpp \
					$(SRC_DIR)Server.cpp \
					$(SRC_DIR)Channel.cpp \
					$(SRC_DIR)User.cpp \
					$(SRC_DIR)utils.cpp \
					$(SRC_DIR)Message.cpp \
					$(SRC_DIR)ModeFlag.cpp \
					$(SRC_DIR)StringKey.cpp \
					$(SRC_DIR)Commands.cpp

OBJS			=	$(SRCS:%.cpp=$(OBJ_DIR)%.o)
INCLUDES		=	-I$(INC_DIR)

CXX				=	clang++
CXXFLAGS		=	-Wall -Werror -Wextra -std=c++98 $(INCLUDES) $(DEBUG)

MSG_SUCCESS		=	echo -e "$(GRN)=> Success!$(NOCOL)"
MSG_REMOVE		=	echo -e "$(ORG)=> Deleted!$(NOCOL)"

all:			$(NAME)

$(NAME):		$(OBJS)
				@$(CXX) $(CXXFLAGS) $^ -o $@
				@$(MSG_SUCCESS)

$(OBJ_DIR)%.o:	%.cpp
				@mkdir -p $(OBJ_DIR)
				@mkdir -p $(OBJ_DIR)$(SRC_DIR)
				$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
				@$(RM) -rf $(OBJ_DIR)

fclean:			clean
				@$(RM) $(NAME)
				@$(RM) *.dSYM

re:				fclean all

debug:			DEBUG += -g3 -fsanitize=address -D DEBUG=1
debug:			irc
