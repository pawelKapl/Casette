#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <functional>
#include <unistd.h>
#include <cmath>
#include <atomic>
#include <optional>
#include <filesystem>
#include "json.hpp"
#include <stdexcept>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Initializers/RollingFileInitializer.h>

#define j_int uint32_t

#define log_debug PLOG_DEBUG
#define log_info PLOG_INFO
#define log_warn PLOG_WARNING
#define log_error PLOG_ERROR
#define log_fatal PLOG_FATAL

void initLogger();

