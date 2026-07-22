#pragma once
#include "vulkan/vulkan.hpp"
#include <cstring>
#include <quill/core/LogLevel.h>
#include <utility>

namespace mycraft_vulkan {
constexpr auto LOG_LEVEL_ENV_VAR_NAME = "LOG_LEVEL";
constexpr auto LOG_LEVEL_CRITICAL_ENV_VAR_VALUE = "critical";
constexpr auto LOG_LEVEL_ERROR_ENV_VAR_VALUE = "error";
constexpr auto LOG_LEVEL_WARNING_ENV_VAR_VALUE = "warning";
constexpr auto LOG_LEVEL_INFO_ENV_VAR_VALUE = "info";
constexpr auto LOG_LEVEL_NOTICE_ENV_VAR_VALUE = "notice";
constexpr auto LOG_LEVEL_DEBUG_ENV_VAR_VALUE = "debug";
constexpr auto LOG_LEVEL_TRACE_L1_ENV_VAR_VALUE = "trace_l1";
constexpr auto LOG_LEVEL_TRACE_L2_ENV_VAR_VALUE = "trace_l2";
constexpr auto LOG_LEVEL_TRACE_L3_ENV_VAR_VALUE = "trace_l3";
constexpr auto LOG_LEVEL_OFF_ENV_VAR_VALUE = "off";

constexpr auto log_level_env_var_to_quill_log_level(const char *log_level_env_var, quill::LogLevel default_if_null = quill::LogLevel::None)
    -> quill::LogLevel {
    if (!log_level_env_var) {
        return default_if_null;
    }
    if (std::strcmp(log_level_env_var, LOG_LEVEL_CRITICAL_ENV_VAR_VALUE) == 0) {
        return quill::LogLevel::Critical;
    }
    if (std::strcmp(log_level_env_var, LOG_LEVEL_ERROR_ENV_VAR_VALUE) == 0) {
        return quill::LogLevel::Error;
    }
    if (std::strcmp(log_level_env_var, LOG_LEVEL_WARNING_ENV_VAR_VALUE) == 0) {
        return quill::LogLevel::Warning;
    }
    if (std::strcmp(log_level_env_var, LOG_LEVEL_INFO_ENV_VAR_VALUE) == 0) {
        return quill::LogLevel::Info;
    }
    if (std::strcmp(log_level_env_var, LOG_LEVEL_NOTICE_ENV_VAR_VALUE) == 0) {
        return quill::LogLevel::Notice;
    }
    if (std::strcmp(log_level_env_var, LOG_LEVEL_DEBUG_ENV_VAR_VALUE) == 0) {
        return quill::LogLevel::Debug;
    }
    if (std::strcmp(log_level_env_var, LOG_LEVEL_TRACE_L1_ENV_VAR_VALUE) == 0) {
        return quill::LogLevel::TraceL1;
    }
    if (std::strcmp(log_level_env_var, LOG_LEVEL_TRACE_L2_ENV_VAR_VALUE) == 0) {
        return quill::LogLevel::TraceL2;
    }
    if (std::strcmp(log_level_env_var, LOG_LEVEL_TRACE_L3_ENV_VAR_VALUE) == 0) {
        return quill::LogLevel::TraceL3;
    }
    return quill::LogLevel::None;
}

constexpr auto log_level_env_var_to_vk_debug_utils_message_serverity_flags(const char *log_level_env_var,
                                                                           vk::DebugUtilsMessageSeverityFlagsEXT default_if_null = {})
    -> vk::DebugUtilsMessageSeverityFlagsEXT {
    using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    if (!log_level_env_var) {
        return default_if_null;
    }
    const auto log_level = log_level_env_var_to_quill_log_level(log_level_env_var);
    switch (log_level) { // NOLINT(switch)
    case quill::LogLevel::TraceL3:
    case quill::LogLevel::TraceL2:
    case quill::LogLevel::TraceL1:
    case quill::LogLevel::Debug:
        return Severity::eVerbose | Severity::eInfo | Severity::eWarning | Severity::eError;
    case quill::LogLevel::Info:
    case quill::LogLevel::Notice:
        return Severity::eInfo | Severity::eWarning | Severity::eError;
    case quill::LogLevel::Warning:
        return Severity::eWarning | Severity::eError;
    case quill::LogLevel::Error:
        return Severity::eError;
    case quill::LogLevel::Critical:
    case quill::LogLevel::None:
        return {};
    case quill::LogLevel::Backtrace:
        // log_level_env_var_to_quill_log_level never returns quill::LogLevel::Backtrace.
        std::unreachable();
    }
    return {};
}
} // namespace mycraft_vulkan
