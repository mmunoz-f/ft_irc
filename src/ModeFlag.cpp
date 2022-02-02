#include "ModeFlag.hpp"
#include "Channel.hpp"
#include "User.hpp"
#include "utils.hpp"

ModeFlag::ModeFlag() : type(NO_TYPE), id(0), value(0), op_need(false) {}
ModeFlag::~ModeFlag() {}
ModeFlag::ModeFlag(const char &new_id, const bool &new_op_need, const ModeType &new_type, const char &new_value) : type(new_type), id(new_id), value(new_value), op_need(new_op_need) {}
ModeFlag::ModeFlag(const ModeFlag &other) : type(other.type), id(other.id), op_need(other.op_need) {}

ModeFlag	&ModeFlag::operator=(const ModeFlag &other) {
	if (this == &other)
		return (*this);
	type = other.type;
	id = other.id;
	op_need = other.op_need;
	return (*this);
}

bool	ModeFlag::checkOp(const char &mode) const {
	if (op_need && !(mode & C_O))
		return (false);
	return (true);
}

TypeA_ModeFlag::TypeA_ModeFlag() : ModeFlag() {}
TypeA_ModeFlag::~TypeA_ModeFlag() {}
TypeA_ModeFlag::TypeA_ModeFlag(const TypeA_ModeFlag &other) : ModeFlag(other), set(other.set), rpl_list(other.rpl_list), rpl_end(other.rpl_end), rpl_end_msg(other.rpl_end_msg), rpl_notfound(other.rpl_notfound), rpl_already_on_list(other.rpl_already_on_list), rpl_set(other.rpl_set), rpl_unset(other.rpl_unset) {}
TypeA_ModeFlag::TypeA_ModeFlag(const char &new_id, const bool &new_op_need, const char &new_value, Channel::string_set Channel::*new_set, const string &new_rpl_list, const string &new_rpl_end, const string &new_rpl_end_msg, const string &new_rpl_notfound, const string &new_rpl_already_on_list, const string &new_rpl_set, const string &new_rpl_unset)
	: ModeFlag(new_id, new_op_need, A, new_value), set(new_set), rpl_list(new_rpl_list), rpl_end(new_rpl_end), rpl_end_msg(new_rpl_end_msg), rpl_notfound(new_rpl_notfound), rpl_already_on_list(new_rpl_already_on_list), rpl_set(new_rpl_set), rpl_unset(new_rpl_unset) {}

TypeA_ModeFlag	&TypeA_ModeFlag::operator=(const TypeA_ModeFlag &other) {
	if (this == &other)
		return (*this);
	ModeFlag::operator=(other);
	set = other.set;
	rpl_list = other.rpl_list;
	rpl_end = other.rpl_end;
	rpl_end_msg = other.rpl_end_msg;
	rpl_notfound = other.rpl_notfound;
	rpl_already_on_list = other.rpl_already_on_list;
	rpl_set = other.rpl_set;
	rpl_unset = other.rpl_unset;
	return (*this);
}

bool	TypeA_ModeFlag::setFlag(const int &fd, void *src, const string *arg) const {
	Channel *channel = reinterpret_cast<Channel *>(src);
	if (!checkOp(channel->users_fd[fd].mode)) {
		channel->server->sendReply(fd, channel->server->getServerIP(), string(ERR_CHANOPRIVSNEEDED) + " " + channel->users_fd[fd].user->getNickname() + " " + channel->name + " " + ERR_CHANOPRIVSNEEDED_MSG);
		return (false);
	}
	if ((channel->*set).count(*arg)) {
		channel->server->sendReply(fd, channel->server->getServerIP(), string(ERR_INVALIDMODEPARAM) + " " + channel->name + " " + id + " " + rpl_already_on_list + " " + *arg);
		return (false);
	}
	addToList(channel->*set, *arg);
	channel->mode |= value;
	channel->sendToAll(fd, channel->users_fd[fd].user->getNickname() + " " + rpl_set + " " + *arg);
	return (true);
}
bool	TypeA_ModeFlag::unSetFlag(const int &fd, void *src, const string *arg) const {
	Channel *channel = reinterpret_cast<Channel *>(src);
	if (!checkOp(channel->users_fd[fd].mode)) {
		channel->server->sendReply(fd, channel->server->getServerIP(), string(ERR_CHANOPRIVSNEEDED) + " " + channel->users_fd[fd].user->getNickname() + " " + channel->name + " " + ERR_CHANOPRIVSNEEDED_MSG);
		return (false);
	}
	if ((channel->*set).count(*arg) == 0) {
		channel->server->sendReply(fd, channel->server->getServerIP(), string(ERR_INVALIDMODEPARAM) + " " + channel->name + " " + id + " " + rpl_notfound + " " + *arg);
		return (false);
	}
	(channel->*set).erase(*arg);
	if ((channel->*set).empty())
		channel->mode &= ~value;
	channel->sendToAll(fd, channel->users_fd[fd].user->getNickname() + " " + rpl_unset + " " + *arg);
	return (true);
}

void	TypeA_ModeFlag::displayList(Channel &channel, const int &fd) {
	if ((channel.*set).empty())
		return (channel.server->sendReply(fd, channel.server->getServerIP(), rpl_end + " " + channel.name + " " + rpl_end_msg));

	string	reply(rpl_list + " " + channel.users_fd[fd].user->getNickname() + " " + channel.name);
	for (Channel::string_set::const_iterator it = (channel.*set).begin(); it != (channel.*set).end(); it ++)
		channel.server->sendReply(fd, channel.server->getServerIP(), reply + " " + *it);
	channel.server->sendReply(fd, channel.server->getServerIP(), rpl_end + " " + channel.name + " " + rpl_end_msg);
}

TypeB_ModeFlag::TypeB_ModeFlag() : ModeFlag() {}
TypeB_ModeFlag::~TypeB_ModeFlag() {}
TypeB_ModeFlag::TypeB_ModeFlag(const TypeB_ModeFlag &other) : ModeFlag(other), rpl_set(other.rpl_set), rpl_unset(other.rpl_unset) {}
TypeB_ModeFlag::TypeB_ModeFlag(const char &new_id, const bool &new_op_need, const char &new_value, const string &new_rpl_set, const string &new_rpl_unset)
	: ModeFlag(new_id, new_op_need, B, new_value), rpl_set(new_rpl_set), rpl_unset(new_rpl_unset) {}

TypeB_ModeFlag	&TypeB_ModeFlag::operator=(const TypeB_ModeFlag &other) {
	if (this == &other)
		return (*this);
	ModeFlag::operator=(other);
	rpl_set = other.rpl_set;
	rpl_unset = other.rpl_unset;
	return (*this);
}

bool	TypeB_ModeFlag::setFlag(const int &fd, void *src, const string *arg) const {
	Channel	*channel = reinterpret_cast<Channel *>(src);
	if (!checkOp(channel->users_fd[fd].mode)) {
		channel->server->sendReply(fd, channel->server->getServerIP(), string(ERR_CHANOPRIVSNEEDED) + " " + channel->users_fd[fd].user->getNickname() + " " + channel->name + " " + ERR_CHANOPRIVSNEEDED_MSG);
		return (false);
	}
	if (channel->users_nick.count((*arg)) == 0) {
		channel->server->sendReply(fd, channel->server->getServerIP(), string(ERR_NOSUCHNICK) + " " + channel->users_fd[fd].user->getNickname() + " " + *arg + " " + ERR_NOSUCHNICK_MSG);
		return (false);
	}
	if (channel->users_nick[(*arg)]->mode & value)
		return (false);
	channel->users_nick[(*arg)]->mode |= value;
	channel->sendToAll(fd, channel->users_fd[fd].user->getNickname() + " " + rpl_set + " " + *arg);
	return (true);
}
bool	TypeB_ModeFlag::unSetFlag(const int &fd, void *src, const string *arg) const {
	Channel	*channel = reinterpret_cast<Channel *>(src);
	if (!checkOp(channel->users_fd[fd].mode)) {
		channel->server->sendReply(fd, channel->server->getServerIP(), string(ERR_CHANOPRIVSNEEDED) + " " + channel->users_fd[fd].user->getNickname() + " " + channel->name + " " + ERR_CHANOPRIVSNEEDED_MSG);
		return (false);
	}
	if (channel->users_nick.count((*arg)) == 0) {
		channel->server->sendReply(fd, channel->server->getServerIP(), string(ERR_NOSUCHNICK) + " " + channel->users_fd[fd].user->getNickname() + " " + *arg + " " + ERR_NOSUCHNICK_MSG);
		return (false);
	}
	if (!(channel->users_nick[(*arg)]->mode & value))
		return (false);
	channel->users_nick[(*arg)]->mode &= ~value;
	channel->sendToAll(fd, channel->users_fd[fd].user->getNickname() + " " + rpl_unset + " " + *arg);
	return (true);
}

TypeC_ModeFlag::TypeC_ModeFlag() : ModeFlag() {}
TypeC_ModeFlag::~TypeC_ModeFlag() {}
TypeC_ModeFlag::TypeC_ModeFlag(const TypeC_ModeFlag &other) : ModeFlag(other) {}
TypeC_ModeFlag::TypeC_ModeFlag(const char &new_id, const bool &new_op_need, const char &new_value) : ModeFlag(new_id, new_op_need, C, new_value) {}

TypeC_ModeFlag	&TypeC_ModeFlag::operator=(const TypeC_ModeFlag &other) {
	if (this == &other)
		return (*this);
	ModeFlag::operator=(other);
	return (*this);
}

TypeD_ModeFlag::TypeD_ModeFlag() : ModeFlag() {}
TypeD_ModeFlag::~TypeD_ModeFlag() {}
TypeD_ModeFlag::TypeD_ModeFlag(const TypeD_ModeFlag &other) : ModeFlag(other) {}
TypeD_ModeFlag::TypeD_ModeFlag(const char &new_id, const bool &new_op_need, const char &new_value) : ModeFlag(new_id, new_op_need, D, new_value) {}

TypeD_ModeFlag	&TypeD_ModeFlag::operator=(const TypeD_ModeFlag &other) {
	if (this == &other)
		return (*this);
	ModeFlag::operator=(other);
	return (*this);
}

bool	TypeD_ModeFlag::setFlag(const int &fd, void *src, const string *) const {
	Channel	*channel = reinterpret_cast<Channel *>(src);
	if (!checkOp(channel->users_fd[fd].mode)) {
		channel->server->sendReply(fd, channel->server->getServerIP(), string(ERR_CHANOPRIVSNEEDED) + " " + channel->users_fd[fd].user->getNickname() + " " + channel->name + " " + ERR_CHANOPRIVSNEEDED_MSG);
		return (false);
	}
	if (channel->mode & value)
		return (false);
	channel->mode |= value;
	return (true);
}
bool	TypeD_ModeFlag::unSetFlag(const int &fd, void *src, const string *) const {
	Channel *channel = reinterpret_cast<Channel *>(src);
	if (!checkOp(channel->users_fd[fd].mode)) {
		channel->server->sendReply(fd, channel->server->getServerIP(), string(ERR_CHANOPRIVSNEEDED) + " " + channel->users_fd[fd].user->getNickname() + " " + channel->name + " " + ERR_CHANOPRIVSNEEDED_MSG);
		return (false);
	}
	if (!(channel->mode & value))
		return (false);
	channel->mode &= ~value;
	return (true);
}

O_ModeFlag::O_ModeFlag() : TypeB_ModeFlag('o', true, C_O, RPL_SETOP_MSG, RPL_UNSETOP_MSG) {}
O_ModeFlag::~O_ModeFlag() {}
O_ModeFlag::O_ModeFlag(const O_ModeFlag &other) : TypeB_ModeFlag(other) {};

O_ModeFlag	&O_ModeFlag::operator=(const O_ModeFlag &other) {
	if (this == &other)
		return (*this);
	TypeB_ModeFlag::operator=(other);
	return (*this);
}

V_ModeFlag::V_ModeFlag() : TypeB_ModeFlag('v', true, C_V, RPL_SETVOICE_MSG, RPL_UNSETVOICE_MSG) {}
V_ModeFlag::~V_ModeFlag() {}
V_ModeFlag::V_ModeFlag(const V_ModeFlag &other) : TypeB_ModeFlag(other) {};

V_ModeFlag	&V_ModeFlag::operator=(const V_ModeFlag &other) {
	if (this == &other)
		return (*this);
	TypeB_ModeFlag::operator=(other);
	return (*this);
}

B_ModeFlag::B_ModeFlag() : TypeA_ModeFlag('b', true, C_B, &Channel::ban_list, RPL_BANLIST, RPL_ENDOFBANLIST, RPL_ENDOFBANLIST_MSG, ERR_NOTONBANLIST_MSG, ERR_ALREADYONBANLIST_MSG, RPL_BANSET_MSG, RPL_BANUNSET_MSG) {}
B_ModeFlag::~B_ModeFlag() {}
B_ModeFlag::B_ModeFlag(const B_ModeFlag &other) : TypeA_ModeFlag(other) {};

B_ModeFlag	&B_ModeFlag::operator=(const B_ModeFlag &other) {
	if (this == &other)
		return (*this);
	TypeA_ModeFlag::operator=(other);
	return (*this);
}

void	B_ModeFlag::addToList(Channel::string_set &set, const string &arg) const {
	string tmp = arg;
	set.insert(utils::userMask(tmp));
}
