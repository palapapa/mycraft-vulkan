#pragma once
#include "CLI/CLI.hpp"
#include "formatters.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <quill/core/LogLevel.h>
#include <string>
#include <unordered_map>
#include <utility>

namespace mycraft_vulkan {
constexpr auto LOG_LEVEL_OPTION_NAME = "LOG_LEVEL";
constexpr auto LOG_LEVEL_CRITICAL_OPTION_VALUE = "critical";
constexpr auto LOG_LEVEL_ERROR_OPTION_VALUE = "error";
constexpr auto LOG_LEVEL_WARNING_OPTION_VALUE = "warning";
constexpr auto LOG_LEVEL_INFO_OPTION_VALUE = "info";
constexpr auto LOG_LEVEL_NOTICE_OPTION_VALUE = "notice";
constexpr auto LOG_LEVEL_DEBUG_OPTION_VALUE = "debug";
constexpr auto LOG_LEVEL_TRACE_L1_OPTION_VALUE = "tracel1";
constexpr auto LOG_LEVEL_TRACE_L2_OPTION_VALUE = "tracel2";
constexpr auto LOG_LEVEL_TRACE_L3_OPTION_VALUE = "tracel3";
constexpr auto LOG_LEVEL_OFF_OPTION_VALUE = "off";
const auto LOG_LEVEL_OPTION_VALUE_TO_QUILL_LOG_LEVEL = std::unordered_map<std::string, quill::LogLevel>{
    {LOG_LEVEL_CRITICAL_OPTION_VALUE, quill::LogLevel::Critical}, {LOG_LEVEL_ERROR_OPTION_VALUE, quill::LogLevel::Error},
    {LOG_LEVEL_WARNING_OPTION_VALUE, quill::LogLevel::Warning},   {LOG_LEVEL_INFO_OPTION_VALUE, quill::LogLevel::Info},
    {LOG_LEVEL_NOTICE_OPTION_VALUE, quill::LogLevel::Notice},     {LOG_LEVEL_DEBUG_OPTION_VALUE, quill::LogLevel::Debug},
    {LOG_LEVEL_TRACE_L1_OPTION_VALUE, quill::LogLevel::TraceL1},  {LOG_LEVEL_TRACE_L2_OPTION_VALUE, quill::LogLevel::TraceL2},
    {LOG_LEVEL_TRACE_L3_OPTION_VALUE, quill::LogLevel::TraceL3},  {LOG_LEVEL_OFF_OPTION_VALUE, quill::LogLevel::None},
};

constexpr auto log_level_to_vk_debug_utils_message_serverity_flags(quill::LogLevel log_level) -> vk::DebugUtilsMessageSeverityFlagsEXT {
    using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
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
}

struct Config {
    quill::LogLevel log_level;
};

inline auto parse_arguments(int argc, char *argv[]) -> Config { // NOLINT(modernize-avoid-c-arrays)
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic): Checked bounds.
    const auto executable_name =
        argc > 0 && argv[0] != nullptr && argv[0][0] != '\0' ? std::filesystem::path(argv[0]).filename().string() : std::string("mycraft-vulkan");
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto argument_parser = CLI::App("palapapa's 3D engine in Vulkan", executable_name);
    argv = argument_parser.ensure_utf8(argv);
    argument_parser.formatter(std::make_shared<Cli11Formatter>());
    auto log_level = quill::LogLevel();
    argument_parser.add_option("-l,--log-level", log_level, "The log level to use.")
        ->type_name("LEVEL")
        ->transform(checked_transformer_with_sorted_keys(LOG_LEVEL_OPTION_VALUE_TO_QUILL_LOG_LEVEL, CLI::ignore_case))
        ->default_val(LOG_LEVEL_INFO_OPTION_VALUE);
    try {
        argument_parser.parse(argc, argv);
    }
    catch (const CLI::ParseError &e) {
        std::exit(argument_parser.exit(e));
    }
    return {.log_level = log_level};
}
} // namespace mycraft_vulkan
