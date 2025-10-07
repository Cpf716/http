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

    string greeting(header::map headers, class request request) {
        url host(request.headers()["host"]);

        // Permanent Redirect
        return redirect(headers, 308, "http://" + host.host() + ":" + to_string(host.port().value() + 1) + request.url());
    }

    string ping(header::map headers) {
        string body = "Hello, world!";

        headers["Content-Type"] = string("text/plain; charset=utf-8");

        return response(body, headers);
    }
};

// Non-Member Fields

const size_t  PORT = 8080;

tcp_server*   server = NULL;
class service service;

// Non-Member Functions

// HTTP/1.1 default
size_t keep_alive_timeout() {
    return 5;
}

// Optional; assign a value < 0 to disable
int keep_alive_max() {
    return 200;
}

header::map headers() {
    // Default response headers
    header::map result = {
        { "Access-Control-Allow-Origin", "*" },
        { "Connection", "keep-alive" },
    };
    vector<string> keep_alive = { "timeout=" + to_string(keep_alive_timeout()) };

    if (keep_alive_max() >= 1)
        keep_alive.push_back("max=" + to_string(keep_alive_max()));

    result["Keep-Alive"] = keep_alive;

    return result;
}

string handle_request(header::map headers, class request request) {
    auto options = [](header::map headers) {
        headers["Access-Control-Allow-Origin"] = { "GET", "HEAD", "PUT", "PATCH", "POST", "DELETE" };

        return response(204, "No Content", "", headers);
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
    server = new tcp_server(PORT, [](tcp_server::connection* connection) {
        // Handle request in its own thread
        thread([connection]() {
            // Number of messages received
            atomic<int> recvc = 0;

            // Set connection timeout
            thread([&recvc, connection]() {
                size_t timeout = http::timeout();

                for (size_t i = 0; i < timeout && !recvc.load(); i++)
                    this_thread::sleep_for(chrono::milliseconds(1000));

                if (recvc.load())
                    return;

                recvc.store(1);
                connection->close();
            }).detach();

            
            // Wait for non-empty request
            while (true) {
                try {
                    string request = connection->recv();

                    if (request.empty())
                        continue;

                    recvc.fetch_add(1);

                    auto handle_response = [connection](const string response) {
#if LOGGING == LEVEL_DEBUG
                        cout << response << endl;
#endif

                        connection->send(response);
                    };

                    try {
                        class request request_obj = parse_request(request);

                        if (request_obj.headers()["host"].str().length()) {
                            log_request(request_obj);

                            try {
                                string response = handle_request(headers(), request_obj);

                                handle_response(response);

                                int last_id = recvc.load();

                                if (last_id >= keep_alive_max()) {
                                    connection->close();
                                    break;
                                }

                                // Keep alive
                                thread([&recvc, last_id, connection]() {
                                    for (int i = 0; i < keep_alive_timeout() && recvc.load() == last_id; i++) 
                                        this_thread::sleep_for(chrono::milliseconds(1000));

                                    if (recvc.load() == last_id)
                                        connection->close();
                                }).detach();
                            } catch (http::error& e) {
                                handle_response(response(e.status(), e.status_text(), e.text(), headers()));
                            }
                        } else {
                            handle_response(response(400, "Bad Request", to_string(0), {
                                { "Connection", "close" },
                                { "Transfer-Encoding", "chunked "}   
                            }));

                            connection->close();
                            break;
                        }
                    } catch (http::error& e) {
                        handle_response(response(400, "Bad Request", e.text(), {
                            { "Connection", "close" }
                        }, false));
                
                        connection->close();
                        break;
                    }
                } catch (mysocket::error& e) {
                    // Connection timed out; suppress error
                    if (recvc.load())
                        return;

                    throw e;
                }
            }
        }).detach();
    });

    signal(SIGINT, onsignal);
    signal(SIGTERM, onsignal);

    cout << "Server listening on port " << PORT << "...\n";

    while (true)
        continue;
}
