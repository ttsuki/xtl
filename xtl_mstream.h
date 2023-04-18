/// @file
/// @brief  xtl::mstream
/// @author ttsuki

#pragma once

#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>
#include <algorithm>

#include "xtl_rastream.h"
#include "xtl_spin_lock_mutex.h"

namespace xtl
{
    /// On-memory byte stream.
    class random_access_memory_stream
    {
        static inline constexpr size_t alignment = 32;
        static inline constexpr size_t block_size = 65536;

        struct alignas(alignment) block
        {
            std::byte data[block_size];
        };

        mutable xtl::recursive_spin_lock_mutex mutex_{};
        std::vector<std::unique_ptr<block>> memory_{};
        size_t length_{};

    public:
        random_access_memory_stream() = default;
        random_access_memory_stream(const random_access_memory_stream& other) = delete;
        random_access_memory_stream(random_access_memory_stream&& other) noexcept = delete;
        random_access_memory_stream& operator=(const random_access_memory_stream& other) = delete;
        random_access_memory_stream& operator=(random_access_memory_stream&& other) noexcept = delete;
        ~random_access_memory_stream() = default;

        auto lock() const { return mutex_.lock(); }
        auto try_lock() const { return mutex_.try_lock(); }
        auto unlock() const { return mutex_.unlock(); }

        [[nodiscard]] size_t read(void* buffer, size_t cursor, size_t length) const
        {
            std::lock_guard lock(*this);

            if (cursor > length_) return 0;
            auto slen = length_ - cursor;
            auto size = std::min(slen, length);

            // copy data
            auto dst = static_cast<std::byte*>(buffer);
            auto rem = size;
            while (rem)
            {
                auto s = cursor;
                auto e = std::min((s + block_size) / block_size * block_size, s + rem);
                auto i = s / block_size;
                auto p = s % block_size;
                auto l = e - s;
                memcpy(dst, memory_.at(i)->data + p, l);
                cursor += l;
                dst += l;
                rem -= l;
            }
            return size;
        }

        [[nodiscard]] size_t write(const void* data, size_t cursor, size_t length)
        {
            std::lock_guard lock(*this);

            auto slen = (length_ >= cursor + length ? length_ : resize(cursor + length)) - cursor;
            auto size = std::min(slen, length);

            // copy data
            auto src = static_cast<const std::byte*>(data);
            auto rem = size;
            while (rem)
            {
                auto s = cursor;
                auto e = std::min((s + block_size) / block_size * block_size, s + rem);
                auto i = s / block_size;
                auto p = s % block_size;
                auto l = e - s;

                memcpy(memory_.at(i)->data + p, src, l);
                cursor += l;
                src += l;
                rem -= l;
            }

            return size;
        }

        [[nodiscard]] size_t size() const
        {
            std::lock_guard lock(*this);

            return length_;
        }

        [[nodiscard]] size_t resize(size_t length)
        {
            std::lock_guard lock(*this);

            // allocates more memory block if needed.
            const size_t new_block_count = (length + block_size - 1) / block_size;
            for (size_t i = memory_.size(); i < new_block_count; i++)
                memory_.push_back(std::make_unique<block>());

            length_ = length;
            return length;
        }
    };

    class mstream : private random_access_memory_stream, private iorastream<random_access_memory_stream*>
    {
    public:
        mstream() : iorastream(this) {}
        using iorastream::get_base_stream;
        using iorastream::size;
        using iorastream::read;
        using iorastream::tellg;
        using iorastream::seekg;
        using iorastream::write;
        using iorastream::tellp;
        using iorastream::seekp;
        using random_access_memory_stream::resize;
    };

    class shared_mstream : private iorastream<std::shared_ptr<random_access_memory_stream>>
    {
    public:
        shared_mstream() : iorastream(std::make_shared<random_access_memory_stream>()) {}
        explicit shared_mstream(std::shared_ptr<random_access_memory_stream> base) : iorastream(std::move(base)) {}
        using iorastream::get_base_stream;
        using iorastream::size;
        using iorastream::read;
        using iorastream::tellg;
        using iorastream::seekg;
        using iorastream::write;
        using iorastream::tellp;
        using iorastream::seekp;
        [[nodiscard]] size_t resize(size_t length) { return get_base_stream()->resize(length); }
    };

    using memory_byte_stream = shared_mstream;
}
