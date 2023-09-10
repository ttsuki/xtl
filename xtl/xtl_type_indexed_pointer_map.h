/// @file
/// @brief  xtl::type_indexed_pmap - A simple type-pointer container
/// @author (C) 2023 ttsuki
/// Distributed under the Boost Software License, Version 1.0.

#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <stdexcept>

namespace xtl
{
    class type_indexed_pointer_map
    {
        // type_index(T) -> T
        std::unordered_map<std::type_index, std::shared_ptr<void>> container_{};
        template <class T> static inline std::type_index index = std::type_index(typeid(T*));

    public:
        type_indexed_pointer_map() = default;
        type_indexed_pointer_map(const type_indexed_pointer_map& other) = default;
        type_indexed_pointer_map(type_indexed_pointer_map&& other) noexcept = default;
        type_indexed_pointer_map& operator=(const type_indexed_pointer_map& other) = default;
        type_indexed_pointer_map& operator=(type_indexed_pointer_map&& other) noexcept = default;
        ~type_indexed_pointer_map() = default;

        template <class T>
        T* insert(std::shared_ptr<T> p)
        {
            auto r = p.get();
            container_.insert_or_assign(index<T>, std::move(p));
            return r;
        }

        template <class T, class U = T, class... Args>
        T* emplace(Args&&... args)
        {
            return this->insert<T>(std::make_shared<U>(std::forward<Args>(args)...));
        }

        template <class T>
        [[nodiscard]] T* find() const noexcept
        {
            if (auto it = container_.find(index<T>); it != container_.end())
                return std::static_pointer_cast<T>(it->second);

            return nullptr; // not set
        }

        template <class T>
        [[nodiscard]] size_t count() const noexcept
        {
            return find<T>() ? 1 : 0;
        }

        template <class T>
        [[nodiscard]] T* at() const noexcept
        {
            if (auto* p = find<T>())
                return p;

            throw std::out_of_range("no such key");
        }

        template <class T>
        size_t erase()
        {
            return container_.erase(index<T>);
        }
    };
}
