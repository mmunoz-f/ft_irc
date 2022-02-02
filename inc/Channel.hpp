#pragma once

#include "Server.hpp"

class	TypeA_ModeFlag;

class Channel {
	friend class User;
	friend class Server;
	friend class TypeA_ModeFlag;
	friend class TypeB_ModeFlag;
	friend class TypeC_ModeFlag;
	friend class TypeD_ModeFlag;
	friend class O_ModeFlag;
	friend class V_ModeFlag;
	friend class B_ModeFlag;

	typedef struct	channel_user_t {
		User	*user;
		char	mode;

		channel_user_t();
		channel_user_t(User *, char);
		channel_user_t(const channel_user_t &other);
		channel_user_t	&operator=(const channel_user_t &other);
	} channel_user;

	public:
		typedef map<int, channel_user>					userfd_map;
		typedef map<const stringKey, channel_user_t *>	usernick_map;
		typedef set<string>								string_set;
		typedef Server::flag_map						flag_map;

		Channel();
		Channel(const string &name, Server *hostname);
		Channel(const Channel &other);
		~Channel();

		Channel				&operator=(const Channel &other);

		bool				addUser(User &user, const char mode);
		void				removeUser(const User &user, const string &reason = string());
		void				sendToAll(const int &fd, const string &msg) const;

		bool				empty();

		void				setTopic(const string &topic);
		const string		&getTopic() const;

		bool				isBanned(const string &user) const;

		Server				&getServer();

		void				setMode(const User &user, const Message &cmd);
		void				setUserMode(const User&user, const Message &cmd);
		short				getUserMode(const string &user) const;
		void				displayMode(const User &user) const;

		void				displayUsers(const User &user) const;
		int					countOperators();
		User*				getOperator();

		static flag_map		initChannelFlagMap();

	private:

		bool				execFlag(TypeA_ModeFlag *flag);

		static flag_map		flags;
		usernick_map		users_nick;
		string_set			ban_list;
		string				name;
		string				topic;
		string				password;
		userfd_map			users_fd;
		Server				*server;
		char				mode;
};
