/// @file
/// @brief  xtl::enum_indexed_array
/// @author (C) 2023 ttsuki
/// Distributed under the Boost Software License, Version 1.0.

#pragma once

#include <cstddef>
#include <type_traits>
#include <array>

namespace xtl
{
    template <class T, auto count, std::enable_if_t<std::is_enum_v<decltype(count)>> * = nullptr>
    class enum_indexed_array : private std::array<T, static_cast<size_t>(count)>
    {
    public:
        using base_array = std::array<T, static_cast<size_t>(count)>;
        using value_type = typename base_array::value_type;
        using size_type = typename base_array::size_type;
        using difference_type = typename base_array::difference_type;
        using index_type = decltype(count);

        using pointer = typename base_array::pointer;
        using reference = typename base_array::reference;
        using iterator = typename base_array::iterator;
        using reverse_iterator = typename base_array::reverse_iterator;
        using const_pointer = typename base_array::const_pointer;
        using const_reference = typename base_array::const_reference;
        using const_iterator = typename base_array::const_iterator;
        using const_reverse_iterator = typename base_array::const_reverse_iterator;

        enum_indexed_array() = default;
        enum_indexed_array(std::initializer_list<T> il) : base_array{il} { }
        enum_indexed_array(const enum_indexed_array& other) = default;
        enum_indexed_array(enum_indexed_array&& other) noexcept = default;
        enum_indexed_array& operator=(const enum_indexed_array& other) = default;
        enum_indexed_array& operator=(enum_indexed_array&& other) noexcept = default;

        constexpr reference at(index_type i) noexcept { return base_array::at(static_cast<size_type>(i)); }
        constexpr const_reference at(index_type i) const noexcept { return base_array::at(static_cast<size_type>(i)); }
        constexpr reference operator[](index_type i) noexcept { return base_array::operator[](static_cast<size_type>(i)); }
        constexpr const_reference operator[](index_type i) const noexcept { return base_array::operator[](static_cast<size_type>(i)); }

        using base_array::begin;
        using base_array::cbegin;
        using base_array::rbegin;
        using base_array::crbegin;
        using base_array::end;
        using base_array::cend;
        using base_array::rend;
        using base_array::crend;

        using base_array::empty;
        using base_array::size;
        using base_array::max_size;

        constexpr base_array& as_base_array() noexcept { return *this; }
        constexpr const base_array& as_base_array() const noexcept { return *this; }
        friend bool operator==(const enum_indexed_array& lhs, const enum_indexed_array& rhs) noexcept { return lhs.as_base_array() == rhs.as_base_array(); }
        friend bool operator!=(const enum_indexed_array& lhs, const enum_indexed_array& rhs) noexcept { return lhs.as_base_array() != rhs.as_base_array(); }
        friend bool operator<(const enum_indexed_array& lhs, const enum_indexed_array& rhs) noexcept { return lhs.as_base_array() < rhs.as_base_array(); }
        friend bool operator>(const enum_indexed_array& lhs, const enum_indexed_array& rhs) noexcept { return lhs.as_base_array() > rhs.as_base_array(); }
        friend bool operator<=(const enum_indexed_array& lhs, const enum_indexed_array& rhs) noexcept { return lhs.as_base_array() <= rhs.as_base_array(); }
        friend bool operator>=(const enum_indexed_array& lhs, const enum_indexed_array& rhs) noexcept { return lhs.as_base_array() >= rhs.as_base_array(); }
    };
}
