/// @file
/// @brief  xtl::type_indexed_map
/// @author ttsuki

#pragma once

#include <typeindex>
#include <unordered_map>
#include <stdexcept>

#include "xtl_any.h"

namespace xtl
{
    class type_indexed_map final
    {
        // type_index(T) -> T
        std::unordered_map<std::type_index, any> container_{};
        template <class T> static inline std::type_index index = std::type_index(typeid(T));

    public:
        type_indexed_map() = default;
        type_indexed_map(const type_indexed_map& other) = delete;
        type_indexed_map(type_indexed_map&& other) noexcept = delete;
        type_indexed_map& operator=(const type_indexed_map& other) = delete;
        type_indexed_map& operator=(type_indexed_map&& other) noexcept = delete;
        ~type_indexed_map() = default;

        template <class T>
        T& insert(std::decay_t<T> value)
        {
            container_.erase(index<T>);
            auto [it, inserted] = container_.insert_or_assign(index<T>, std::move(value));
            return *it->second.template get_if<T>();
        }

        template <class T, class...Args>
        T& emplace(Args&&...args)
        {
            container_.erase(index<T>);
            auto [it, inserted] = container_.emplace(std::piecewise_construct, std::make_tuple(index<T>), std::forward_as_tuple(std::in_place_type<T>, std::forward<Args>(args)...));
            return *it->second.template get_if<T>();
        }

        template <class T>
        [[nodiscard]] const T* find() const noexcept
        {
            if (auto it = container_.find(index<T>); it != container_.end())
                return it->second.template get_if<T>();

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
        [[nodiscard]] const T& at() const
        {
            if (auto* p = find<T>())
                return *p;

            throw std::out_of_range("no such key");
        }

        template <class T>
        [[nodiscard]] T& at()
        {
            return const_cast<T&>(const_cast<const type_indexed_map*>(this)->at<T>());
        }

        template <class T>
        void erase()
        {
            return container_.erase(index<T>);
        }
    };
}
