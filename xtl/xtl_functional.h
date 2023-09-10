/// @file
/// @brief  xtl::functional
/// @author (C) 2021 ttsuki
/// Distributed under the Boost Software License, Version 1.0.

#pragma once

#include <utility>
#include <type_traits>

namespace xtl
{
    template <class T, class TResult, class... TArgs>
    static inline constexpr auto bind_this(T* this_pointer, TResult (T::* member_function_pointer)(TArgs... args)) noexcept
    {
        return [this_pointer, member_function_pointer](TArgs... args) -> TResult
        {
            return (this_pointer->*member_function_pointer)(std::forward<TArgs>(args)...);
        };
    }

    /// Provides fixed point combinator
    template <class F>
    class fixed_point_combinator final : F
    {
    public:
        template <class TF> constexpr fixed_point_combinator(TF&& f) : F(std::forward<TF>(f)) { }
        fixed_point_combinator(const fixed_point_combinator& other) = delete;
        fixed_point_combinator(fixed_point_combinator&& other) noexcept = delete;
        fixed_point_combinator& operator=(const fixed_point_combinator& other) = delete;
        fixed_point_combinator& operator=(fixed_point_combinator&& other) noexcept = delete;
        ~fixed_point_combinator() = default;

        template <typename... TArgs>
        constexpr decltype(auto) operator()(TArgs&&... args) const
        noexcept(noexcept(F::operator()(std::declval<fixed_point_combinator>(), std::declval<TArgs>()...)))
        {
            return F::operator()(*this, std::forward<TArgs>(args)...);
        }
    };

    /// Wraps lambda function F by a fixed point combinator.
    ///
    /// usage:
    /// <pre>
    ///   // Defining recursive function lambda.
    ///   auto fact = xtl::with_fixed(
    ///     [](auto&& fact, int x) {
    ///       return x == 0 ? 1 : x * fact(x-1); // `f` is `fact` itself.
    ///     } 
    ///   );
    ///
    /// usage:
    /// <pre>
    ///   // Calling recursively unnamed lambda.
    ///   auto fibonacci_of_5 = xtl::with_fixed([](auto&& f, int x) { return x == 0 || x == 1 ? x : f(x-2) + f(x-1); })(5);
    /// </pre>
    ///
    template <class F>
    [[nodiscard]] static inline constexpr auto with_fixed(F&& f)
    {
        return fixed_point_combinator<std::decay_t<F>>(std::forward<std::decay_t<F>>(f));
    }
}
