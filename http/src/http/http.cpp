//
//  http.cpp
//  socket
//
//  Created by Corey Ferguson on 9/24/25.
//

#include "http.h"

namespace http {
    // Non-Member Fields

    // Constant
    std::map<int, std::string> REDIRECT_CODES = {
        { 302, "Found" },
        { 307, "Temporary Redirect" },
        { 308, "Permanent Redirect" }
    };
    const std::string          HTTP_VERSION = "HTTP/1.1";

    std::map<int, std::string> _error_codes = {
        { 0, "Unknown error" },
        { 404, "Not Found" },
        { 500, "Internal Server Error"}
    };
    size_t                     _timeout = 30;

    // Constructors

    error::error(const std::string what) {
        this->_status = 0;
        this->_what = what;
    }

    error::error(const size_t status, const std::string what) {
        this->_status = status;
        this->_what = what;
    }

    header::header() {
        this->_set("");
    }

    header::header(const char* value) {
        this->_set(std::string(value));
    }

    header::header(const int value) {
        this->_set(value);
    }

    header::header(const std::string value) {
        this->_set(value);
    }

    header::header(const std::vector<std::string> value) {
        this->_set(value);
    }

    request::request(const std::string method, class url url, header::map headers, const std::string body) {
        this->_method = method;
        this->_url = url.target();
        this->_params = url.params();
        this->_headers = headers;
        this->_body = body;
    }

    // Operators

    header::operator int() {
        return this->int_value();
    }

    header::operator std::string() {
        return this->str();
    }

    header::operator std::vector<std::string>() {
        return this->list();
    }

    int header::operator=(const int value) {
        return this->_set(value);
    }

    std::string header::operator=(const std::string value) {
        return this->_set(value);
    }

    std::vector<std::string> header::operator=(const std::vector<std::string> value) {
        return this->_set(value);
    }

    bool header::operator==(const char* value) {
        return this->str() == std::string(value);
    }

    bool header::operator==(const header value) {
        return this->str() == value.str();
    }

    bool header::operator==(const int value) {
        return this->int_value() == value;
    }

    bool header::operator==(const std::string value) {
        return this->str() == value;
    }

    bool header::operator!=(const char* value) {
        return !(*this == value);
    }

    bool header::operator!=(const header value) {
        return !(*this == value);
    }

    bool header::operator!=(const int value) {
        return !(*this == value);
    }

    bool header::operator!=(const std::string value) {
        return !(*this == value);
    }

    // Member Functions

    int header::_set(const int value) {
        this->_parsed = true;
        this->_int = value;

        this->_set(std::to_string(this->_int));
        this->_list.push_back(this->str());

        return this->_int;
    }

    std::string header::_set(const std::string value) {
        this->_str = value;

        return this->str();
    }

    std::vector<std::string> header::_set(const std::vector<std::string> value) {
        this->_list = value;
        
        this->_set(join(this->list(), ", "));

        return this->list();
    }

    std::string request::body() const {
        return this->_body;
    }

    header::map request::headers() {
        return this->_headers;
    }

    int header::int_value() {
        if (!this->_parsed) {
            this->_int = parse_int(this->str());
            this->_parsed = true;
        }

        return this->_int;
    }

    std::vector<std::string> header::list() const {
        return this->_list;
    }

    std::string request::method() const {
        return this->_method;
    }

    url::param::map request::params() {
        return this->_params;
    }

    size_t error::status() const {
        return this->_status;
    }

    std::string header::str() const {
        return this->_str;
    }

    std::string request::url() const {
        return this->_url;
    }

    const char* error::what() const throw() {
        return this->_what.c_str();
    }

    // Non-Member Functions

    std::map<int, std::string> error_codes() {
        return _error_codes;
    }

    request parse_request(const std::string message) {
#if LOGGING == LEVEL_DEBUG
        std::cout << message << std::endl;
#endif

        std::istringstream iss(message);
        std::string        str;

        if (!getline(iss, str))
            throw http::error("Empty message");

        std::vector<std::string> tokens = ::tokens(str);

        if (!(tokens.size() == 3 && tokens[2] == HTTP_VERSION))
            throw http::error("Start line is required");

        std::string method = tolowerstr(tokens[0]);
        std::string target = tokens[1];
        header::map headers;

        while (getline(iss, str)) {
            std::vector<std::string> header = split(str, ":");

            if (header.size() == 1)
                break;

            headers[tolowerstr(header[0])] = trim(str.substr(header[0].length() + 1));
        }

        int         content_length = headers["content-length"];
        std::string body = "";

        if (content_length != INT_MIN) {
            std::vector<std::string> value;

            while (getline(iss, str))
                value.push_back(trim_end(str));

            body = join(value, "\r\n");
            body = body.substr(0, std::min((int)body.length(), content_length));
        }

        return request(method, url(target), headers, body);
    }

    std::string redirect(header::map& headers, const size_t status, const std::string location) {
        headers["Location"] = location;

        std::string status_text = REDIRECT_CODES[(int)status];

        return response(status, status_text, status_text + ". Redirecting to " + location, headers);
    }

    std::string redirect(header::map& headers, const std::string location) {
        return redirect(headers, 302, location);
    }

    std::string response(const std::string text, header::map headers) {
        return response(200, "OK", text, headers);
    }

    std::string response(const size_t status, const std::string status_text, const std::string text, header::map headers) {
        std::ostringstream oss(HTTP_VERSION + " ");

        oss.seekp(0, std::ios::end);

        oss << std::to_string(status) << " " << status_text << "\r\n";

        // Response headers
        time_t now = time(0);
        tm*    gmtm = gmtime(&now);
        char*  dt = asctime(gmtm);
        
        std::vector<std::string> tokens = ::tokens(std::string(dt));

        oss << "Date" << ": ";

        std::string day = tokens[0],
                    month = tokens[1],
                    date = tokens[2],
                    time = tokens[3],
                    year = tokens[4];

        oss << day << ", " << date << " " << month << " " << year << " " << time << " GMT\r\n";

        for (const auto& [key, value]: headers)
            oss << key << ": " << value.str() << "\r\n";
        
        if (text.length()) {
            oss << "Content-Length: " << text.length() << "\r\n";
            oss << "\r\n";
            oss << text << "\r\n";
        }
        
        return oss.str();
    }

    size_t& timeout() {
        return _timeout;
    }
}
