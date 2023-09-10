/// @file
/// @brief  xtl::type_punning_iterator
/// @author (C) 2023 ttsuki
/// Distributed under the Boost Software License, Version 1.0.

#pragma once

#include <iterator>
#include <type_traits>

namespace xtl
{
    template <class PunnedType, class BaseIterator>
    struct type_punning_iterator : BaseIterator
    {
        using iterator_category = typename BaseIterator::iterator_category;
        using value_type = PunnedType;
        using difference_type = typename BaseIterator::difference_type;
        using pointer = std::conditional_t<std::is_const_v<std::remove_pointer_t<typename BaseIterator::pointer>>, const PunnedType*, PunnedType*>;
        using reference = std::conditional_t<std::is_const_v<std::remove_pointer_t<typename BaseIterator::pointer>>, const PunnedType&, PunnedType&>;

        type_punning_iterator(BaseIterator it) : BaseIterator(it) {}

        [[nodiscard]] constexpr reference operator*() const noexcept { return *operator->(); }
        [[nodiscard]] constexpr pointer operator->() const noexcept { return reinterpret_cast<pointer>(BaseIterator::operator->()); }

        constexpr type_punning_iterator& operator++() noexcept { return (void)BaseIterator::operator++(), *this; }
        constexpr type_punning_iterator& operator--() noexcept { return (void)BaseIterator::operator--(), *this; }

        [[nodiscard]] constexpr type_punning_iterator operator++(int) noexcept
        {
            auto t = *this;
            ++*this;
            return t;
        }

        [[nodiscard]] constexpr type_punning_iterator operator--(int) noexcept
        {
            auto t = *this;
            --*this;
            return t;
        }

        constexpr type_punning_iterator& operator+=(difference_type offset) noexcept { return (void)(static_cast<BaseIterator&>(*this) += offset), *this; }
        constexpr type_punning_iterator& operator-=(difference_type offset) noexcept { return (void)(static_cast<BaseIterator&>(*this) -= offset), *this; }

        [[nodiscard]] constexpr type_punning_iterator operator+(difference_type offset) noexcept { return type_punning_iterator(static_cast<BaseIterator&>(*this) + offset); }
        [[nodiscard]] constexpr type_punning_iterator operator-(difference_type offset) noexcept { return type_punning_iterator(static_cast<BaseIterator&>(*this) - offset); }
        [[nodiscard]] constexpr reference operator[](difference_type offset) const noexcept { return *reinterpret_cast<pointer>(&BaseIterator::operator[](offset)); }

        [[nodiscard]] constexpr bool operator==(const type_punning_iterator& other) const noexcept { return static_cast<const BaseIterator&>(*this) == static_cast<const BaseIterator&>(other); }
        [[nodiscard]] constexpr bool operator!=(const type_punning_iterator& other) const noexcept { return static_cast<const BaseIterator&>(*this) != static_cast<const BaseIterator&>(other); }
        [[nodiscard]] constexpr bool operator<(const type_punning_iterator& other) const noexcept { return static_cast<const BaseIterator&>(*this) < static_cast<const BaseIterator&>(other); }
        [[nodiscard]] constexpr bool operator>(const type_punning_iterator& other) const noexcept { return static_cast<const BaseIterator&>(*this) > static_cast<const BaseIterator&>(other); }
        [[nodiscard]] constexpr bool operator<=(const type_punning_iterator& other) const noexcept { return static_cast<const BaseIterator&>(*this) <= static_cast<const BaseIterator&>(other); }
        [[nodiscard]] constexpr bool operator>=(const type_punning_iterator& other) const noexcept { return static_cast<const BaseIterator&>(*this) >= static_cast<const BaseIterator&>(other); }
    };
}
