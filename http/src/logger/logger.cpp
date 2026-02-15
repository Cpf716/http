//
//  logger.cpp
//  http
//
//  Created by Corey Ferguson on 2/12/26.
//

#include "logger.h"

namespace logger {
    void error(const std::string message) {
        info("Error - " + message);
    }

    void info(const std::string message) {
        std::cout << message << std::endl;
    }
}
