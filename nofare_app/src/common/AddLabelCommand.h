
#ifndef SMART_SCREEN_ADDLABELCOMMAND_H
#define SMART_SCREEN_ADDLABELCOMMAND_H

#include <string>

struct AddLabelCommand {

    using EventIdType = unsigned long long;
    using LabelType = double;



    AddLabelCommand() = default;

    AddLabelCommand(EventIdType init_event_id, LabelType init_label)
            : event_id(init_event_id), label(init_label) {}

    EventIdType event_id;
    LabelType label;


    static bool isAddLabelCommand(const std::string &str);

    std::string serialize();

    static AddLabelCommand deserialize(const std::string &std);


private:
    static std::string commandPrefix();
};


#endif //SMART_SCREEN_ADDLABELCOMMAND_H
