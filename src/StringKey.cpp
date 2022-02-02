#include "StringKey.hpp"
#include "utils.hpp"

stringKey::stringKey() {}
stringKey::stringKey(const string &str) : string(utils::strtoupper(str)) {}
stringKey::stringKey(const char *str) : string(utils::strtoupper(str)) {}
stringKey::stringKey(const stringKey &other) : string(other) {}
stringKey::~stringKey() {}

stringKey	&stringKey::operator=(const stringKey &other) {
	if (this == &other)
		return (*this);
	string::operator=(other);
	return (*this);
}
stringKey	&stringKey::operator=(const string &str) {
	string::operator=(utils::strtoupper(str));
	return (*this);
}
stringKey	&stringKey::operator=(const char str[]) {
	string::operator=(utils::strtoupper(str));
	return (*this);
}

bool	operator==(const string &value, const stringKey &key) {
	return (utils::strtoupper(value) == static_cast<string>(key));
}
bool	operator==(const stringKey &key, const string &value) {
	return (utils::strtoupper(value) == key);
}