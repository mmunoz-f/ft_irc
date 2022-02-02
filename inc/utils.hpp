#pragma once

#include "Server.hpp"

class Message;

namespace utils {
	int					checkNickname(string nick);
	bool				checkNicknameExists(const string& nick, Server::usernick_map user_map);
	bool				caseIndependentCompare(string s1, string s2);
	void				strtrim(string& str);
	string				strtoupper(string str);
	vector<string>		strsplit(string str, const char set);
	string				randomStringGenerator(const int &size);
	string				userMask(string& src);
	void				recSignals();
	void				handleSig(int sig);

	struct	keyCompare {
		bool	operator()(const string &value, const string &key);
	};
}
