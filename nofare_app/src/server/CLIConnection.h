
#ifndef SMART_SCREEN_CLICONNECTION_H
#define SMART_SCREEN_CLICONNECTION_H


#include <boost/interprocess/ipc/message_queue.hpp>

void cleanUp(int signum);

bool setupMessageQueue();

bool askUserForQueueReset();

extern std::unique_ptr<boost::interprocess::message_queue> cli_message_queue;

#endif //SMART_SCREEN_CLICONNECTION_H
