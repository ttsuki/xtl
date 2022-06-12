/// @file
/// @brief  xtl type indexed map
/// @author (C) 2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <unordered_map>
#include <typeindex>

#include "xtl_any.h"

namespace
XTL_NAMESPACE
{
    class type_indexed_map final
    {
        // type_index(T) -> T
        std::unordered_map<std::type_index, xtl::any> container_{};
        template <class T> static inline std::type_index index = std::type_index(typeid(T));

    public:
        type_indexed_map() = default;
        type_indexed_map(const type_indexed_map& other) = delete;
        type_indexed_map(type_indexed_map&& other) noexcept = delete;
        type_indexed_map& operator=(const type_indexed_map& other) = delete;
        type_indexed_map& operator=(type_indexed_map&& other) noexcept = delete;
        ~type_indexed_map() = default;

        template <class T>
        [[nodiscard]] const T* find() const noexcept
        {
            if (auto it = container_.find(index<T>); it != container_.end())
                return it->second.template get<T>();

            return nullptr; // not set
        }

        template <class T>
        [[nodiscard]] T* find() noexcept
        {
            return const_cast<T*>(const_cast<const type_indexed_map*>(this)->find<T>());
        }

        template <class T>
        [[nodiscard]] size_t count() const noexcept
        {
            return find<T>() ? 1 : 0;
        }

        template <class T>
        [[nodiscard]] const T& get() const
        {
            if (auto* p = find<T>())
                return *p;

            throw std::out_of_range("no such key");
        }

        template <class T>
        [[nodiscard]] T& get()
        {
            return const_cast<T&>(const_cast<const type_indexed_map*>(this)->get<T>());
        }

        template <class T>
        void set(std::decay_t<T> value)
        {
            container_.erase(index<T>);
            container_.emplace(index<T>, std::move(value));
        }

        template <class T>
        void erase()
        {
            container_.erase(std::type_index(typeid(T)));
        }
    };
}
