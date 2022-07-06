/// @file
/// @brief  xtl::aligned_memory_block
/// @author ttsuki

#pragma once
#include "xtl.config.h"

#include <cstddef>
#include <memory>

namespace
XTL_NAMESPACE
{
    template <size_t alignment = 32>
    class aligned_memory_block
    {
        struct alignas(alignment) E
        {
            std::byte e[alignment];
        };

        static inline constexpr size_t pre_under_block_count_ = 1;
        static inline constexpr size_t post_over_block_count_ = 1;

        std::unique_ptr<E[]> ptr_{};
        size_t capacity_{};
        size_t size_{};

    public:
        /// Empty
        aligned_memory_block() = default;

        /// Allocates memory block
        explicit aligned_memory_block(size_t size) { resize(size); }

        aligned_memory_block(const aligned_memory_block& other) = delete;
        aligned_memory_block(aligned_memory_block&& other) noexcept = default;
        aligned_memory_block& operator=(const aligned_memory_block& other) = delete;
        aligned_memory_block& operator=(aligned_memory_block&& other) noexcept = default;
        ~aligned_memory_block() = default;

        /// Gets the pointer to the buffer.
        [[nodiscard]] void* get() { return ptr_.get() + pre_under_block_count_; }

        /// Gets the pointer to the buffer.
        [[nodiscard]] void* data() { return ptr_.get() + pre_under_block_count_; }

        /// Gets the pointer to the buffer.
        [[nodiscard]] const void* data() const { return ptr_.get() + pre_under_block_count_; }

        /// Gets the current size.
        [[nodiscard]] size_t size() const { return size_; }

        /// Gets the current capacity.
        [[nodiscard]] size_t capacity() const { return capacity_; }

        /// Expands capacity if needed.
        void reserve(size_t capacity, bool keep_data = true)
        {
            if (capacity > capacity_)
            {
                size_t new_count = pre_under_block_count_ + ((capacity + (alignment - 1)) / alignment) + post_over_block_count_;
                std::unique_ptr<E[]> new_pointer = std::make_unique<E[]>(new_count);
                if (keep_data) std::memcpy(new_pointer.get(), ptr_.get(), size_);
                ptr_ = std::move(new_pointer);
                capacity_ = capacity;
            }
        }

        /// Sets the size, and expands capacity if needed.
        void resize(size_t size, bool keep_data = true)
        {
            reserve(size, keep_data);
            size_ = size;
        }
    };
}
