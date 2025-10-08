//
//  http.h
//  http
//
//  Created by Corey Ferguson on 9/24/25.
//

#ifndef http_h
#define http_h

#include "url.h"
#include "util.h"
#include <cmath>
#include <set>

namespace http {
    // Typedef

    struct error: public std::exception {
        // Constructors

        error(const size_t status);

        error(const size_t status, const std::string text);

        // Member Fields

        size_t      status() const;

        std::string status_text() const;

        std::string text() const;

        const char* what() const throw();
    private:
        // Member Fields

        size_t      _status;
        std::string _status_text;
        std::string _text;
    };

    struct header {
        // Typedef

        using map = std::map<std::string, header>;

        // Constructors

        header();

        header(const char* value);

        header(const int value);

        header(const std::string value);

        header(std::set<std::string> value);

        // Operators

        operator              int();

        operator              std::string();

        operator              std::set<std::string>();

        int                   operator=(const int value);

        std::string           operator=(const std::string value);

        std::set<std::string> operator=(std::set<std::string> value);

        bool                  operator==(const char* value);

        bool                  operator==(const int value);

        bool                  operator==(const std::string value);

        bool                  operator==(const header value);

        bool                  operator!=(const char* value);

        bool                  operator!=(const int value);

        bool                  operator!=(const std::string value);

        bool                  operator!=(const header value);

        // Member Functions

        int                   int_value() const;

        std::set<std::string> list();

        std::string           str() const;
    private:
        // Member Fields

        int                   _int;
        std::set<std::string> _list;
        std::string           _str;

        // Member Functions

        int                   _set(const int value);

        std::string           _set(const std::string value);

        std::set<std::string> _set(std::set<std::string> value);
    };

    struct request {
        // Constructors

        request(const std::string method, const std::string url, header::map headers = {}, const std::string body = "");

        // Member Functions

        std::string     body() const;

        header::map     headers();

        std::string     method() const;

        url::param::map params();

        std::string     url() const;
    private:
        // Member Fields

        std::string     _body;
        header::map     _headers;
        std::string     _method;
        url::param::map _params;
        std::string     _url;
    };

    // Non-Member Functions

    std::string http_version();

    request     parse_request(const std::string text);

    std::string redirect(header::map& headers, const std::string location);

    std::string redirect(header::map& headers, const size_t status, const std::string location);

    std::string response(const std::string text, header::map headers);

    std::string response(const size_t status, const std::string status_text, const std::string text, header::map headers, const bool date = true);

    size_t      timeout();
}

#endif /* http_h */
