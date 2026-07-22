#pragma once
#include "constants.hpp"
#include "formatters.hpp" // IWYU pragma: keep (needed for the formatters to work)
#include <cstdlib>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/core/LogLevel.h>
#include <quill/sinks/ConsoleSink.h>
#include <utility>

namespace mycraft_vulkan {
inline auto global_logger() -> quill::Logger * {
    static auto *logger = [] {
        quill::Backend::start();
        auto sink_config = quill::ConsoleSinkConfig();
        sink_config.set_stream("stderr");
        auto sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("default-sink", sink_config);
        auto log_level = log_level_env_var_to_quill_log_level(std::getenv(LOG_LEVEL_ENV_VAR_NAME), quill::LogLevel::Info);
        auto *created_logger = quill::Frontend::create_or_get_logger("default-logger", std::move(sink));
        created_logger->set_log_level(log_level);
        return created_logger;
    }();
    return logger;
}
} // namespace mycraft_vulkan

#define LOG_CRITICAL(fmt, ...) QUILL_LOG_CRITICAL(global_logger(), fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) QUILL_LOG_ERROR(global_logger(), fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) QUILL_LOG_WARNING(global_logger(), fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) QUILL_LOG_INFO(global_logger(), fmt, ##__VA_ARGS__)
#define LOG_NOTICE(fmt, ...) QUILL_LOG_NOTICE(global_logger(), fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) QUILL_LOG_DEBUG(global_logger(), fmt, ##__VA_ARGS__)
#define LOG_TRACE_L1(fmt, ...) QUILL_LOG_TRACE_L1(global_logger(), fmt, ##__VA_ARGS__)
#define LOG_TRACE_L2(fmt, ...) QUILL_LOG_TRACE_L2(global_logger(), fmt, ##__VA_ARGS__)
#define LOG_TRACE_L3(fmt, ...) QUILL_LOG_TRACE_L3(global_logger(), fmt, ##__VA_ARGS__)
