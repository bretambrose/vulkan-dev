#pragma once

#include <ip/core/memory/Memory.h>
#include <ip/core/logging/BackgroundLogger.h>
#include <ip/core/logging/LogScope.h>
#include <ip/core/logging/RollingFileLogger.h>
#include <ip/core/logging/StandardLogLineFormatter.h>
#include <ip/core/utils/SystemUtils.h>

#include <chrono>

#define DECLARE_BACKGROUND_FILE_LOGGER(variableName, filenamePrefix, directory) \
    IP::Logging::LogScope variableName(std::move(IP::MakeUnique<IP::Logging::BackgroundLogger>(MEMORY_TAG, \
        [](){ return IP::MakeUnique<IP::Logging::RollingFileLogger>(MEMORY_TAG, \
            IP::MakeUnique<IP::Logging::StandardLogLineFormatter>(MEMORY_TAG), \
            filenamePrefix, \
            directory); })));

