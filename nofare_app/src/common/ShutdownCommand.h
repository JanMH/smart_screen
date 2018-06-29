
#ifndef SMART_SCREEN_SHUTDOWNCOMMAND_HPP
#define SMART_SCREEN_SHUTDOWNCOMMAND_HPP

#include <string>

class ShutdownCommand {
public:
    static bool isShutDownCommand(const std::string& str);


    std::string serialize();

private:
    static std::string commandPrefix();

};


#endif //SMART_SCREEN_SHUTDOWNCOMMAND_HPP
