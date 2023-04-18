/// @file
/// @brief  xtl::temporally memory buffer
/// @author ttsuki

#pragma once

#include <cstddef>
#include <new>
#include <memory>
#include <type_traits>

namespace xtl
{
    template <std::size_t Alignment, std::size_t BlockSize, std::size_t HeapAllocThreshold>
    class temp_memory_buffer_t final
    {
        struct alignas(Alignment) block_t
        {
            std::byte e[BlockSize];
        };

        static_assert(HeapAllocThreshold % BlockSize == 0);

        size_t capacity_ = sizeof(soo_);
        block_t soo_[HeapAllocThreshold / BlockSize];
        std::unique_ptr<block_t[]> ptr_;

    public:
        template <class T = std::byte, std::enable_if_t<std::is_trivial_v<T>>* = nullptr>
        [[nodiscard]] T* get(size_t count)
        {
            // re-allocates if need
            if (const auto requested_bytes = sizeof(T) * count; requested_bytes > capacity_)
            {
                const auto byte_count = std::max(requested_bytes * 5 / 4, requested_bytes + BlockSize);
                const size_t block_count = (byte_count + BlockSize - 1) / BlockSize;
                ptr_ = std::make_unique<block_t[]>(block_count + 1);
                capacity_ = block_count * BlockSize;
            }

            return capacity_ == sizeof(soo_) ? reinterpret_cast<T*>(&soo_) : reinterpret_cast<T*>(ptr_.get());
        }
    };

    using temp_memory_buffer = temp_memory_buffer_t<32, 256, 4096>; // <=4KB from stack or allocate from heap.
}
