/*		(◕ᴥ◕ʋ)		*/

#pragma once

#include "Server.hpp"

class Channel;

class User {
	friend class Server;
	friend class Channel;

	public:
		typedef map<string, Channel*>		channel_map;
		typedef Server::flag_map			flag_map;

		User();
		User(int i_fd, in_addr address, Server* i_server);
		User(const User &other);
		~User();

		User				&operator=(const User &other);
		void				setMode(const Message &cmd);
		void				displayMode() const;

		const string		&getNickname() const;

		void				joinChannel(Channel *channel, const char mode = 0);
		void				leaveChannel(Channel *channel = NULL, const string &reason = string());

		static flag_map		initFlagMap();
		string				getFullIdentifier() const;

	private:
		channel_map			channels;
		static flag_map		flags;
		string				nickname;
		string				passwd;
		string				username;
		string				hostname;
		string				fullname;
		string				ping_key;
		Server*				server;
		time_t				timestamp;
		time_t				timestamp_login;
		int					fd;
		char				mode;
		bool				serveraccess;
		bool				ping_request;
};
