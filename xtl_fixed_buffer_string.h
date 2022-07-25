/// @file
/// @brief  xtl::fixed_buffer_string
/// @author ttsuki

#pragma once
#include "xtl.config.h"

#include <array>
#include <algorithm>

namespace
XTL_NAMESPACE
{
    template <class T, size_t Size>
    class fixed_buffer_string final
    {
        using base_container = std::array<T, Size>;

    public:
        using value_type = typename base_container::value_type;
        using size_type = typename base_container::size_type;
        using difference_type = typename base_container::difference_type;
        using iterator = typename base_container::iterator;
        using reference = typename base_container::reference;
        using pointer = typename base_container::pointer;
        using const_iterator = typename base_container::const_iterator;
        using const_reference = typename base_container::const_reference;
        using const_pointer = typename base_container::const_pointer;

    private:
        base_container container_{};
        size_type size_{};

    public:
        constexpr fixed_buffer_string() = default;

        constexpr fixed_buffer_string(const value_type* data, size_t length)
        {
            size_type sz = size_ = std::min(length, capacity());
            for (size_t i = 0; i < sz; ++i) container_[i] = data[i]; // std::copy_n
        }

        constexpr fixed_buffer_string(std::initializer_list<value_type> list)
        {
            size_type sz = size_ = std::min(list.size(), capacity());
            for (size_t i = 0; i < sz; ++i) container_[i] = *(list.begin() + i); // std::copy_n
        }


        [[nodiscard]] constexpr pointer data() noexcept { return container_.data(); }
        [[nodiscard]] constexpr const_pointer data() const noexcept { return container_.data(); }
        [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
        [[nodiscard]] constexpr size_type capacity() const noexcept { return container_.size(); }
        [[nodiscard]] constexpr reference operator[](size_type i) noexcept { return container_.operator[](i); }
        [[nodiscard]] constexpr const_reference operator[](size_type i) const noexcept { return container_.operator[](i); }

        [[nodiscard]] constexpr iterator begin() noexcept { return container_.begin(); }
        [[nodiscard]] constexpr iterator end() noexcept { return container_.begin() + static_cast<difference_type>(size_); }
        [[nodiscard]] constexpr const_iterator begin() const noexcept { return container_.begin(); }
        [[nodiscard]] constexpr const_iterator end() const noexcept { return container_.begin() + static_cast<difference_type>(size_); }

        constexpr void clear() noexcept { size_ = 0; }
        constexpr void resize(size_t sz) noexcept { size_ = sz; }

        template <size_t Size2>
        constexpr fixed_buffer_string& operator +=(const fixed_buffer_string<T, Size2>& rhs) noexcept
        {
            size_type sz = std::min(capacity() - size(), rhs.size());
            for (size_t i = 0; i < sz; ++i) container_[size_ + i] = rhs[i]; // std::copy_n
            size_ += sz;
            return *this;
        }

        constexpr fixed_buffer_string& operator +=(std::initializer_list<value_type> rhs) noexcept
        {
            size_type sz = std::min(capacity() - size(), rhs.size());
            for (size_t i = 0; i < sz; ++i) container_[size_ + i] = std::data(rhs)[i]; // std::copy_n
            size_ += sz;
            return *this;
        }
    };

    template <class T, size_t Size, class U>
    constexpr auto operator +(fixed_buffer_string<T, Size> lhs, const U& rhs) noexcept -> std::decay_t<decltype(lhs += rhs)>
    {
        return lhs += rhs;
    }
}
