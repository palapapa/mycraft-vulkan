#pragma once
#include <VkBootstrap.h>
#include <cstddef>
#include <magic_enum/magic_enum_iostream.hpp>
#include <ostream>
#include <quill/HelperMacros.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_to_string.hpp>

namespace vk {
/// @brief When a @ref vk::Result is logged through quill, this converts the
/// error code to a string representation automatically.
inline auto operator<<(std::ostream &out, Result result) -> std::ostream & {
    return out << to_string(result);
}
} // namespace vk

/// @brief When a raw @ref VkResult is logged through quill, this converts the
/// error code to a string representation automatically.
inline auto operator<<(std::ostream &out, VkResult result) -> std::ostream & {
    return out << vk::to_string(static_cast<vk::Result>(result));
}

namespace vkb {
inline auto operator<<(std::ostream &out, Error const &error) -> std::ostream & {
    out << error.type.message();
    if (error.vk_result != VK_SUCCESS) {
        out << " (VkResult: " << vk::to_string(static_cast<vk::Result>(error.vk_result)) << ')';
    }
    if (!error.detailed_failure_reasons.empty()) {
        out << " [";
        for (size_t i = 0; i < error.detailed_failure_reasons.size(); ++i) {
            if (i != 0) {
                out << "; ";
            }
            out << error.detailed_failure_reasons[i];
        }
        out << ']';
    }
    return out;
}
} // namespace vkb

QUILL_LOGGABLE_DEFERRED_FORMAT(vk::Result)
QUILL_LOGGABLE_DEFERRED_FORMAT(VkResult)
QUILL_LOGGABLE_DEFERRED_FORMAT(vkb::Error)

namespace mycraft_vulkan {
// Makes all enums printable with their variant names instead of raw values.
using magic_enum::iostream_operators::operator<<;
} // namespace mycraft_vulkan
