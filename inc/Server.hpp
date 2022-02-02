/*		(｡◕‿‿◕｡)		*/

#pragma once

#include "conf.hpp"
#include "Message.hpp"
#include "StringKey.hpp"

#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <map>
#include <set>
#include <list>
#include <ctime>
#include <vector>
#include <climits>
#include <cstring>
#include <sstream>
#include <iostream>
#include <algorithm>

class	User;
class	Channel;
class	ModeFlag;

using std::map;
using std::set;
using std::pair;
using std::list;
using std::string;
using std::vector;

class Server {
	public:
		typedef void (Server::*ptr)(const int&, const Message&);
		typedef map<stringKey, ptr>					command_map;
		typedef map<int, User>						userfd_map;
		typedef map<stringKey, User *>				usernick_map;
		typedef map<const stringKey, class Channel>	channel_map;
		typedef vector<pollfd>						pollfd_vct;
		typedef map<string, string>					opCredentials_t;

		typedef map<char, ModeFlag *>				flag_map;
		typedef map<int, string>					msg_map;

		Server();
		~Server();
		Server(const Server& other);
		Server&			operator=(const Server& other);

		void			init();
		void			start();
		void			pingloop();
		string			getServerIP();
		int				check(int argc, char** argv);
		void			getMsg(pollfd_vct::iterator &it);
		void			handleMsg(pollfd_vct::iterator &it);
		void			addClient(pollfd_vct::iterator &it);
		void			removeClient(pollfd_vct::iterator &it);
		void			updateTimestamp(const int &fd);
		void			validateUser(const int &fd, const Message &cmd, User &user);

		void			who(const int& fd, const Message& cmd);
		void			ping(const int& fd, const Message& cmd);
		void			pong(const int& fd, const Message& cmd);
		void			pass(const int& fd, const Message& cmd);
		void			nick(const int& fd, const Message& cmd);
		void			user(const int& fd, const Message& cmd);
		void			quit(const int& fd, const Message& cmd);
		void			join(const int& fd, const Message& cmd);
		void			part(const int& fd, const Message& cmd);
		void			mode(const int& fd, const Message& cmd);
		void			oper(const int& fd, const Message& cmd);
		void			list(const int& fd, const Message& cmd);
		void			motd(const int& fd, const Message& cmd);
		void			kick(const int& fd, const Message& cmd);
		void			ison(const int& fd, const Message& cmd);
		void			kill(const int& fd, const Message& cmd);
		void			topic(const int& fd, const Message& cmd);
		void			whois(const int& fd, const Message& cmd);
		void			names(const int& fd, const Message& cmd);
		void			squit(const int& fd, const Message& cmd);
		void			privmsg(const int& fd, const Message& cmd);

		bool			createChannel(const int &fd, const string &name);
		void			sendReplyToChannel(Channel &channel, const string &prefix, const string &reply, const int &self);
		void			sendReplyToAll(const string &prefix, const string &reply);
		void			sendReply(const int &fd, const string &prefix, const string &reply);
		void			sendCustom(const int &fd, const string& msg, size_t size, int flags);
		void			setTerminate(bool value);

	private:
		opCredentials_t	op_credentials;
		channel_map		channels;
		command_map		commands;
		userfd_map		users_fd;
		usernick_map	users_nick;
		msg_map			recv_leftovers;
		msg_map			send_leftovers;
		pollfd_vct		poll_fds;

		socklen_t		server_size, conn_size;
		sockaddr_in		server_addr, conn_addr;
		string			hostname;
		string			receivedMsg;
		string			server_passwd;
		string			timestamp_creation;
		char			buffer[BUFF_SIZE];
		int				server_fd;
		int				server_port;
		bool			terminate;

		void			loadCommandMap();
};

extern bool *killSwitch;