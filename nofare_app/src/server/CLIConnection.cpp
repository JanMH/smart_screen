
#include "CLIConnection.h"
#include <iostream>
#include "nofare_definitions.h"
#include <csignal>

std::unique_ptr<boost::interprocess::message_queue> cli_message_queue;

bool setupMessageQueue() {
    using namespace boost::interprocess;

    try {

        cli_message_queue = std::make_unique<message_queue>(create_only,
                                                            MESSAGE_QUEUE_NAME, MAX_MESSAGE_NUMBER, MAX_MESSAGE_SIZE);

    } catch (interprocess_exception &ex) {
        std::cout << ex.what() << "\n";
        if (askUserForQueueReset()) {
            message_queue::remove(MESSAGE_QUEUE_NAME);
            return setupMessageQueue();
        } else {
            return false;
        }
    }

    signal(SIGABRT, &cleanUp);
    signal(SIGTERM, &cleanUp);
    signal(SIGKILL, &cleanUp);
    return true;
}

bool askUserForQueueReset() {
    using namespace std;
    cout << "The server seems to be running already. If you are sure that it is not running, please type \"yes\"\n";
    std::string answer;
    cin >> answer;

    std::transform(answer.begin(), answer.end(), answer.begin(), ::tolower);

    return answer == "yes";
}




void cleanUp(int signum) {
    boost::interprocess::message_queue::remove(MESSAGE_QUEUE_NAME);
}
