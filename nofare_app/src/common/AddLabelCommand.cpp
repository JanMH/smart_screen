
#include "AddLabelCommand.h"
#include <sstream>
#include <cassert>

bool AddLabelCommand::isAddLabelCommand(const std::string &str) {
    return str.substr(0, AddLabelCommand::commandPrefix().size()) == commandPrefix();
}

std::string AddLabelCommand::commandPrefix() {
    return "add-label:";
}

std::string AddLabelCommand::serialize() {
    std::stringstream stringstream;
    stringstream << AddLabelCommand::commandPrefix();
    stringstream << this->event_id << "," << this->label;
    return stringstream.str();
}

AddLabelCommand AddLabelCommand::deserialize(const std::string &str) {
    assert(isAddLabelCommand(str));
    auto values = str.substr(commandPrefix().size());
    auto comma_pos = values.find(',');
    if (comma_pos == std::string::npos || comma_pos == values.length() -1) {
        throw -1;
    }
    AddLabelCommand result;
    result.event_id = std::stoul(values.substr(0,comma_pos));
    auto label_str = values.substr(comma_pos + 1);
    result.label = std::stod(label_str);
    return result;
}
