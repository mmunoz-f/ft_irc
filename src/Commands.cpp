/*		ฅ^-ﻌ-^ฅ		*/

#include "User.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"

void		Server::pass(const int &fd, const Message &cmd) {
	User*	user = &users_fd[fd];
	if (user->serveraccess == true)
		return sendReply(fd, hostname, ERR_ALREADYREGISTERED + string(" ") + user->nickname + string(" ") + ERR_ALREADYREGISTERED_MSG);
	if (cmd.params.size() < 1)
		return sendReply(fd, hostname, ERR_NEEDMOREPARAMS + string(" ") + user->nickname + string(" ") + ERR_NEEDMOREPARAMS_MSG(PASS));
	if (cmd.params[0] != server_passwd)
		return sendReply(fd, hostname, ERR_PASSWDMISMATCH + string(" ") + user->nickname + string(" ") + ERR_PASSWDMISMATCH_MSG);
	users_fd[fd].serveraccess = true;
	std::cout << GRN << "[Server]: Client " << fd
		<< " from " << users_fd[fd].hostname
		<< " gained access to this IRC Server" << NCL << std::endl;
}

void	Server::nick(const int& fd, const Message &cmd) {
	bool	nickchange = false;
	string	old_nick;
	User*	user = &users_fd[fd];
	if (user->serveraccess == false)
		return ;
	if (cmd.params.empty())
		return sendReply(fd, hostname, ERR_NONICKNAMEGIVEN + string(" * ") + ERR_NONICKNAMEGIVEN_MSG);
	if (cmd.params.size() > 1 || !utils::checkNickname(cmd.params[0]))
		return sendReply(fd, hostname, ERR_ERRONEUSNICKNAME + string(" * ") + cmd.getStringParam() + string(" ") + ERR_ERRONEUSNICKNAME_MSG);
	if (utils::checkNicknameExists(cmd.params[0], users_nick) && !utils::caseIndependentCompare(user->nickname, cmd.params[0]))
		return sendReply(fd, hostname, ERR_NICKNAMEINUSE + string(" * ") + ERR_NICKNAMEINUSE_MSG);
	if (!user->nickname.empty()) {
		if (utils::caseIndependentCompare(user->nickname, cmd.params[0]))
			return ;
		sendReplyToAll(user->getFullIdentifier() , cmd.name + " :" + cmd.params[0]);
		old_nick = user->nickname;
		users_nick.erase(old_nick);
		nickchange = true;
	}
	user->nickname = cmd.params[0];
	users_nick[user->nickname] = user;
	if (nickchange == true) {
		for (User::channel_map::iterator cit = user->channels.begin(); cit != user->channels.end(); cit++) {
			cit->second->users_nick[(user->nickname)] = cit->second->users_nick[(old_nick)];
			cit->second->users_nick.erase((old_nick));
		}
	}
	validateUser(fd, cmd, *user);
}

void	Server::user(const int& fd, const Message &cmd) {
	User*	user = &users_fd[fd];
	if (user->serveraccess == false)
		return ;
	if (cmd.params.size() < 4)
		return sendReply(fd, hostname, ERR_NEEDMOREPARAMS + string(" ") + user->nickname + string(" ") + ERR_NEEDMOREPARAMS_MSG(USER));
	if (!user->username.empty())
		return sendReply(fd, hostname, ERR_ALREADYREGISTERED + string(" ") + user->nickname + string(" ") + ERR_ALREADYREGISTERED_MSG);
	user->username = cmd.params[0];
	user->hostname = inet_ntoa(conn_addr.sin_addr);
	user->fullname = cmd.last.empty() ? cmd.last : cmd.params[3];
	validateUser(fd, cmd, *user);
}

void	Server::whois(const int& fd, const Message &cmd) {
	if (cmd.params.size() < 1)
		return sendReply(fd, hostname, ERR_NEEDMOREPARAMS + string(" ") + users_fd[fd].nickname + string(" ") + ERR_NEEDMOREPARAMS_MSG(WHOIS));

	usernick_map::iterator	found = users_nick.find((cmd.params[0]));
	if (found == users_nick.end())
		return sendReply(fd, hostname, ERR_NOSUCHNICK + string(" ") + users_fd[fd].nickname + string(" ") + ERR_NOSUCHNICK_MSG);

	User			*user = found->second;
	string					channel_list;
	std::stringstream		iddletime;
	iddletime << (std::time(NULL) - user->timestamp) << " " << user->timestamp_login;

	sendReply(fd, hostname, RPL_WHOIS + string(" ") + users_fd[fd].nickname + " " + user->nickname + " " + user->username + " " + user->hostname + " * " + user->fullname);
	if (user->mode & S_OP)
		sendReply(fd, hostname, RPL_WHOISOPERATOR + string(" ") + users_fd[fd].nickname + " " + user->nickname + " :is an IRC operator");
	sendReply(fd, hostname, RPL_WHOISIDLE + string(" ") + users_fd[fd].nickname + " " + user->nickname + " " + iddletime.str() + " :seconds iddle time, signon time");

	for (channel_map::iterator cit = channels.begin(); cit != channels.end(); cit++) {
		if (cit != channels.begin())
			channel_list += " ";
		channel_list += cit->second.name;
	}
	sendReply(fd, hostname, RPL_WHOISCHANNELS + string(" ") + users_fd[fd].nickname + " " + user->nickname + " " + channel_list);
	sendReply(fd, hostname, RPL_ENDOFWHOIS + string(" ") + users_fd[fd].nickname + " " + user->nickname + " " + RPL_ENDOFWHOIS_MSG);
}

bool	Server::createChannel(const int &fd, const string &name) {
	if ((name[0] != '&' && name[0] != '#') || name.find(' ') != name.npos || name.find(7) != name.npos || name.find(',') != name.npos) {
		sendReply(fd, hostname, string(ERR_BADCHANMASK) + " " + name + " " + ERR_BADCHANMASK_MSG);
		return (false);
	}
	channels.insert(make_pair((name), Channel(name, this)));
	return (true);
}

void	Server::join(const int& fd, const Message &cmd) {
	User	&user = users_fd[fd];

	if (cmd.params.empty())
		return (sendReply(fd, hostname, string(ERR_NEEDMOREPARAMS) + " " + user.nickname + " " + ERR_NEEDMOREPARAMS_MSG(JOIN)));
	if (cmd.params[0] == "0") {
		user.leaveChannel();
		for (User::channel_map::const_iterator i = user.channels.begin(); i != user.channels.end(); i++)
			if (i->second->empty())
				channels.erase(i->second->name);
		return ;
	}

	vector<string>	tmp_channels = utils::strsplit(cmd.params[0], ',');
	vector<string>	tmp_keys;
	if (cmd.params.size() > 1)
		tmp_keys = utils::strsplit(cmd.params[1], ',');
	for (vector<string>::iterator it = tmp_channels.begin(); it != tmp_channels.end(); it++) {
		if (channels.count((*it)) == 0) {
			if (createChannel(fd, *it) == false)
				continue;
			user.joinChannel(&(channels[(*it)]), C_O);
		}
		else
			user.joinChannel(&(channels[(*it)]));
	}
}

void	Server::part(const int& fd, const Message &cmd) {
	User				&user = users_fd[fd];

	if (cmd.params.empty())
		return (sendReply(fd, hostname, string(ERR_NEEDMOREPARAMS) + " " + user.nickname + " " + ERR_NEEDMOREPARAMS_MSG(PART)));

	string			ch_upper = (cmd.params[0]);
	vector<string>	tmp_channels = utils::strsplit(ch_upper, ',');
	for (vector<string>::iterator it = tmp_channels.begin(); it != tmp_channels.end(); it++) {
		if (channels.count(*it) == 0)
			sendReply(fd, hostname, string(ERR_NOSUCHCHANNEL) + " " + user.nickname + " " + *it + " " + ERR_NOSUCHCHANNEL_MSG);
		else if (user.channels.count(*it) == 0)
			sendReply(fd, hostname, string(ERR_NOTONCHANNEL) + " " + user.nickname + " " + *it + " " + ERR_NOTONCHANNEL_MSG);
		else {
			Channel		*c = &channels[*it];
			string		successor;
			if (c->users_fd[fd].mode & C_O && c->users_fd.size() > 1 && c->countOperators() == 1) {
				for (Channel::userfd_map::iterator mit = c->users_fd.begin(); mit != c->users_fd.end(); mit++)
					if (mit->second.user->nickname != user.nickname) {
						successor = mit->second.user->nickname;
						break ;
					}
				if (!successor.empty()) {
					Message		tmp("MODE " + c->name + " +o " + successor);
					mode(fd, tmp);
				}
			}
			user.leaveChannel(&(channels[*it]));
		}
		if (channels[*it].empty())
			channels.erase(*it);
	}
}

void	Server::quit(const int& fd, const Message &cmd) {
	string		reason = "Quit: " + (cmd.params.size() > 0 ? cmd.params[0] : "");
	std::list<int>	unique_users;

	for (pollfd_vct::iterator it = poll_fds.begin(); it != poll_fds.end(); it++)
		if (it->fd == fd) {
			for (User::channel_map::iterator cit = users_fd[fd].channels.begin(); cit != users_fd[fd].channels.end(); cit++) {
				for (Channel::userfd_map::iterator uit = cit->second->users_fd.begin(); uit != cit->second->users_fd.end(); uit++)
					unique_users.push_back(uit->first);
			}
			unique_users.sort();
			unique_users.unique();
			for (std::list<int>::iterator lit = unique_users.begin(); lit != unique_users.end(); lit++)
				sendReply(*lit, hostname, "QUIT " + reason);
			removeClient(it);
			break ;
		}
}

void	Server::mode(const int &fd, const Message &cmd) {
	User	&user = users_fd[fd];

	if (cmd.params.empty())
		return (sendReply(fd, hostname, string(ERR_NEEDMOREPARAMS) + " " + user.nickname + " " + ERR_NEEDMOREPARAMS_MSG(MODE)));
	if (cmd.params[0][0] == '#' || cmd.params[0][0] == '&') {
		if (channels.count((cmd.params[0]))) {
			Channel	&channel = channels[(cmd.params[0])];
			if (cmd.params.size() == 1)
				channel.displayMode(user);
			else
				channel.setMode(user, cmd);
		}
		else
			return (sendReply(fd, hostname, string(ERR_NOSUCHCHANNEL) + " " + user.nickname + " " + cmd.params[0] + " " + ERR_NOSUCHCHANNEL_MSG));
	}
	else {
		if (users_nick.count((cmd.params[0]))) {
			if ((cmd.params[0]) != (user.nickname))
				return (sendReply(fd, hostname, string(ERR_USERSDONTMATCH) + " " + ERR_USERSDONTMATCH_SEE_MSG));
			if (cmd.params.size() == 1)
				user.displayMode();
			else
				user.setMode(cmd);
		}
		else
			return (sendReply(fd, hostname, string(ERR_NOSUCHNICK) + " " + cmd.params[0] + " " + ERR_NOSUCHNICK_MSG));
	}
}

void	Server::oper(const int &fd, const Message &cmd) {
	if (cmd.params.size() < 2)
		return (sendReply(fd, hostname, string(ERR_NEEDMOREPARAMS) + " " + users_fd[fd].nickname + " " + ERR_NEEDMOREPARAMS_MSG(OPER)));
	if (op_credentials.count(cmd.params[0]) && op_credentials[cmd.params[0]] == cmd.params[1]) {
		users_fd[fd].mode |= S_OP;
		sendReply(fd, hostname, string(RPL_YOUREOPER) + " " + users_fd[fd].nickname + " " + RPL_YOUREOPER_MSG);
	}
	else
		sendReply(fd, hostname, string(ERR_PASSWDMISMATCH) + " " + users_fd[fd].nickname + " " + ERR_PASSWDMISMATCH_MSG);
}

void	Server::topic(const int &fd, const Message &cmd) {
	User	&user = users_fd[fd];

	if (cmd.params.empty())
		return(sendReply(fd, hostname, string(ERR_NEEDMOREPARAMS) + " " + user.nickname + " " + ERR_NEEDMOREPARAMS_MSG(TOPIC)));

	string	ch_upper = (cmd.params[0]);
	if (channels.count(ch_upper) == 0)
		return (sendReply(fd, hostname, string(ERR_NOSUCHCHANNEL) + " " + user.nickname + " " + cmd.params[0] + " " + ERR_NOSUCHCHANNEL_MSG));
	if (user.channels.count(ch_upper) == 0)
		return (sendReply(fd, hostname, string(ERR_NOTONCHANNEL) + " " + user.nickname + " " + cmd.params[0] + " " + ERR_NOTONCHANNEL_MSG));
	Channel	&channel = channels[ch_upper];
	if (cmd.params.size() < 2) {
		string	topic = channel.getTopic();
		topic.empty() ?
			sendReply(fd, hostname, string(RPL_NOTOPIC) + " " + user.nickname + " " + channel.name + " " + RPL_NOTOPIC_MSG)
		:	sendReply(fd, hostname, string(RPL_TOPIC) + " " + user.nickname + " " + channel.name + " " + cmd.name + " " + topic);
	}
	else if (channel.mode & C_T && !(channel.users_fd[fd].mode & C_O))
		sendReply(fd, hostname, string(ERR_CHANOPRIVSNEEDED) + " " + user.nickname + " " + channel.name + " " + ERR_CHANOPRIVSNEEDED_MSG);
	else {
		string	topicstr = cmd.last[0] == ':' ? &cmd.last[1] : cmd.last;
		channel.setTopic(topicstr);
		if (cmd.last.empty())
			for (Channel::userfd_map::iterator it = channel.users_fd.begin(); it != channel.users_fd.end(); it++)
				sendReply(it->first, hostname, string(RPL_NOTOPIC) + " " + user.nickname + " " + channel.name + " " + RPL_NOTOPIC_MSG);
		else
			for (Channel::userfd_map::iterator it = channel.users_fd.begin(); it != channel.users_fd.end(); it++)
				sendReply(it->first, hostname, string(RPL_TOPIC) + " " + user.nickname + " " + channel.name + " :" + topicstr);
	}
}

void	Server::names(const int& fd, const Message& cmd) {
	if (cmd.params.empty())
		return (sendReply(fd, hostname, string(RPL_ENDOFNAMES) + " " + users_fd[fd].nickname + " * " RPL_ENDOFNAMES_MSG));

	vector<string>	tmp_channels = utils::strsplit(cmd.params[0], ',');
	for (vector<string>::iterator it = tmp_channels.begin(); it != tmp_channels.end(); it++) {
		if (!channels.count((*it)))
			sendReply(fd, hostname, string(RPL_ENDOFNAMES) + " " + users_fd[fd].nickname + " " + *it + " " + RPL_ENDOFNAMES_MSG);
		else
			channels[(*it)].displayUsers(users_fd[fd]);
	}
}

void	Server::privmsg(const int &fd, const Message &cmd) {
	User*		src = &users_fd[fd];
	int			err_fd = (cmd.name == "PRIVMSG" ? fd : -1);

	if (cmd.last.empty() && cmd.params.empty())
		return sendReply(err_fd, hostname, ERR_NOTEXTTOSEND + string(" ") + ERR_NOTEXTTOSEND_MSG);

	string				param_users_upper = (cmd.params[0]);
	vector<string>		u = utils::strsplit(param_users_upper, ',');
	for (vector<string>::iterator uit = u.begin(); uit != u.end(); uit++) {
		string		src_nick = (src->nickname);
		if ((*uit)[0] == '#') {
			Channel*		dst_ch = (channels.count(*uit) > 0 ? &channels[*uit] : NULL);
			if (!dst_ch)
				return sendReply(err_fd, hostname, string(ERR_NOSUCHCHANNEL) + " " + src->nickname + " " + ERR_NOSUCHCHANNEL_MSG);
			if (dst_ch->isBanned(src->getFullIdentifier()) && !(dst_ch->users_fd[fd].mode & C_O))
				return sendReply(src->fd, getServerIP(), string(ERR_CANNOTSENDTOCHAN) + " " + src->nickname + " " + dst_ch->name + " " + ERR_CANNOTSENDTOCHAN_MSG);
			else if (dst_ch->users_nick.count(src_nick) == 0 || (dst_ch->mode & C_M && !(dst_ch->users_nick[src_nick]->mode & (C_V | C_O))))
				sendReply(err_fd, hostname, string(ERR_CANNOTSENDTOCHAN) + " " + src->nickname  + " " + ERR_CANNOTSENDTOCHAN_MSG);
			else
				sendReplyToChannel(*dst_ch, src->getFullIdentifier(), cmd.name + " " + *uit + " :" + cmd.constructMsg(), src->fd);
		}
		else {
			User*		dst = (users_nick.count(*uit) > 0 ? users_nick[*uit] : NULL);
			if (!dst)
				sendReply(err_fd, hostname, ERR_NOSUCHNICK + string(" ") + ERR_NOSUCHNICK_MSG);
			else
				sendReply(dst->fd, src->getFullIdentifier(), cmd.name + " " + *uit + " :" + cmd.constructMsg());
		}
	}
}

void	Server::list(const int &fd, const Message &cmd) {
	sendReply(fd, hostname, string(RPL_LIST_HEAD) + " Channel :Users Name");
	if (cmd.params.empty()) {
		for(Server::channel_map::iterator it = channels.begin(); it != channels.end(); it++) {
			std::stringstream	size;
			size << (*it).second.users_fd.size();
			sendReply(fd, hostname, string(RPL_LIST) + " " + users_fd[fd].nickname + " " + (*it).second.name + " " + size.str() + " :" + (*it).second.topic);
		}
	}
	else {
		string			ch_upper = (cmd.params[0]);
		vector<string>	tmp_channels = utils::strsplit(ch_upper, ',');
		for (vector<string>::iterator it = tmp_channels.begin(); it != tmp_channels.end(); it++) {
			if (channels.find(*it) != channels.end()) {
				std::stringstream	size;
				size << channels[*it].users_fd.size();
				sendReply(fd, hostname, string(RPL_LIST) + " " + users_fd[fd].nickname + " " + channels[*it].name + " " + size.str() + " :" + channels[*it].topic);
			}
		}
	}
	sendReply(fd, hostname, string(RPL_LISTEND) + " " + RPL_LISTEND_MSG);
}

void	Server::motd(const int &fd, const Message &cmd) {
	(void)cmd;
	sendReply(fd, hostname, RPL_MOTDSTART + string(" ") + users_fd[fd].nickname + " " + RPL_MOTDSTART_MSG);
	sendReply(fd, hostname, RPL_MOTD + string(" ") + users_fd[fd].nickname + " :                                                 ");
	sendReply(fd, hostname, RPL_MOTD + string(" ") + users_fd[fd].nickname + " :                      ▀████▀███▀▀▀██▄   ▄▄█▀▀▀█▄█");
	sendReply(fd, hostname, RPL_MOTD + string(" ") + users_fd[fd].nickname + " :                        ██   ██   ▀██▄▄██▀     ▀█");
	sendReply(fd, hostname, RPL_MOTD + string(" ") + users_fd[fd].nickname + " :       ▄██  ███▀██▄     ██   ██   ▄██ ██▀       ▀");
	sendReply(fd, hostname, RPL_MOTD + string(" ") + users_fd[fd].nickname + " :      ████ ███   ██     ██   ███████  ██         ");
	sendReply(fd, hostname, RPL_MOTD + string(" ") + users_fd[fd].nickname + " :    ▄█▀ ██     ▄▄██     ██   ██  ██▄  ██▄        ");
	sendReply(fd, hostname, RPL_MOTD + string(" ") + users_fd[fd].nickname + " :  ▄█▀   ██  ▄▄█▀        ██   ██   ▀██▄▀██▄     ▄▀");
	sendReply(fd, hostname, RPL_MOTD + string(" ") + users_fd[fd].nickname + " :  █████████████████   ▄████▄████▄ ▄███▄ ▀▀█████▀ ");
	sendReply(fd, hostname, RPL_MOTD + string(" ") + users_fd[fd].nickname + " :       ██                                        ");
	sendReply(fd, hostname, RPL_MOTD + string(" ") + users_fd[fd].nickname + " :       ██                                        ");
	sendReply(fd, hostname, RPL_MOTD + string(" ") + users_fd[fd].nickname + " :                                                 ");
	sendReply(fd, hostname, RPL_ENDOFMOTD + string(" ") + users_fd[fd].nickname + " " + RPL_ENDOFMOTD_MSG);
}

void	Server::kick(const int &fd, const Message &cmd) {
	User*				user = &users_fd[fd];
	if (cmd.params.size() < 2)
		return sendReply(fd, hostname, ERR_NEEDMOREPARAMS + string(" ") + user->nickname + string(" ") + ERR_NEEDMOREPARAMS_MSG(KICK));

	string				ch_upper = (cmd.params[0]);
	string				params_user_upper = (cmd.params[1]);
	vector<string>		u = utils::strsplit(params_user_upper, ',');
	if (channels.count(ch_upper) == 0)
		return sendReply(fd, hostname, string(ERR_NOSUCHCHANNEL) + " " + user->nickname + " " + cmd.params[0] + " " + ERR_NOSUCHCHANNEL_MSG);

	Channel*			channel = &channels[ch_upper];
	if (!(channel->users_fd[fd].mode & C_O))
		return sendReply(fd, hostname, string(ERR_CHANOPRIVSNEEDED) + " " + user->nickname + " " + channel->name + " " + ERR_CHANOPRIVSNEEDED_MSG);

	string				reason = cmd.params[2].empty() ? RPL_DEFAULTKICK_MSG : cmd.params[2];
	for (vector<string>::iterator uit = u.begin(); uit != u.end(); uit++) {
		if (channel->users_nick.count(*uit) == 0) {
			sendReply(fd, hostname, string(ERR_NOTONCHANNEL) + " " + user->nickname + " " + *uit + " " + ERR_NOTONCHANNEL_MSG);
			continue ;
		}
		sendReplyToChannel(*channel, user->getFullIdentifier(), cmd.name + " " + channel->name + " " + *uit + " :" + reason, -1);
		channel->users_fd.erase(users_nick[*uit]->fd);
		channel->users_nick.erase(*uit);
	}
}

void	Server::ping(const int &fd, const Message &cmd) {
	User*	user = &users_fd[fd];

	if (cmd.params.size() < 1)
		return sendReply(fd, hostname, ERR_NEEDMOREPARAMS + string(" ") + user->nickname + string(" ") + ERR_NEEDMOREPARAMS_MSG(PING));
	string	mid = cmd.params.size() > 1 ? cmd.params[1] + " " : "";
	sendReply(fd, hostname, "PONG " + mid + cmd.params[0]);
}

void	Server::pong(const int &fd, const Message &cmd) {
	User*	user = &users_fd[fd];

	if (cmd.params.empty())
		return sendReply(fd, hostname, ERR_NEEDMOREPARAMS + string(" ") + user->nickname + string(" ") + ERR_NEEDMOREPARAMS_MSG(PONG));
	if (user->ping_request == false)
		return ;
	if (cmd.params[0] == user->ping_key || cmd.params[0] == hostname) {
		user->ping_request = false;
		user->timestamp = std::time(NULL);
	}
	else {
		sendReply(fd, hostname, "ERROR [Incorrect ping reply]");
		quit(fd, cmd);
	}
}

void	Server::ison(const int &fd, const Message &cmd) {
	User*	user = &users_fd[fd];
	string	list;

	if (cmd.params.empty())
		return sendReply(fd, hostname, ERR_NEEDMOREPARAMS + string(" ") + user->nickname + string(" ") + ERR_NEEDMOREPARAMS_MSG(ISON));
	for (size_t i = 0; i < cmd.params.size(); i++) {
		string		param_upper = (cmd.params[i]);
		if (users_nick.count(param_upper)) {
			list += cmd.params[i];
			if (i < cmd.params.size() - 1)
				list += " ";
		}
	}
	sendReply(fd, hostname, RPL_ISON + string(" ") + user->nickname + " :" + list);
}

void	Server::squit(const int &fd, const Message &cmd) {
	User*	user = &users_fd[fd];
	(void)cmd;
	if (!(user->mode & S_OP))
		return sendReply(fd, hostname, ERR_NOPRIVILEGES + string(" ") + user->nickname + string(" ") + ERR_NOPRIVILEGES_MSG);
	terminate = true;
	std::cout << BLU << "[Server]: IRC Server stopped" << std::endl;
}

void	Server::kill(const int &fd, const Message &cmd) {
	User*	user = &users_fd[fd];

	if (cmd.params.size() < 2)
		return sendReply(fd, hostname, ERR_NEEDMOREPARAMS + string(" ") + user->nickname + string(" ") + ERR_NEEDMOREPARAMS_MSG(KILL));
	if (!(user->mode & S_OP))
		return sendReply(fd, hostname, ERR_NOPRIVILEGES + string(" ") + user->nickname + string(" ") + ERR_NOPRIVILEGES_MSG);
	for (pollfd_vct::iterator it = poll_fds.begin(); it != poll_fds.end(); it++) {
		if (users_fd[it->fd].nickname == cmd.params[0]) {
			sendReply(it->fd, hostname, cmd.full);
			sendReply(it->fd, hostname, ":Closing link. Killed by " + user->nickname + " :" + cmd.params[1]);
			Message		tmpcmd("QUIT :Killed " + user->nickname + " " + cmd.params[1]);
			quit(it->fd, tmpcmd);
			return ;
		}
	}
}

void	Server::who(const int &fd, const Message &cmd) {
	User*		user = &users_fd[fd];
	Channel*	channel = NULL;

	if (cmd.params.size() < 1)
		return sendReply(fd, hostname, ERR_NEEDMOREPARAMS + string(" ") + user->nickname + string(" ") + ERR_NEEDMOREPARAMS_MSG(WHO));
	if (channels.count((cmd.params[0])))
		channel = &(channels[(cmd.params[0])]);
	if (channel) {
		for (Channel::usernick_map::const_iterator it = channel->users_nick.begin(); it != channel->users_nick.end(); it++) {
			string	prefix;
			char	mode = it->second->mode;
			User	&target = *(it->second->user);
			if (mode & C_O)
				prefix += "@";
			else if (mode & C_V)
				prefix += "+";
			sendReply(fd, hostname, string(RPL_WHOREPLY) + " " + user->nickname + " " + channel->name + " " + target.username + " " + target.hostname + " " + getServerIP() + " " +  target.nickname + " G" + prefix + " :0 " + target.fullname);
		}
	}
	sendReply(fd, hostname, RPL_ENDOFWHO + string(" ") + user->nickname + " " + cmd.params[0] + " " + RPL_ENDOFWHO_MSG);
}