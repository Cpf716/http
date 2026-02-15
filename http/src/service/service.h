//
//  service.h
//  http
//
//  Created by Corey Ferguson on 2/12/26.
//

#ifndef service_h
#define service_h

#include "http.h"
#include "logger.h"

using namespace http;
using namespace std;

struct service {
    string greeting(header::map headers, class request request);
    
    string ping(header::map headers);
};

#endif /* service_h */
