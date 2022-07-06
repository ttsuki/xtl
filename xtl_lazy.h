/// @file
/// @brief  xtl::lazy
/// @author ttsuki

#pragma once
#include "xtl.config.h"

#include <functional>
#include <stdexcept>
#include <variant>

namespace
XTL_NAMESPACE
{
    template <class T>
    struct lazy
    {
        using value_t = T;
        using factory_t = std::function<value_t()>;
        mutable std::variant<value_t, factory_t> instance;

        struct lazy_ctor
        {
        };

        lazy(value_t value)
            : instance(std::move(value))
        {
        }

        lazy(factory_t factory)
            : instance(std::move(factory))
        {
        }

        template <class...TArgs>
        lazy(lazy_ctor, TArgs ...args)
            : lazy(
                factory_t(
                    [tpl = std::make_tuple(std::move(args)...)]() mutable
                    {
                        return std::apply([](auto&& ...arg)
                        {
                            return value_t(std::forward<decltype(arg)>(arg)...);
                        }, std::move(tpl));
                    })
            )
        {
        }


        T& get()
        {
            if (std::holds_alternative<factory_t>(instance)) { instance = std::get<factory_t>(instance)(); }
            if (std::holds_alternative<value_t>(instance)) { return std::get<value_t>(instance); }
            throw std::runtime_error("invalid instance status!");
        }

        const T& get() const
        {
            return const_cast<lazy*>(this)->get();
        }

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
