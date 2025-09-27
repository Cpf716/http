//
//  url.cpp
//  http
//
//  Created by Corey Ferguson on 9/24/25.
//

#include "url.h"

// Non-Member Fields

// Constant
std::map<std::string, int> PROTOCOLS = {
    { "http", 80 },
    { "https", 443 }
};

// Constructors

url::error::error(const std::string what) {
    this->_what = what;
}

url::param::param() {
    this->_set("");
}

url::param::param(const char* value) {
    this->_set(std::string(value));
}

url::param::param(const double value) {
    this->_set(value);
}

url::param::param(const std::string value) {
    this->_set(value);
}

url::param::param(const std::vector<std::string> value) {
    this->_set(value);
}

url::portinfo::portinfo() {
    this->_value = INT_MIN;
    this->_typed = false;
}

url::portinfo::portinfo(const int value, const bool typed) {
    this->_value = value;
    this->_typed = typed;
}

url::url() {
    this->_host = "";
    this->_protocol = "";
    this->_target = "";
}

url::url(const std::string value) {
    int start = 0;

    while (start < (int)value.length() - 1 && (value[start] != '/' || value[start + 1] != '/'))
        start++;

    if (start == value.length() - 1)
        start = 0;
    else {
        if (start)
            this->_protocol = tolowerstr(value.substr(0, start - 1));

        start += 2;
    }

    size_t end = start;

    while (end < value.length() && value[end] != '/')
        end++;

    std::vector<std::string> host = split(value.substr(start, end - start), ":");

    this->_host = host[0];

    if (host.size() == 1) {
        this->port() = portinfo((PROTOCOLS)[this->protocol()], false);
    } else
        this->port() = portinfo(parse_int(host[1]), true);

    std::vector<std::string> target = split(value.substr(end), "?");

    if (target.size() > 2)
        throw url::error("Unexpected token: ?");

    this->_target = target[0];

    if (target.size() == 2) {
        std::vector<std::string> query = split(target[1], "&");

        for (std::string param: query) {
            std::vector<std::string> kvp = split(param, "=");
            
            this->_params[kvp[0]] = kvp.size() == 1 ? "" : kvp[1];
        }
    }
}

// Operators

url::param::operator double() {
    return this->number();
}

url::param::operator std::string() {
    return this->str();
}

url::param::operator std::vector<std::string>() {
    return this->list();
}

double url::param::operator=(const double value) {
    return this->_set(value);
}

std::string url::param::operator=(const std::string value) {
    return this->_set(value);
}

std::vector<std::string> url::param::operator=(const std::vector<std::string> value) {
    return this->_set(value);
}

bool url::param::operator==(const char* value) {
    return this->str() == std::string(value);
}

bool url::param::operator==(const param value) {
    return this->str() == value.str();
}

bool url::param::operator==(const double value) {
    return this->number() == value;
}

bool url::param::operator==(const std::string value) {
    return this->str() == value;
}

bool url::param::operator!=(const char* value) {
    return !(*this == value);
}

bool url::param::operator!=(const param value) {
    return !(*this == value);
}

bool url::param::operator!=(const double value) {
    return !(*this == value);
}

bool url::param::operator!=(const std::string value) {
    return !(*this == value);
}

// Member Functions

std::string& url::host() {
    return this->_host;
}

url::param::map& url::params() {
    return this->_params;
}

std::string& url::protocol() {
    return this->_protocol;
}

url::portinfo& url::port() {
    return this->_port;
}

std::string& url::target() {
    return this->_target;
}

bool& url::portinfo::typed() {
    return this->_typed;
}

int& url::portinfo::value() {
    return this->_value;
}

double url::param::_set(const double value) {
    this->_parsed = true;
    this->_number = value;

    this->_set(std::to_string(this->_number));
    this->_list.push_back(this->str());

    return this->_number;
}

std::string url::param::_set(const std::string value) {
    this->_str = value;

    return this->str();
}

std::vector<std::string> url::param::_set(const std::vector<std::string> value) {
    this->_list = value;
    
    this->_set(join(value, ","));

    return this->_list;
}

std::vector<std::string> url::param::list() {
    if (this->_list.size() == 0) {
        split(this->_list, this->str(), ",");

        for (size_t i = 0; i < this->_list.size(); i++)
            this->_list[i] = trim(this->_list[i]);
    }

    return this->_list;
}

double url::param::number() {
    if (!this->_parsed) {
        this->_number = parse_number(this->str());
        this->_parsed = true;
    }

    return this->_number;
}

std::string url::param::str() const {
    return this->_str;
}

std::string url::str() {
    std::ostringstream oss;

    if (this->protocol().length())
        oss << this->protocol() << "://";

    oss << this->host();

    if (this->port().typed())
        oss << ":" << this->port().value();

    oss << this->target();

    if (this->params().size()) {
        oss << "?";

        size_t               index = 0;
        param::map::iterator it = this->params().begin();

        for (; index < this->params().size() - 1 && it != this->params().end(); index++, it++)
            oss << (* it).first << "=" << (* it).second.str() << "&";

        oss << (* it).first << "=" << (* it).second.str();
    }

    return oss.str();
}

const char* url::error::what() const throw() {
    return this->_what.c_str();
}
