/// @file
/// @brief  xtl memory_byte_stream
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>
#include <algorithm>

#include "xtl_aligned_memory_block.h"
#include "xtl_spin_lock_mutex.h"

namespace
XTL_NAMESPACE
{
    /// On-memory byte stream.
    class memory_byte_stream
    {
        static inline constexpr size_t block_size = 262144;
        using block = aligned_memory_block<32>;
        static block allocate_block() { return block(block_size); }

        struct memory_block
        {
            recursive_spin_lock_mutex mutex_{};
            std::vector<block> memory_{};
            size_t length_{};
        };

        std::shared_ptr<memory_block> stream_{};
        size_t cursor_{};

    public:
        memory_byte_stream()
            : stream_(std::make_shared<memory_block>())
        {
        }

    private:
        memory_byte_stream(const memory_byte_stream& other) = default;
        memory_byte_stream& operator=(const memory_byte_stream& other) = default;

    public:
        /// Create stream shared data with separated cursor
        memory_byte_stream get_shared_stream() { return memory_byte_stream{*this}; }

        memory_byte_stream(memory_byte_stream&& other) noexcept = default;
        memory_byte_stream& operator=(memory_byte_stream&& other) noexcept = default;
        ~memory_byte_stream() = default;

        size_t read(void* buffer, size_t bufferSize)
        {
            lock_guard lock(stream_->mutex_);

            auto size = std::min(stream_->length_ - cursor_, bufferSize);

            // copy data
            auto dst = static_cast<std::byte*>(buffer);
            auto rem = size;
            while (rem)
            {
                auto s = cursor_;
                auto e = std::min((s + block_size) / block_size * block_size, s + rem);
                auto i = s / block_size;
                auto p = s % block_size;
                auto l = e - s;

                memcpy(dst, static_cast<std::byte*>(stream_->memory_[i].get()) + p, l);
                cursor_ += l;
                dst += l;
                rem -= l;
            }
            return size;
        }

        size_t write(const void* data, size_t dataLength)
        {
            lock_guard lock(stream_->mutex_);
            resize(std::max(stream_->length_, cursor_ + dataLength));

            // copy data
            auto src = static_cast<const std::byte*>(data);
            auto rem = dataLength;
            while (rem)
            {
                auto s = cursor_;
                auto e = std::min((s + block_size) / block_size * block_size, s + rem);
                auto i = s / block_size;
                auto p = s % block_size;
                auto l = e - s;

                memcpy(static_cast<std::byte*>(stream_->memory_[i].get()) + p, src, l);
                cursor_ += l;
                src += l;
                rem -= l;
            }

            return dataLength;
        }

        [[nodiscard]]
        size_t size() const
        {
            return stream_->length_;
        }

        size_t resize(size_t length)
        {
            lock_guard lock(stream_->mutex_);

            // updates stream length
            stream_->length_ = length;

            // allocates more memory block if needed.
            size_t blockCount = (length + block_size - 1) / block_size;
            for (size_t i = stream_->memory_.size(); i < blockCount; i++)
                stream_->memory_.push_back(allocate_block());

            return length;
        }

        [[nodiscard]]
        size_t tell() const { return cursor_; }

        size_t seek(ptrdiff_t offset, int whence)
        {
            lock_guard lock(stream_->mutex_);

            switch (whence)
            {
            case SEEK_SET: return cursor_ = std::clamp<size_t>(offset, 0, size());
            case SEEK_CUR: return cursor_ = std::clamp<size_t>(tell() + offset, 0, size());
            case SEEK_END: return cursor_ = std::clamp<size_t>(size() + offset, 0, size());
            default: return cursor_;
            }
        }
    };
}
