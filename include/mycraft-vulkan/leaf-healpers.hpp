#pragma once
#include "logger.hpp"
#include <boost/leaf/diagnostics.hpp>
#include <boost/leaf/result.hpp>
#include <exception>
#include <sstream>
#include <type_traits>

namespace mycraft_vulkan {
/// @brief Wrapper of @ref boost::leaf::try_handle_all that always appends a
/// handler that accepts a @ref boost::leaf::diagnostic_details that logs the
/// error and terminates the app. This is so that you don't have to come up with
/// a catch-all handler if you are sure you have handled all errors.
template <class TryBlock, class... H>
auto try_handle_all_relaxed(TryBlock &&try_block, H &&...h) -> std::decay_t<decltype(std::declval<TryBlock>()().value())> {
    return boost::leaf::try_handle_all(
        std::forward<TryBlock>(try_block), std::forward<H>(h)...,
        [](const boost::leaf::diagnostic_details &details) -> std::decay_t<decltype(std::declval<TryBlock>()().value())> {
            auto out = std::ostringstream();
            out << details;
            LOG_ERROR("Unhandled error! Error: {}", out.str());
            std::terminate();
        });
}

/// @brief For use in @ref BOOST_LEAF_AUTO like `BOOST_LEAF_AUTO(value,
/// add_error(get_value(), MyError))`. This makes @ref BOOST_LEAF_AUTO return
/// `value` if `get_value` is successful, or return the error that `get_value`
/// returned with `MyError` attached.
template <class T, class... E> auto add_error(boost::leaf::result<T> r, E &&...e) -> boost::leaf::result<T> {
    if (!r) {
        return r.load(std::forward<E>(e)...);
    }
    return r;
}
} // namespace mycraft_vulkan
