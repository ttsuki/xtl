/// @file
/// @brief  xtl::memory_block_iostream
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <cstring>
#include <cstddef>
#include <memory>
#include <algorithm>

namespace
XTL_NAMESPACE
{
    class memory_block_istream
    {
        const void* target{};
        size_t length{};
        size_t cursor{};

    public:
        memory_block_istream(const void* target, size_t length)
            : target(target)
            , length(length)
        {
        }

        size_t size() { return length; }
        size_t tellg() const { return cursor; }

        size_t seekg(ptrdiff_t offset, int whence = SEEK_SET)
        {
            switch (whence)
            {
            case SEEK_SET: return cursor = static_cast<size_t>(offset);
            case SEEK_CUR: return cursor = cursor + offset;
            case SEEK_END: return cursor = length + offset;
            default: break;
            }
            return cursor;
        }

        size_t read(void* buffer, size_t len)
        {
            if (cursor >= length) return 0;
            size_t reading = std::min(length - cursor, len);
            memcpy(buffer, static_cast<const std::byte*>(target) + cursor, reading);
            cursor += reading;
            return reading;
        }
    };

    class memory_block_ostream
    {
        void* target{};
        size_t length{};
        size_t cursor{};

    public:
        memory_block_ostream(void* target, size_t length)
            : target(target)
            , length(length)
        {
        }

        size_t size() const { return length; }
        size_t tellp() const { return cursor; }

        size_t seekp(ptrdiff_t offset, int whence = SEEK_SET)
        {
            switch (whence)
            {
            case SEEK_SET: return cursor = static_cast<size_t>(offset);
            case SEEK_CUR: return cursor = cursor + offset;
            case SEEK_END: return cursor = length + offset;
            default: break;
            }
            return cursor;
        }

        size_t write(const void* data, size_t len)
        {
            if (cursor >= length) return 0;
            size_t writing = std::min(length - cursor, len);
            if (data) memcpy(static_cast<std::byte*>(target) + cursor, data, writing);
            else memset(static_cast<std::byte*>(target) + cursor, 0, writing);
            cursor += writing;
            return writing;
        }
    };

    class memory_block_iostream
        : memory_block_istream
        , memory_block_ostream
    {
    public:
        memory_block_iostream(void* target, size_t length)
            : memory_block_istream(target, length)
            , memory_block_ostream(target, length)
        {
        }

        using memory_block_istream::size;
        using memory_block_istream::read;
        using memory_block_istream::seekg;
        using memory_block_ostream::write;
        using memory_block_ostream::seekp;

        size_t seek(size_t pos)
        {
            memory_block_istream::seekg(pos);
            memory_block_ostream::seekp(pos);
            return pos;
        }
    };

    static inline std::shared_ptr<memory_block_istream> make_memory_block_istream(std::shared_ptr<const void> target, size_t length)
    {
        auto istream = std::make_shared<memory_block_istream>(target.get(), length);
        return {
            istream.get(),
            [istream, target](void*) mutable
            {
                istream.reset();
                target.reset();
            }
        };
    }

    static inline std::shared_ptr<memory_block_ostream> make_memory_block_ostream(std::shared_ptr<void> target, size_t length)
    {
        auto ostream = std::make_shared<memory_block_ostream>(target.get(), length);
        return {
            ostream.get(),
            [ostream, target](void*) mutable
            {
                ostream.reset();
                target.reset();
            }
        };
    }

    static inline std::shared_ptr<memory_block_iostream> make_memory_block_iostream(std::shared_ptr<void> target, size_t length)
    {
        auto iostream = std::make_shared<memory_block_iostream>(target.get(), length);
        return {
            iostream.get(),
            [iostream, target](void*) mutable
            {
                iostream.reset();
                target.reset();
            }
        };
    }
}
