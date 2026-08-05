#pragma once
#include "CLI/CLI.hpp"
#include <VkBootstrap.h>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <magic_enum/magic_enum_iostream.hpp>
#include <ostream>
#include <quill/HelperMacros.h>
#include <ranges>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_to_string.hpp>

namespace vk {
/// @brief When a `vk::Result` is logged through quill, this converts the error
/// code to a string representation automatically.
inline auto operator<<(std::ostream &out, Result result) -> std::ostream & {
    return out << to_string(result);
}
} // namespace vk

/// @brief When a raw `VkResult` is logged through quill, this converts the
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

/// @brief Custom CLI11 help message formatter that prints the default value
/// more clearly.
class Cli11Formatter : public CLI::Formatter {
  public:
    auto make_option_opts(const CLI::Option *opt) const -> std::string override {
        std::stringstream out;
        if (!opt->get_option_text().empty()) {
            out << " " << opt->get_option_text();
        }
        else {
            if (opt->get_type_size() != 0) {
                if (enable_option_type_names_) {
                    if (!opt->get_type_name().empty()) {
                        out << " " << get_label(opt->get_type_name());
                    }
                }
                if (enable_option_defaults_) {
                    if (!opt->get_default_str().empty()) {
                        out << " (default: " << opt->get_default_str() << ") ";
                    }
                }
                if (opt->get_expected_max() == CLI::detail::expected_max_vector_size) {
                    out << " ...";
                }
                else if (opt->get_expected_min() > 1) {
                    out << " x " << opt->get_expected();
                }
                if (opt->get_required()) {
                    out << " " << get_label("REQUIRED");
                }
            }
            if (!opt->get_envname().empty()) {
                out << " (" << get_label("Env") << ":" << opt->get_envname() << ")";
            }
            if (!opt->get_needs().empty()) {
                out << " " << get_label("Needs") << ":";
                for (const CLI::Option *op : opt->get_needs()) {
                    out << " " << op->get_name();
                }
            }
            if (!opt->get_excludes().empty()) {
                out << " " << get_label("Excludes") << ":";
                for (const CLI::Option *op : opt->get_excludes()) {
                    out << " " << op->get_name();
                }
            }
        }
        return out.str();
    }
};

/// @brief Returns a `CLI::CheckedTransformer` with its
/// `CLI::CheckedTransformer::description` called and set to a more readable
/// form than the default. This is used to fix
/// https://github.com/CLIUtils/CLI11/issues/554, since the default help message
/// generated with `CLI::CheckedTransformer` is quite unreadable. The generated
/// help message displays the allowed keys in `mapping`, sorted.
///
/// @tparam Mapping An iterable with tuple-like elements. Must be an input range
/// whose 0-th elements of its tuple-like elements must be able to be used to
/// construct a `std::string`.
///
/// @tparam Filters The filter type as passed to the second constructor argument
/// of `CLI::CheckedTransformer`.
///
/// @param mapping The mapping that should be passed the the first constructor
/// argument of `CLI::CheckedTransformer`.
///
/// @param filters The filters that should be passed the the second constructor
/// argument of `CLI::CheckedTransformer`.
///
/// @return A `CLI::CheckedTransformer` with modified description.
template <typename Mapping, typename... Filters>
    requires std::ranges::input_range<Mapping> && requires(const std::remove_reference_t<std::ranges::range_reference_t<Mapping>> &mapping_entry) {
        requires std::constructible_from<std::string, decltype(std::get<0>(mapping_entry))>;
    }
auto checked_transformer_with_sorted_keys(Mapping &&mapping, Filters &&...filters) -> CLI::CheckedTransformer {
    auto keys = std::vector<std::string>();
    if constexpr (std::ranges::sized_range<Mapping>) {
        keys.reserve(std::ranges::size(mapping));
    }
    for (const auto &entry : mapping) {
        keys.emplace_back(std::get<0>(entry));
    }
    std::ranges::sort(keys);
    const auto duplicates = std::ranges::unique(keys);
    keys.erase(duplicates.begin(), duplicates.end());
    auto description = std::string("One of {");
    for (std::size_t index = 0; index < keys.size(); ++index) {
        if (index != 0) {
            description += ", ";
        }
        description += keys[index];
    }
    description += '}';
    auto transformer = CLI::CheckedTransformer(std::forward<Mapping>(mapping), std::forward<Filters>(filters)...);
    transformer.description(std::move(description));
    return transformer;
}
} // namespace mycraft_vulkan
