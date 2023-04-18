/// @file
/// @brief  xtl::lazy
/// @author ttsuki

#pragma once

#include <stdexcept>
#include <functional>
#include <variant>
#include <future>

#include "xtl_delegate.h"

namespace xtl
{
    template <class T>
    struct lazy
    {
        using empty_t = std::nullptr_t;
        using value_t = T;
        using factory_t = delegate<T()>;

        mutable std::variant<empty_t, value_t, factory_t, std::exception_ptr> instance;

        struct lazy_ctor_t
        {
            constexpr explicit lazy_ctor_t() = default;
        } static constexpr inline lazy_ctor = lazy_ctor_t{};

        lazy() = default;
        lazy(value_t value) : instance(std::move(value)) { }
        lazy(factory_t factory) : instance(std::move(factory)) { }
        lazy(std::function<T()> factory) : instance(factory_t(std::move(factory))) { }
        lazy(std::future<T> future) : lazy(factory_t([future = std::move(future)]() mutable { return future.get(); })) { }

        template <class... TArgs>
        lazy(lazy_ctor_t, TArgs... args) : lazy(factory_t([tpl = std::make_tuple(std::move(args)...)]() mutable
        {
            return std::apply([](auto&&... arg) { return value_t(std::forward<decltype(arg)>(arg)...); }, std::move(tpl));
        })) { }

        T* try_get() noexcept
        {
            if (std::holds_alternative<factory_t>(instance))
            {
                try { instance = std::get<factory_t>(instance)(); }
                catch (...) { instance = std::current_exception(); }
            }

            return std::get_if<value_t>(&instance);
        }

        T& get()
        {
            if (auto pointer = try_get()) { return *pointer; }
            if (std::holds_alternative<std::exception_ptr>(instance)) { std::rethrow_exception(std::get<std::exception_ptr>(instance)); }
            throw std::runtime_error("invalid instance status!");
        }

        const T* try_get() const noexcept { return const_cast<lazy*>(this)->try_get(); }
        const T& get() const { return const_cast<lazy*>(this)->get(); }

        operator T&() { return get(); }
        T& operator ()() { return get(); }
        T& operator *() { return get(); }
        T* operator ->() { return &get(); }

        operator const T&() const { return get(); }
        const T& operator ()() const { return get(); }
        const T& operator *() const { return get(); }
        const T* operator ->() const { return &get(); }
    };
}
