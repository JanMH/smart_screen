
#include "ReclassifyCommand.h"
#include <sstream>
#include <cassert>

bool ReclassifyCommand::isReclassifyCommand(const std::string &str) {
    return str.substr(0, ReclassifyCommand::commandPrefix().size()) == commandPrefix();
}

std::string ReclassifyCommand::commandPrefix() {
    return "reclassify:";
}

std::string ReclassifyCommand::serialize() {
    std::stringstream stringstream;
    stringstream << ReclassifyCommand::commandPrefix();
    stringstream << this->event_id;
    return stringstream.str();
}

ReclassifyCommand ReclassifyCommand::deserialize(const std::string &str) {
    assert(isReclassifyCommand(str));
    auto value = str.substr(commandPrefix().size());
    ReclassifyCommand result;
    result.event_id = std::stoul(value);
    return result;
}
