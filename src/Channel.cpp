#include "Server.hpp"
#include "User.hpp"
#include "Channel.hpp"
#include "utils.hpp"
#include "ModeFlag.hpp"

Channel::channel_user::channel_user_t() : user(NULL), mode(0) {}
Channel::channel_user::channel_user_t(User *new_user, char new_mode) : user(new_user), mode(new_mode) {}
Channel::channel_user::channel_user_t(const channel_user_t &other) : user(other.user), mode(other.mode) {}
Channel::channel_user	&Channel::channel_user::operator=(const channel_user_t &other) {
	if (this == &other)
		return (*this);
	user = other.user;
	mode = other.mode;
	return (*this);
}

Channel::Channel() : mode(C_T) {}

Channel::Channel(const string &new_name, Server *new_server) : name(new_name), server(new_server), mode(C_T) {}

Channel::Channel(const Channel &other) { *this = other; }

Channel::~Channel() {}
Channel	&Channel::operator=(const Channel &other) {
	if (this == &other)
		return (*this);
	name = other.name;
	mode = other.mode;
	topic = other.topic;
	users_fd = other.users_fd;
	users_nick = other.users_nick;
	ban_list = other.ban_list;
	return (*this);
}

bool	Channel::addUser(User &user, const char mode) {
	if (users_fd.count(user.fd)) {
		server->sendReply(user.fd, server->getServerIP(), string(ERR_USERONCHANNEL) + " " + user.nickname + " " + name + " " + ERR_USERONCHANNEL_MSG);
		return (false);
	}
	if (isBanned(user.getFullIdentifier())) {
		server->sendReply(user.fd, server->getServerIP(), string(ERR_BANNEDFROMCHAN) + " " + user.nickname + " " + name + " " + ERR_BANNEDFROMCHAN_MSG);
		return false;
	}

	users_fd[user.fd] = channel_user(&user, mode);
	users_nick[user.nickname] = &users_fd[user.fd];
	sendToAll(user.fd, "JOIN :" + name);
	topic.empty() ?
			server->sendReply(user.fd, server->getServerIP(), string(RPL_NOTOPIC) + " " + user.nickname + " " + name + " " + RPL_NOTOPIC_MSG)
		:	server->sendReply(user.fd, server->getServerIP(), string(RPL_TOPIC) + " " + user.nickname + " " + name + " " + topic);
	return (true);
}

void	Channel::removeUser(const User &user, const string &reason) {
	if (!users_fd.count(user.fd))
		return;
	sendToAll(user.fd, "PART " + name + " :" + reason);
	users_nick.erase(users_fd[user.fd].user->nickname);
	users_fd.erase(user.fd);
}

void	Channel::sendToAll(const int &fd, const string &msg) const {
	for (userfd_map::const_iterator it = users_fd.begin(); it != users_fd.end(); it++)
		server->sendReply(it->first, users_fd.at(fd).user->getFullIdentifier(), msg);
}

bool	Channel::empty() {
	return (users_fd.empty());
}

void	Channel::setTopic(const string &new_topic) {
	topic = new_topic;
}

const string	&Channel::getTopic() const {
	return (topic);
}

bool	Channel::isBanned(const string &user) const {
	utils::keyCompare comp;
	for (string_set::const_iterator it = ban_list.begin(); it != ban_list.end(); it++)
		if (comp(*it, user))
			return (true);
	return (false);
}

void	Channel::setMode(const User &user, const Message &cmd) {
	string							result;
	string							success_args;
	bool							add = true;
	bool							changed = true;
	vector<string>::const_iterator	arg_index = cmd.params.begin() + 1;
	if (arg_index != cmd.params.end())
		arg_index++;

	for (size_t i = 0; i < cmd.params[1].length(); i++) {
		if (cmd.params[1][i] == '-') {
			if (add != false)
				changed = true;
			add = false;
		}
		else if (cmd.params[1][i] == '+') {
			if (add != true)
				changed = true;
			add = true;
		}
		else if (flags.count(cmd.params[1][i])) {
			ModeFlag *flag = flags[cmd.params[1][i]];
			bool	worked = false;
			if ((add && flag->type == A) && cmd.params.size() == 2)
				reinterpret_cast<TypeA_ModeFlag *>(flag)->displayList(*this, user.fd);
			else {
				if (flag->type == D || (!add && flag->type == C) || arg_index != cmd.params.end())
					add ? worked = flag->setFlag(user.fd, this, (flag->type == D) ? NULL : &(*arg_index))
					: worked = flag->unSetFlag(user.fd, this, (flag->type == D || flag->type == C) ? NULL : &(*arg_index));
				else
					server->sendReply(user.fd, server->getServerIP(), string(ERR_INVALIDMODEPARAM) + " " + user.nickname + " " + name + " " + (add ? '+' : '-') + flag->id + " " + ERR_BADPARAMUSE_MSG);
			}
			if (worked) {
				if (changed)
					add ? result += '+' : result += '-';
				result += cmd.params[1][i];
				if (!(flag->type == D) && !(!add && flag->type == C))
				 	success_args += " " + *arg_index;
				changed = false;
			}
			if (!(flag->type == D) && !(!add && flag->type == C))
			 	arg_index++;
		}
		else
			server->sendReply(user.fd, server->getServerIP(), string(ERR_UNKNOWNMODE) + " " + user.nickname + " " + cmd.params[1][i] + " " + ERR_UNKNOWNMODE_MSG);
	}
	if (!result.empty())
		for (userfd_map::const_iterator it = users_fd.begin(); it != users_fd.end(); it++)
			server->sendReply(it->second.user->fd, user.getFullIdentifier(), "MODE " + name + " " + result + success_args);
}

short	Channel::getUserMode(const string &user) const {
	if (users_nick.count(user))
		return (users_nick.at(user)->user->mode);
	else
		return (0);
}

void	Channel::displayMode(const User &user) const {
	string	reply = string(RPL_CHANNELMODEIS) + " " + user.nickname + " " + name + " :";
	string	result;

	for (flag_map::const_iterator it = flags.begin(); it != flags.end(); it++)
		if (mode & it->second->value) {
			if (result.empty())
				result += "+";
			result += it->second->id;
		}
	server->sendReply(user.fd, server->getServerIP(), reply + result);
}

void	Channel::displayUsers(const User &user) const {
	string	namesReply(string(RPL_NAMREPLY) + " " + user.nickname + " = " + name + " :");

	for (userfd_map::const_iterator it = users_fd.begin(); it != users_fd.end(); it++) {
		if (it != users_fd.begin())
			namesReply += " ";
		if (it->second.mode & C_O)
			namesReply += "@";
		else if (it->second.mode & C_V)
			namesReply += "+";
		namesReply += it->second.user->nickname;
	}
	server->sendReply(user.fd, server->getServerIP(), namesReply);
	server->sendReply(user.fd, server->getServerIP(), string(RPL_ENDOFNAMES) + " " + user.nickname + " " + name + " " + RPL_ENDOFNAMES_MSG);
}

int		Channel::countOperators() {
	int		count = 0;

	for (Channel::userfd_map::iterator mit = users_fd.begin(); mit != users_fd.end(); mit++) {
		if (mit->second.mode & C_O)
			count++;
	}
	return count;
}

User*	Channel::getOperator() {
	for (Channel::userfd_map::iterator mit = users_fd.begin(); mit != users_fd.end(); mit++) {
		if (mit->second.mode & C_O)
			return mit->second.user;
	}
	return NULL;
}

Channel::flag_map	Channel::initChannelFlagMap() {
	flag_map	map;
	map['m'] = new TypeD_ModeFlag('m', true, C_M);
	map['n'] = new TypeD_ModeFlag('n', true, C_N);
	map['t'] = new TypeD_ModeFlag('t', true, C_T);
	map['o'] = new O_ModeFlag();
	map['v'] = new V_ModeFlag();
	map['b'] = new B_ModeFlag();
	return (map);
}

Channel::flag_map	Channel::flags = Channel::initChannelFlagMap();
