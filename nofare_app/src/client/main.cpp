
#include <iostream>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <AddLabelCommand.h>
#include <ShutdownCommand.h>
#include <ReclassifyCommand.h>
#include "nofare_definitions.h"

inline void printUsage(int argc, char *argv[]) {
    std::cout << "usage: " << argv[0] << " <command> [args..]\n";
    std::cout << "available commands:\n ";
    std::cout << "    add-label <event id> <label>\n ";
    std::cout << "    reclassify <event id>\n ";
    std::cout << "    shutdown\n ";


}

int addLabel(boost::interprocess::message_queue &queue, int argc, char **argv);

int shutdownCommand(boost::interprocess::message_queue &queue, int argc, char **argv);
int reclassifyCommand(boost::interprocess::message_queue &queue, int argc, char **argv);


int main(int argc, char *argv[]) {
    using namespace boost::interprocess;
    if (argc < 2) {
        printUsage(argc, argv);
        return -1;
    }

    try {
        message_queue queue(open_only, MESSAGE_QUEUE_NAME);
        std::string command(argv[1]);
        if (command == "add-label")
            return addLabel(queue, argc, argv);
        else if (command == "shutdown")
            return shutdownCommand(queue, argc, argv);
        else if (command == "reclassify")
            return reclassifyCommand(queue, argc, argv);
        else {
            std::cerr << "Unknown command " << argv[1] << "\n";
            return -1;
        }

    } catch (...) {
        std::cerr << "Could not send add the label. Is the server running?\n";
        return -2;
    }
    return 0;
}

int addLabel(boost::interprocess::message_queue &queue, int argc, char **argv) {
    if (argc < 4) {
        std::cerr << "not enough operands for this command\n";
        printUsage(argc, argv);
        return -1;
    }
    auto message = AddLabelCommand(std::stoll(argv[2]), std::stod(argv[3])).serialize();
    if (message.size() > MAX_MESSAGE_SIZE) {
        std::cout << "parameters too long\n";
        return -1;
    }
    queue.send(message.c_str(), message.size(), 1);

    return 0;
}

int shutdownCommand(boost::interprocess::message_queue &queue, int argc, char **argv) {
    auto message = ShutdownCommand().serialize();
    queue.send(message.c_str(), MAX_MESSAGE_SIZE, 1);
    return 0;
}

int reclassifyCommand(boost::interprocess::message_queue &queue, int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "not enough operands for this command\n";
        printUsage(argc, argv);
        return -1;
    }
    auto message = ReclassifyCommand(std::stoull(argv[2])).serialize();
    if (message.size() > MAX_MESSAGE_SIZE) {
        std::cout << "parameters too long\n";
        return -1;
    }
    queue.send(message.c_str(), message.size(), 1);

    return 0;
}
