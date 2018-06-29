#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

/// @var This variable contains a logger object. Please see the spdlog documentation for more information on it's usage
extern std::shared_ptr<spdlog::logger> program_log;

#endif