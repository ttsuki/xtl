/// @file
/// @brief  xtl::fixed_memory_stream
/// @author ttsuki

#pragma once
#include <cstddef>
#include <cstring>
#include <algorithm>

#include "xtl_rastream.h"

namespace xtl
{
    class fixed_memory_block_access final
    {
        void* ptr;
        size_t len;
        bool writable;

    public:
        fixed_memory_block_access(const void* ptr, size_t len) : ptr(const_cast<void*>(ptr)), len(len), writable(false) {}
        fixed_memory_block_access(void* ptr, size_t len, bool writable) : ptr(ptr), len(len), writable(writable) {}

        [[nodiscard]] size_t size() const noexcept { return len; }

        [[nodiscard]] size_t read(void* buffer, size_t cursor, size_t length) const noexcept
        {
            auto size = std::min(len - cursor, length);
            memcpy(buffer, static_cast<const std::byte*>(ptr) + cursor, size);
            return size;
        }

        [[nodiscard]] size_t write(const void* buffer, size_t cursor, size_t length) noexcept
        {
            if (!writable) return 0;
            auto size = std::min(len - cursor, length);
            memcpy(static_cast<std::byte*>(ptr) + cursor, buffer, size);
            return size;
        }

        // pointer semantic
        fixed_memory_block_access* operator ->() noexcept { return this; }
        const fixed_memory_block_access* operator ->() const noexcept { return this; }
    };

    /// read only stream for fixed memory block
    class fixed_memory_stream_ro : private irastream<fixed_memory_block_access>
    {
    public:
        explicit fixed_memory_stream_ro(const void* ptr, size_t len) : irastream(fixed_memory_block_access(ptr, len)) { }
        fixed_memory_stream_ro(const fixed_memory_stream_ro& other) = default;
        fixed_memory_stream_ro(fixed_memory_stream_ro&& other) noexcept = default;
        fixed_memory_stream_ro& operator=(const fixed_memory_stream_ro& other) = default;
        fixed_memory_stream_ro& operator=(fixed_memory_stream_ro&& other) noexcept = default;
        ~fixed_memory_stream_ro() = default;

        using irastream::size;
        using irastream::read;
        using irastream::tellg;
        using irastream::seekg;
    };

    /// read/write stream for fixed memory block
    class fixed_memory_stream_rw : private iorastream<fixed_memory_block_access>
    {
    public:
        explicit fixed_memory_stream_rw(void* ptr, size_t len) : iorastream(fixed_memory_block_access(ptr, len, true)) { }
        fixed_memory_stream_rw(const fixed_memory_stream_rw& other) = default;
        fixed_memory_stream_rw(fixed_memory_stream_rw&& other) noexcept = default;
        fixed_memory_stream_rw& operator=(const fixed_memory_stream_rw& other) = default;
        fixed_memory_stream_rw& operator=(fixed_memory_stream_rw&& other) noexcept = default;
        ~fixed_memory_stream_rw() = default;

        using iorastream::size;
        using iorastream::read;
        using iorastream::tellg;
        using iorastream::seekg;
        using iorastream::write;
        using iorastream::tellp;
        using iorastream::seekp;
    };
}
