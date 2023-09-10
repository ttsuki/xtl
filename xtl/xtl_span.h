/// @file
/// @brief  xtl::span - view of continuous memory buffer
/// @author (C) 2023 ttsuki
/// Distributed under the Boost Software License, Version 1.0.

#pragma once

#include <cstddef>
#include <type_traits>
#include <stdexcept>

namespace span
{
    template <class T>
    struct span_1d
    {
        using element_type = T;
        using void_t = std::conditional_t<std::is_const_v<T>, const void, void>;
        using byte_t = std::conditional_t<std::is_const_v<T>, const std::byte, std::byte>;

        void_t* pointer{};
        size_t width{};

        constexpr span_1d() = default;
        [[nodiscard]] constexpr size_t size() const noexcept { return width; }
        [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
        [[nodiscard]] constexpr T& cell(size_t x) const { return *static_cast<T*>(slice(x, 1).pointer); }
        [[nodiscard]] constexpr T& operator [](size_t index) const { return this->cell(index); }

        [[nodiscard]] constexpr span_1d slice(size_t x, size_t w) const
        {
#ifdef _DEBUG
            if (x > this->width) throw std::out_of_range("x");
            if (x + w > this->width) throw std::out_of_range("x + w");
#endif
            return span_1d{ reinterpret_cast<byte_t*>(pointer) + sizeof(T) * x, w };
        }

        template <class U>
        [[nodiscard]] constexpr span_1d<U> reinterpret_as() const
        {
            return span_1d<U>{pointer, width * sizeof(T) / sizeof(U)};
        }

        [[nodiscard]] constexpr T* data() const { return static_cast<T*>(pointer); }
        [[nodiscard]] constexpr auto begin() const { return data(); }
        [[nodiscard]] constexpr auto end() const { return data() + width; }
    };

    template <class T>
    struct span_2d
    {
        using element_type = T;
        using void_t = std::conditional_t<std::is_const_v<T>, const void, void>;
        using byte_t = std::conditional_t<std::is_const_v<T>, const std::byte, std::byte>;

        void_t* pointer{};
        size_t width{};
        size_t height{};
        size_t width_pitch{};

        constexpr span_2d() = default;
        [[nodiscard]] constexpr size_t size() const noexcept { return height; }
        [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
        [[nodiscard]] constexpr span_1d<T> row(size_t y) const { return span_1d<T>{this->slice(0, y, 0, 1).pointer, width}; }
        [[nodiscard]] constexpr span_1d<T> operator [](size_t index) const { return this->row(index); }

        [[nodiscard]] constexpr span_2d<T> slice(size_t x, size_t y, size_t w, size_t h) const
        {
#ifdef _DEBUG
            if (x > this->width) throw std::out_of_range("x");
            if (x + w > this->width) throw std::out_of_range("x + w");
            if (y > this->height) throw std::out_of_range("y");
            if (y + h > this->height) throw std::out_of_range("y + h");
#endif
            return span_2d<T>{reinterpret_cast<byte_t*>(pointer) + width_pitch * y + sizeof(T) * x, w, h, width_pitch};
        }

        template <class U>
        [[nodiscard]] constexpr span_2d<U> reinterpret_as() const
        {
            return span_2d<U>{pointer, width * sizeof(T) / sizeof(U), height, width_pitch};
        }
    };

    template <class T>
    struct span_3d
    {
        using element_type = T;
        using void_t = std::conditional_t<std::is_const_v<T>, const void, void>;
        using byte_t = std::conditional_t<std::is_const_v<T>, const std::byte, std::byte>;

        void_t* pointer{};
        size_t width{};
        size_t height{};
        size_t depth{};
        size_t width_pitch{};
        size_t height_pitch{};

        constexpr span_3d() = default;
        [[nodiscard]] constexpr size_t size() const noexcept { return depth; }
        [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
        [[nodiscard]] constexpr span_2d<T> plane(size_t z) const { return span_2d<T>{this->slice(0, 0, z, 0, 0, 1).pointer, width, height, width_pitch}; }
        [[nodiscard]] constexpr span_2d<T> operator [](size_t index) const { return this->plane(index); }

        [[nodiscard]] constexpr span_3d<T> slice(size_t x, size_t y, size_t z, size_t w, size_t h, size_t d) const
        {
#ifdef _DEBUG
            if (x > this->width) throw std::out_of_range("x");
            if (x + w > this->width) throw std::out_of_range("x + w");
            if (y > this->height) throw std::out_of_range("y");
            if (y + h > this->height) throw std::out_of_range("y + h");
            if (z > this->depth) throw std::out_of_range("z");
            if (z + d > this->depth) throw std::out_of_range("z + d");
#endif
            return span_3d<T>{reinterpret_cast<byte_t*>(pointer) + height_pitch * z + width_pitch * y + sizeof(T) * x, w, h, d, width_pitch, height_pitch};
        }

        template <class U>
        [[nodiscard]] constexpr span_3d<U> reinterpret_as() const
        {
            return span_3d<U>{pointer, width * sizeof(T) / sizeof(U), height, depth, width_pitch, height_pitch};
        }
    };

    using byte_span_1d = span_1d<std::byte>;
    using byte_span_2d = span_2d<std::byte>;
    using byte_span_3d = span_3d<std::byte>;

    template <class T> using span = span_1d<T>;
    using byte_span = byte_span_1d;
}
