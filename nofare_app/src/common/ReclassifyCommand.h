
#ifndef SMART_SCREEN_RECLASSIFYCOMMAND_H
#define SMART_SCREEN_RECLASSIFYCOMMAND_H
#include <string>


class ReclassifyCommand {
public:
    using EventIdType = unsigned long long;

    ReclassifyCommand() = default;

    ReclassifyCommand(EventIdType init_event_id)
            : event_id(init_event_id){}


    static bool isReclassifyCommand(const std::string &str);

    std::string serialize();

    static ReclassifyCommand deserialize(const std::string &std);

    EventIdType event_id;
private:
    static std::string commandPrefix();
};


#endif //SMART_SCREEN_RECLASSIFYCOMMAND_H
