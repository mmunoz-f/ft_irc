#pragma once

#include "Server.hpp"
#include "Channel.hpp"

enum	ModeType{
	NO_TYPE,
	A,
	B,
	C,
	D
};

class Channel;
class User;

class	ModeFlag {
	friend class Channel;
	friend class User;

	public:

		ModeFlag();
		virtual ~ModeFlag();
		ModeFlag(const char &id, const bool &op_need, const ModeType &type, const char &value);
		ModeFlag(const ModeFlag &other);

		ModeFlag		&operator=(const ModeFlag &other);

		virtual bool	setFlag(const int &fd, void *src, const string *arg) const = 0;
		virtual bool	unSetFlag(const int &fd, void *src, const string *arg) const = 0;

		bool			checkOp(const char &mode) const;

	protected:
		ModeType	type;
		char		id;
		char		value;
		bool		op_need;
};

class	TypeA_ModeFlag : public ModeFlag {
	public:
		TypeA_ModeFlag();
		virtual ~TypeA_ModeFlag();
		TypeA_ModeFlag(const TypeA_ModeFlag &other);
		TypeA_ModeFlag(const char &id, const bool &op_need, const char &value, Channel::string_set Channel::*set,
			const string &rpl_list, const string &rpl_end, const string &rpl_end_msg, const string &rpl_notfound, const string &rpl_already_on_list, const string &rpl_set, const string &rpl_unset);

		TypeA_ModeFlag	&operator=(const TypeA_ModeFlag &other);

		virtual void	addToList(Channel::string_set &set, const string &arg) const = 0;

		virtual bool	setFlag(const int &fd, void *src, const string *arg) const;
		virtual bool	unSetFlag(const int &fd, void *src, const string *arg) const;

		virtual void	displayList(Channel &channel, const int &fd);

	private:
		Channel::string_set	Channel::*set;
		string				rpl_list;
		string				rpl_end;
		string				rpl_end_msg;
		string				rpl_notfound;
		string				rpl_already_on_list;
		string				rpl_set;
		string				rpl_unset;
};

class	TypeB_ModeFlag : public ModeFlag {
	public:
		TypeB_ModeFlag();
		virtual ~TypeB_ModeFlag();
		TypeB_ModeFlag(const TypeB_ModeFlag &other);
		TypeB_ModeFlag(const char &id, const bool &op_need, const char &value, const string &rpl_set, const string &rpl_unset);

		TypeB_ModeFlag	&operator=(const TypeB_ModeFlag &other);

		bool	setFlag(const int &fd, void *src, const string *arg) const;
		bool	unSetFlag(const int &fd, void *src, const string *arg) const;

	private:
		string	rpl_set;
		string	rpl_unset;
};

class	TypeC_ModeFlag : public ModeFlag {
	public:
		TypeC_ModeFlag();
		virtual ~TypeC_ModeFlag();
		TypeC_ModeFlag(const TypeC_ModeFlag &other);
		TypeC_ModeFlag(const char &id, const bool &op_need, const char &value);

		TypeC_ModeFlag	&operator=(const TypeC_ModeFlag &other);
};

class	TypeD_ModeFlag : public ModeFlag {
	public:
		TypeD_ModeFlag();
		virtual ~TypeD_ModeFlag();
		TypeD_ModeFlag(const TypeD_ModeFlag &other);
		TypeD_ModeFlag(const char &id, const bool &op_need, const char &value);

		TypeD_ModeFlag	&operator=(const TypeD_ModeFlag &other);

		bool	setFlag(const int &fd, void *src, const string *arg) const;
		bool	unSetFlag(const int &fd, void *src, const string *arg) const;
};

class	O_ModeFlag : public TypeB_ModeFlag {
	public:
		O_ModeFlag();
		virtual ~O_ModeFlag();
		O_ModeFlag(const O_ModeFlag &other);

		O_ModeFlag	&operator=(const O_ModeFlag &other);
};

class	V_ModeFlag : public TypeB_ModeFlag {
	public:
		V_ModeFlag();
		virtual ~V_ModeFlag();
		V_ModeFlag(const V_ModeFlag &other);

		V_ModeFlag	&operator=(const V_ModeFlag &other);
};

class	B_ModeFlag : public TypeA_ModeFlag {
	public:
		B_ModeFlag();
		virtual ~B_ModeFlag();
		B_ModeFlag(const B_ModeFlag &other);

		B_ModeFlag	&operator=(const B_ModeFlag &other);

		void	addToList(Channel::string_set &set, const string &arg) const;
};
