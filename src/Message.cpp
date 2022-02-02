/*		༼∵༽ ༼⍨༽ ༼⍢༽ ༼⍤༽		*/

#include "Message.hpp"
#include "utils.hpp"

Message::Message(string input) : singleColon(false) {
	size_t pos = 0;
	string tmp;

	input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
	full = input;
	if (input[0] == ':')
		extractString(input, prefix, pos);
	if (input[pos]) {
		extractString(input, name, pos);
		name = utils::strtoupper(name);
	}
	while (pos < input.length()) {
		extractString(input, tmp, pos);
		if (!tmp.empty())
			params.push_back(tmp);
		tmp.clear();
	}
	if (params.size() > 0)
		last = params[params.size() - 1];
}

Message::Message(const Message& other) : prefix(other.prefix), name(other.name), params(other.params), last(other.last), full(other.full), singleColon(other.singleColon) {}

Message::~Message() {}

Message &Message::operator=(Message const &other) {
	if (this == &other)
		return *this;
	prefix = other.prefix;
	name = other.name;
	params = other.params;
	last = other.last;
	full = other.full;
	singleColon = other.singleColon;
	return (*this);
}

string	Message::getStringParam() const {
	string tmp;
	unsigned long i = 0;

	for ( ; i < params.size() - 1; i++)
		tmp.append(params[i] + " ");
	tmp.append(params[i]);
	return tmp;
}

void	Message::checkSingleColon(string input, size_t pos) {
	singleColon = true;
	size_t i = pos;
	while (input[++i]) {
		if (input[i] && input[i] != ' ') {
			singleColon = false;
			return ;
		}
	}
}

void	Message::extractString(string input, string& output, size_t &pos) {
	bool scState = false;

	pos = input[pos] == ' ' ? pos + 1 : pos;
	if (input[pos] == ':' && pos != 0) {
		scState = true;
		pos++;
	}
	if (scState)
		checkSingleColon(input, pos);
	while (!singleColon && ((input[pos] && input[pos] != ' ' && !scState) || (input[pos] && scState)))
		output.push_back(input[pos++]);
	utils::strtrim(output);
}

void	Message::print() {
	std::cout
		<< "full: " << full << std::endl << std::endl
		<< "prefix: " << prefix << std::endl
		<< "name: " << name << std::endl
		<< "last: " << last << std::endl
		<< "singleColon: " << singleColon << std::endl;

	std::cout << "param_vector size: " << params.size() << std::endl;
	std::cout << "param_vector: ";
	for (unsigned long i = 0; i < params.size(); i++)
		std::cout << "| " << params[i] << " |";
	std::cout << std::endl;
}

string	Message::constructMsg() const {
	string		msg;

	for (vector<string>::const_iterator it = params.begin() + 1; it != params.end(); it++) {
		if (!msg.empty())
			msg += ' ';
		if (full.find(':') != string::npos && it == params.end() - 1)
			break ;
		msg += *it;
	}
	if (full.find(':') != string::npos)
		msg += last;
	return msg;
}