//
//  http.h
//  socket
//
//  Created by Corey Ferguson on 9/24/25.
//

#ifndef http_h
#define http_h

#include "url.h"
#include "util.h"
#include <cmath>

namespace http {
    // Typedef

    struct error: public std::exception {
        // Constructors

        error(const std::string what);

        error(const size_t status, const std::string what);

        // Member Fields

        size_t      status() const;

        const char* what() const throw();
    private:
        // Member Fields

        size_t      _status;

        std::string _what;
    };

    struct header {
        // Typedef

        using map = std::map<std::string, header>;

        // Constructors

        header();

        header(const char* value);

        header(const int value);

        header(const std::string value);

        header(const std::vector<std::string> value);

        // Operators

        operator                 int();

        operator                 std::string();

        operator                 std::vector<std::string>();

        int                      operator=(const int value);

        std::string              operator=(const std::string value);

        std::vector<std::string> operator=(const std::vector<std::string> value);

        bool                     operator==(const char* value);

        bool                     operator==(const int value);

        bool                     operator==(const std::string value);

        bool                     operator==(const header value);

        bool                     operator!=(const char* value);

        bool                     operator!=(const int value);

        bool                     operator!=(const std::string value);

        bool                     operator!=(const header value);

        // Member Functions

        int                      int_value();

        std::vector<std::string> list() const;

        std::string              str() const;
    private:
        // Member Fields

        int                      _int;
        std::vector<std::string> _list;
        bool                     _parsed = false;
        std::string              _str;

        // Member Functions

        int                      _set(const int value);

        std::string              _set(const std::string value);

        std::vector<std::string> _set(const std::vector<std::string> value);
    };

    class request {
        // Member Fields

        std::string     _body;
        header::map     _headers;
        std::string     _method;
        url::param::map _params;
        std::string     _url;

        // Constructors

        request(const std::string method, class url url, header::map headers, const std::string body = "");

        // Non-Member Functions

        friend request parse_request(const std::string text);
    public:
        // Member Functions

        std::string     body() const;

        header::map     headers();

        std::string     method() const;

        url::param::map params();

        std::string     url() const;
    };

    // Non-Member Functions

    std::map<int, std::string> error_codes();

    request                    parse_request(const std::string text);

    std::string                redirect(header::map& headers, const std::string location);

    std::string                redirect(header::map& headers, const size_t status, const std::string location);

    std::string                response(const std::string text, header::map headers);

    std::string                response(const size_t status, const std::string status_text, const std::string text, header::map headers);

    size_t&                    timeout();
}

#endif /* http_h */
