/*		༼∵༽ ༼⍨༽ ༼⍢༽ ༼⍤༽		*/

#pragma once

#include <string.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

using std::string;
using std::vector;

class Message {
	friend class User;
	friend class Server;
	friend class Channel;

	public:
		Message();
		Message(string input);
		Message(const Message& other);
		~Message();

		Message		&operator=(Message const &other);
		void		print();
		string		getStringParam() const;

	private:
		string				prefix;
		string				name;
		vector<string>		params;
		string				last;
		string				full;
		bool				singleColon;

		void		extractString(string input, string& output, size_t &pos);
		void		checkSingleColon(string input, size_t pos);
		string		constructMsg() const;
};
