//
//  main.cpp
//  http
//
//  Created by Corey Ferguson on 9/24/25.
//

#include "http.h"
#include "socket.h"
#include "url.h"

using namespace http;
using namespace mysocket;
using namespace std;

// Typedef

struct service {
    // Member Functions

    std::string greeting(header::map headers, class request request) {
        url host(request.headers()["host"]);

        // Permanent Redirect
        return redirect(headers, 308, "http://" + host.host() + ":" + std::to_string(host.port().value() + 1) + request.url());
    }

    std::string ping(header::map headers) {
        std::string body = "Hello, world!";

        headers["Content-Type"] = std::string("text/plain; charset=utf-8");

        return http::response(body, headers);
    }
};

// Non-Member Fields

const size_t  PORT = 8080;

tcp_server*   server = NULL;
class service service;

// Non-Member Functions

std::string handle_request(header::map headers, class request request) {
    // Optionally override default headers
    auto options = [](header::map headers) {
        headers["Allow"] = { "OPTIONS", "GET", "HEAD", "POST" };

        return http::response(204, "No Content", "", headers);
    };
    
    auto not_found = [request]() {
        throw http::error(404, "Cannot " + toupperstr(request.method()) + " " + request.url());
        return "";
    };

    if (request.url() == "/ping") {
        if (request.method() == "options")
            return options(headers);

        if (request.method() == "get")
            return service.ping(headers);

        return not_found();
    }
    
    if (request.url() == "/greeting") {
        if (request.method() == "options")
            return options(headers);

        if (request.method() == "post")
            return service.greeting(headers, request);

        return not_found();
    }

    return not_found();
}

void log_request(class request request) {
    cout << "url: " << request.url() << ", body: " << (request.body().empty() ? "null" : request.body()) << endl;
}

// Perform garbage collection
void onsignal(int signum) {
    server->close();
}

int main(int argc, const char* argv[]) {
    signal(SIGINT, onsignal);
    signal(SIGTERM, onsignal);

    server = new tcp_server(PORT, [](tcp_server::connection* connection) {
        std::atomic<bool> recved = false;
        
        thread([&recved, connection]() {
            try {
                size_t timeout = http::timeout();

                std::thread([timeout, &recved, connection]() {
                   for (size_t i = 0; i < timeout && !recved.load(); i++)
                       std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                   if (recved.load())
                       return;

                   recved.store(true);
                   connection->close();
                }).detach();

                // Should the server listen for additional requests?
                // while (true) {
                    try {
                        class request request = parse_request(connection->recv());

                        recved.store(true);

                        if (request.headers()["host"].str().length()) {
                            log_request(request);
                            
                            std::string response;
                            header::map headers = {
                                { "Access-Control-Allow-Origin", "*" },
                                { "Connection", "keep-alive" },
                                { "Keep-Alive", "timeout=5" }
                            };

                            try {
                                response = handle_request(headers, request);
                            } catch (http::error& e) {
                                response = http::response(e.status(), error_codes()[e.status()], e.what(), headers);
                            }

#if LOGGING == LEVEL_DEBUG
                            cout << response << endl;
#endif

                            connection->send(response);
                        }

                        connection->close();
                        // break;
                    } catch (http::error& e) { }
                // }
            } catch (mysocket::error& e) {
                 if (recved.load())
                    return;

                throw e;
            }
        }).detach();
    });

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true)
        continue;
}
