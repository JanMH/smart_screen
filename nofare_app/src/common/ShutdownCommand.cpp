
#include "ShutdownCommand.h"

bool ShutdownCommand::isShutDownCommand(const std::string &str) {
    const std::string shutdown = ShutdownCommand::commandPrefix();
    return str.substr(0,shutdown.size()) == shutdown;
}

std::string ShutdownCommand::serialize() {
    return ShutdownCommand::commandPrefix();
}

std::string ShutdownCommand::commandPrefix() {
    return  "shutdown:";
}
