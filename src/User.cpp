/*		◕_◕		*/

#include "User.hpp"
#include "Channel.hpp"
#include "utils.hpp"
#include "ModeFlag.hpp"

User::User() : serveraccess(false) {
	memset(this, 0, sizeof(*this));
}

User::User(int i_fd, in_addr address, Server* i_server) : hostname(inet_ntoa(address)), server(i_server), fd(i_fd), mode(0), serveraccess(false) {}

User::User(const User &other) : serveraccess(false) { *this = other; }

User::~User() {}

User	&User::operator=(const User &other) {
	if (this == &other)
		return (*this);
	mode = other.mode;
	fd = other.fd;
	nickname = other.nickname;
	passwd = other.passwd;
	username = other.username;
	hostname = other.hostname;
	fullname = other.fullname;
	channels = other.channels;
	return (*this);
}

void	User::setMode(const Message &cmd) {
	string	result;
	bool	add = true;
	bool	changed = true;

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
			bool	worked;
			add ? worked = flags[cmd.params[1][i]]->setFlag(fd, this, NULL)
			: worked = flags[cmd.params[1][i]]->unSetFlag(fd, this, NULL);
			if (worked) {
				if (changed)
					add ? result += '+' : result += '-';
				result += cmd.params[1][i];
				changed = false;
			}
		}
		else
			server->sendReply(fd, server->getServerIP(), string(ERR_UMODEUNKNOWNFLAG) + " " + ERR_UMODEUNKNOWNFLAG_MSG);
	}
	if (!result.empty())
		server->sendReply(fd, nickname, "MODE :" + result);
}

void	User::displayMode() const {
	string	reply = string(RPL_UMODEIS) + " " + nickname + " " + RPL_UMODEIS_MSG;
	string	result;

	for (flag_map::const_iterator it = flags.begin(); it != flags.end(); it++)
		if (mode & it->second->value) {
			if (result.empty())
				result += " +";
			result += it->second->id;
		}
	server->sendReply(fd, hostname, reply + result);
}

const string &User::getNickname() const {
	return (nickname);
}

void	User::joinChannel(Channel *channel, const char mode) {
	if (channel->addUser(*this, mode) == false)
		return;
	channels[(channel->name)] = channel;
	channel->displayUsers(*this);
}

void	User::leaveChannel(Channel *channel, const string &reason) {
	if (channel == NULL) {
		for (User::channel_map::iterator it = this->channels.begin(); it != this->channels.end(); it++)
			it->second->removeUser(*this);
		channels.clear();
	}
	else {
		channel->removeUser(*this, reason);
		channels.erase((channel->name));
	}
}

string	User::getFullIdentifier() const {
	return string(nickname + "!" + username + "@" + hostname);
}

User::flag_map	User::initFlagMap() {
	flag_map	map;
	// map['i'] = new I_ModeFlag();
	return (map);
}

User::flag_map	User::flags = User::initFlagMap();
