#pragma once

#include <string>

using std::string;

class	stringKey : public string {
    public:
        stringKey();
        stringKey(const stringKey &other);
        stringKey(const string &string);
        stringKey(const char *str);
        ~stringKey();
        stringKey	&operator=(const stringKey &other);
        stringKey   &operator=(const string &str);
        stringKey   &operator=(const char *str);
    
    friend bool	operator==(const string &value, const stringKey &key);
    friend bool	operator==(const stringKey &key, const string &value);
};
bool	operator==(const string &value, const stringKey &key);
bool	operator==(const stringKey &key, const string &value);