/// @file
/// @brief  xtl::small_object_optimization - memory allocator with sso
/// @author (C) 2023 ttsuki
/// Distributed under the Boost Software License, Version 1.0.

#pragma once

#include <cstddef>
#include <new>
#include <array>
#include <cassert>
#include <memory>
#include <variant>
#include <typeindex>
#include <any>
#include <stdexcept>

namespace xtl
{
    template <class alignment_t>
    struct small_object_optimization
    {
        static_assert(std::is_pod_v<alignment_t>);

        using soo_memory = alignment_t;
        using heap_memory = alignment_t*;

        union alignas(alignment_t) memory // = std::variant<empty_memory, soo_memory, heap_memory>;
        {
            soo_memory soo_memory_;
            heap_memory heap_memory_;
        };

        template <class T>
        static constexpr inline bool use_soo = (sizeof(T) <= sizeof(soo_memory)) && std::is_nothrow_move_constructible_v<T>;

        template <class T>
        static T* pointer(memory& this_)
        {
            if constexpr (use_soo<T>)
            {
                return reinterpret_cast<T*>(&this_.soo_memory_);
            }
            else
            {
                return reinterpret_cast<T*>(this_.heap_memory_);
            }
        }

        template <class T>
        static const T* const_pointer(const memory& this_)
        {
            if constexpr (use_soo<T>)
            {
                return reinterpret_cast<const T*>(&this_.soo_memory_);
            }
            else
            {
                return reinterpret_cast<const T*>(this_.heap_memory_);
            }
        }

        using constructor_function = void(*)(memory&) noexcept;

        template <class T, class... A, std::enable_if_t<std::is_constructible_v<T, A&&...>>* = nullptr>
        static T& construct(memory& this_, A&&... args)
        {
            // alloc memory
            if constexpr (use_soo<T>)
            {
                this_.soo_memory_ = soo_memory{};
            }
            else
            {
                this_.heap_memory_ = new alignment_t[(sizeof(T) + sizeof(alignment_t) - 1) / sizeof(alignment_t)];
            }

            // placement new
            return *new(pointer<T>(this_)) T(std::forward<A>(args)...);
        }

        using move_constructor_function = void(*)(memory&, memory&&) noexcept;

        template <class T>
        static void move_construct(memory& this_, memory&& from) noexcept
        {
            if constexpr (use_soo<T>)
            {
                this_.soo_memory_ = soo_memory{};
                new(pointer<T>(this_)) T(std::move(*pointer<T>(from)));
                pointer<T>(from)->~T();
            }
            else
            {
                this_.heap_memory_ = std::exchange(from.heap_memory_, nullptr);
            }
        }

        using destructor_function = void(*)(memory&) noexcept;

        template <class T>
        static void destruct(memory& this_) noexcept
        {
            // placement delete
            pointer<T>(this_)->~T();

            // free memory
            if constexpr (use_soo<T>)
            {
                // do nothing
            }
            else
            {
                delete[] this_.heap_memory_;
                this_.heap_memory_ = nullptr;
            }
        }

        struct basic_vtable
        {
            const std::type_info& type = typeid(void);
            const move_constructor_function move_constructor;
            const destructor_function destructor;
        };

        template <class T>
        static constexpr inline basic_vtable basic_vtable_for = {
            typeid(T),
            &move_construct<T>,
            &destruct<T>,
        };

        template <class T>
        static constexpr inline const basic_vtable* get_basic_vtable()
        {
            return &basic_vtable_for<T>;
        }
    };
}
