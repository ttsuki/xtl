/// @file
/// @brief  xtl::copy_move_operation_debug_helper
/// @author (C) 2023 ttsuki
/// Distributed under the Boost Software License, Version 1.0.
#pragma once

#include <iostream>

namespace xtl::debug
{
    struct copy_move_operation_debug_helper
    {
        template <class Mark = int>
        struct non_movable
        {
            Mark mark{};
            non_movable(Mark mark = Mark{}) : mark(mark) { std::clog << "ctor " << this << ' ' << mark << std::endl; }
            non_movable(const non_movable& other) = delete;
            non_movable(non_movable&& other) noexcept = delete;
            non_movable& operator=(const non_movable& other) = delete;
            non_movable& operator=(non_movable&& other) noexcept = delete;
            virtual ~non_movable() { std::clog << "dtor " << this << ' ' << mark << std::endl; }
        };

        template <class Mark = int>
        struct movable
        {
            Mark mark{};
            movable(Mark mark = Mark{}) : mark(mark) { std::clog << "ctor " << this << ' ' << mark << std::endl; }
            movable(const movable& other) = delete;
            movable(movable&& other) noexcept : mark(std::exchange(other.mark, Mark{})) { std::clog << "move ctor " << this << ' ' << mark << std::endl; }
            movable& operator=(const movable& other) = delete;
            movable& operator=(movable&& other) noexcept { return ((std::addressof(other) != this) ? (void)(std::clog << "move assign " << this << ' ' << (mark = std::exchange(other.mark, Mark{})) << std::endl) : (void)0), *this; }
            virtual ~movable() { std::clog << "dtor " << this << ' ' << mark << std::endl; }
        };

        template <class Mark = int>
        struct copyable
        {
            Mark mark{};
            copyable(Mark mark = Mark{}) : mark(mark) { std::clog << "ctor " << this << ' ' << mark << std::endl; }
            copyable(const copyable& other) : mark(other.mark) { std::clog << "copy ctor " << this << ' ' << mark << std::endl; }
            copyable(copyable&& other) noexcept : mark(std::exchange(other.mark, Mark{})) { std::clog << "move ctor " << this << ' ' << mark << std::endl; }
            copyable& operator=(const copyable& other) { return ((std::addressof(other) != this) ? (void)(std::clog << "copy assign " << this << ' ' << (mark = other.mark) << std::endl) : (void)0), *this; }
            copyable& operator=(copyable&& other) noexcept { return ((std::addressof(other) != this) ? (void)(std::clog << "move assign " << this << ' ' << (mark = std::exchange(other.mark, Mark{})) << std::endl) : (void)0), *this; }
            virtual ~copyable() { std::clog << "dtor " << this << ' ' << mark << std::endl; }
        };
    };
}
