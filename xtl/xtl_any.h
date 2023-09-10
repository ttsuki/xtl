/// @file
/// @brief  xtl::any - a move only any implementation
/// @author (C) 2022 ttsuki
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

#include "xtl_small_object_optimization.h"

namespace xtl
{
    // move-only any
    class any
    {
        template <class T> static constexpr inline bool is_any = std::is_same_v<std::decay_t<T>, any>;
        template <class T> static constexpr inline bool is_fit = std::is_nothrow_move_constructible_v<T>;

        struct alignas(std::max_align_t) alignment_t
        {
            std::byte buffer[sizeof(void*) * 7];
        };

        using soo = small_object_optimization<alignment_t>;
        const soo::basic_vtable* vtable_{};
        alignas(std::max_align_t) soo::memory memory_{};

    public:
        // default constructor
        any() = default;

        // copy constructor (deleted)
        any(const any& other) = delete;

        // copy assign operator (deleted)
        any& operator=(const any& other) = delete;

        // move constructor
        any(any&& other) noexcept
        {
            this->operator=(std::move(other));
        }

        // move assign operator
        any& operator=(any&& other) noexcept
        {
            if (std::addressof(other) != this)
            {
                this->reset();
                if (other.vtable_)
                {
                    vtable_ = std::exchange(other.vtable_, nullptr);
                    vtable_->move_constructor(this->memory_, std::move(other.memory_));
                }
            }

            return *this;
        }

        // constructor from T&&
        template <class T, std::enable_if_t<!is_any<T> && is_fit<T>>* = nullptr>
        any(T&& object)
        {
            this->emplace<std::decay_t<T>>(std::forward<T>(object));
        }

        // in-place constructor from Args&&...
        template <class T, class... Args, std::enable_if_t<!is_any<T> && is_fit<T> && std::is_constructible_v<T, Args&&...>>* = nullptr>
        any(std::in_place_type_t<T>, Args&&... args)
        {
            this->emplace<T>(std::forward<Args>(args)...);
        }

        // destructor
        ~any()
        {
            this->reset();
        }

        // emplaces from Args&&...
        template <class T, class... Args, std::enable_if_t<!is_any<T> && is_fit<T> && std::is_constructible_v<T, Args&&...>>* = nullptr>
        std::decay_t<T>& emplace(Args&&... args)
        {
            this->reset();
            auto& ref = soo::construct<T>(this->memory_, std::forward<Args>(args)...);
            this->vtable_ = soo::get_basic_vtable<T>();
            return ref;
        }

        // destructs holding object
        void reset() noexcept
        {
            if (this->vtable_)
            {
                this->vtable_->destructor(this->memory_);
                this->vtable_ = nullptr;
            }
        }

        // swaps with rhs
        void swap(any& rhs) noexcept
        {
            any tmp{std::move(rhs)};
            rhs = std::move(*this);
            *this = std::move(tmp);
        }

        [[nodiscard]] bool has_value() const noexcept
        {
            return vtable_;
        }

        [[nodiscard]] const std::type_info& type() const noexcept
        {
            return vtable_ ? vtable_->type : typeid(void);
        }

        [[nodiscard]] std::type_index type_index() const noexcept
        {
            return std::type_index{type()};
        }

        template <class T>
        [[nodiscard]] bool has() const noexcept
        {
            return vtable_ && vtable_->type == typeid(T);
        }

        template <class T>
        [[nodiscard]] T* get_if() noexcept
        {
            return has<T>() ? soo::pointer<T>(memory_) : nullptr;
        }

        template <class T>
        [[nodiscard]] const T* get_if() const noexcept
        {
            return has<T>() ? soo::const_pointer<T>(memory_) : nullptr;
        }
    };

    using std::bad_any_cast;

    template <class T, std::enable_if_t<std::is_constructible_v<T, const std::remove_cv_t<std::remove_reference_t<T>>&>> * = nullptr>
    [[nodiscard]] T any_cast(const any& a)
    {
        std::any x;
        std::any_cast<int>(std::move(x));
        if (auto p = a.get_if<std::remove_cv_t<std::remove_reference_t<T>>>()) return *p;
        else throw bad_any_cast();
    }

    template <class T, std::enable_if_t<std::is_constructible_v<T, std::remove_cv_t<std::remove_reference_t<T>>&>>* = nullptr>
    [[nodiscard]] T any_cast(any& a)
    {
        if (auto p = a.get_if<std::remove_cv_t<std::remove_reference_t<T>>>()) return *p;
        else throw bad_any_cast();
    }

    template <class T, std::enable_if_t<std::is_constructible_v<T, std::remove_cv_t<std::remove_reference_t<T>>>>* = nullptr>
    [[nodiscard]] T any_cast(any&& a)
    {
        if (auto p = a.get_if<std::remove_cv_t<std::remove_reference_t<T>>>()) return std::move(*p);
        else throw bad_any_cast();
    }

    template <class T>
    [[nodiscard]] const T* any_cast(const any* a)
    {
        return a->get_if<T>();
    }

    template <class T>
    [[nodiscard]] T* any_cast(any* a)
    {
        return a->get_if<T>();
    }
}
