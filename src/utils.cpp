#include "utils.hpp"

int 	utils::checkNickname(string nick) {
	if (nick.length() > 9 || !std::isalpha(*nick.begin()))
		return 0;
	for (unsigned int i = 0; i < nick.size(); i++)
		if (!std::isalnum(nick[i]) && nick[i] != '-' && nick[i] != '_' && nick[i] != '[' && nick[i] != ']'
		&& nick[i] != '{' && nick[i] != '}' && nick[i] != '\\' && nick[i] != '`' && nick[i] != '|')
			return 0;
	return 1;
}

bool 	utils::checkNicknameExists(const string& nick, Server::usernick_map users) {
	for (Server::usernick_map::iterator it = users.begin(); it != users.end(); it++)
		if (caseIndependentCompare(nick, it->first))
			return true;
	return false;
}

bool	utils::caseIndependentCompare(string s1, string s2) {
	std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
	std::transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
	return (s1 == s2);
}

void	utils::strtrim(string& str) {
	size_t		idx_begin = 0;
	size_t		idx_end = str.length();

	for (idx_begin = 0; idx_begin < str.length() && std::isspace(str[idx_begin]); idx_begin++);
	for (idx_end = str.length(); idx_end > 0 && std::isspace(str[idx_end - 1]); idx_end--);
	str = str.substr(idx_begin, idx_end);
}

string	utils::strtoupper(string str) {
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

vector<string>	utils::strsplit(string str, const char set) {
	std::vector<string>	v;
	if (str.empty())
		return (v);
	while (!str.empty()) {
		string	section = str.substr(0, str.find(set));
		if (!section.empty())
			v.push_back(section);
		str.find(set) != str.npos ?
			str.erase(0, str.find(set) + 1)
		:   str.erase(0, str.find(set));
	}
	return (v);
}

string		utils::randomStringGenerator(const int &size) {
	string	charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	string	randomstr;
	srand(std::time(NULL));

	for (int i = 0; i < size; i++)
		randomstr += charset[rand() % charset.length()];
	return randomstr;
}

static bool	recursiveComp(const string &key, const string &value, string::const_iterator i, string::const_iterator j) {
	if (i == key.end())
		return (j == value.end());
	else if (*i == '*') {
		for (; j != value.end(); j++)
			if (recursiveComp(key, value, i + 1, j))
				return (true);
		return (recursiveComp(key, value, i + 1, j));
	}
    else if (*i != *j)
        return (false);
    else
        return (recursiveComp(key, value, i + 1, j + 1));
}

bool	utils::keyCompare::operator()(const string &key, const string &value) {
    string  upKey = utils::strtoupper(key);
    string  upValue = utils::strtoupper(value);
	return (recursiveComp(upKey, upValue, upKey.begin(), upValue.begin()));
}

string	utils::userMask(string& src) {
	string		str_nick;
	string		str_username;
	string		str_hostname;

	if (src[0] != '@') {
		str_nick = src.substr(0, src.find('!'));
		src.erase(0, (src.find('!') != std::string::npos ? src.find('!') + 1 : src.find('!')));
		str_username = src.substr(0, src.find('@'));
		src.erase(0, (src.find('@') != std::string::npos ? src.find('@') + 1 : src.find('@')));
	}
	else
		src = src.substr(1);
	str_hostname = src;

	str_nick = str_nick.empty() ? "*" : str_nick;
	str_username = str_username.empty() ? "*" : str_username;
	str_hostname = str_hostname.empty() ? "*" : str_hostname;

	return string(str_nick + '!' + str_username + '@' + str_hostname);
}

void	utils::handleSig(int sig) {
	if (sig == SIGINT || sig == SIGQUIT)
		*killSwitch = true;
}

void	utils::recSignals() {
	signal(SIGINT, handleSig);
	signal(SIGQUIT, handleSig);
	signal(SIGPIPE, SIG_IGN);
}
