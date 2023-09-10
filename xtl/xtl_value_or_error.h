/// @file
/// @brief  xtl::value_or_error
/// @author (C) 2021 ttsuki
/// Distributed under the Boost Software License, Version 1.0.

#pragma once

#include <variant>

namespace xtl
{
    /// Represents a value or a error.
    template <class TValue, class TError>
    class value_or_error
    {
        std::variant<std::nullptr_t, TValue, TError> var_;

    public:
        /// tags for dispatch
        template <size_t TIndex>
        struct tag : std::integral_constant<size_t, TIndex>
        {
        };

        using holds_value = tag<1>;
        using holds_error = tag<2>;

    public:
        value_or_error() = default;

        value_or_error(TValue value, holds_value = {})
            : var_(std::forward<TValue>(value))
        {
        }

        value_or_error(TError error, holds_error = {})
            : var_(std::forward<TError>(error))
        {
        }

        [[nodiscard]] bool has_value() const noexcept { return var_.index() == holds_value::value; }
        [[nodiscard]] TValue& value() { return std::get<holds_value::value>(var_); }
        [[nodiscard]] const TValue& value() const { return std::get<holds_value::value>(var_); }

        [[nodiscard]] bool has_error() const noexcept { return var_.index() == holds_error::value; }
        [[nodiscard]] TError& error() { return std::get<holds_error::value>(var_); }
        [[nodiscard]] const TError& error() const { return std::get<holds_error::value>(var_); }

        [[nodiscard]] explicit operator bool() const { return has_value(); }
        [[nodiscard]] TValue value_or(TValue value_if_error = {}) const { return has_value() ? value() : value_if_error; }

        TValue& operator *() { return value(); }
        const TValue& operator *() const { return value(); }

        TValue* operator ->() { return &value(); }
        const TValue* operator ->() const { return &value(); }
    };
}
