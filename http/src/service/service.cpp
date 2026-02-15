//
//  service.cpp
//  http
//
//  Created by Corey Ferguson on 2/12/26.
//

#include "service.h"

string service::greeting(header::map headers, class request request) {
    url host(request.headers()["host"]);

    return redirect(headers, PERMANENT_REDIRECT, "http://" + host.host() + ":" + to_string(((int) host.port()) + 1) + request.url());
}

string service::ping(header::map headers) {
    headers["Content-Type"] = string("text/plain; charset=utf-8");

    return response("Hello, world!", headers);
}
