/// @file
/// @brief  xtl::memory_byte_stream
/// @author ttsuki

#pragma once

#include <cstddef>
#include <ios>

namespace xtl
{
    // input random access stream interface
    struct ira_base_stream_interface
    {
        virtual ~ira_base_stream_interface() = default;
        [[nodiscard]] virtual size_t size() const = 0;
        [[nodiscard]] virtual size_t read(const void* data, size_t cursor, size_t length) = 0;
    };

    // output random access stream interface
    struct ora_base_stream_interface
    {
        virtual ~ora_base_stream_interface() = default;
        [[nodiscard]] virtual size_t size() const = 0;
        [[nodiscard]] virtual size_t write(const void* data, size_t cursor, size_t length) = 0;
    };

    // input/output random access stream interface
    struct iora_base_stream_interface
    {
        virtual ~iora_base_stream_interface() = default;
        [[nodiscard]] virtual size_t size() const = 0;
        [[nodiscard]] virtual size_t read(const void* data, size_t cursor, size_t length) = 0;
        [[nodiscard]] virtual size_t write(const void* data, size_t cursor, size_t length) = 0;
    };

    // basic input random access stream
    template <class base_stream_pointer_t = ira_base_stream_interface*>
    class irastream
    {
        base_stream_pointer_t stream_{};
        size_t cursor_{};

    public:
        explicit irastream(base_stream_pointer_t base) : stream_(std::move(base)) { }
        irastream(const irastream& other) = default;
        irastream(irastream&& other) noexcept = default;
        irastream& operator=(const irastream& other) = default;
        irastream& operator=(irastream&& other) noexcept = default;
        ~irastream() = default;

        [[nodiscard]] base_stream_pointer_t get_base_stream() { return stream_; }
        [[nodiscard]] size_t size() const { return stream_->size(); }
        [[nodiscard]] size_t tellg() const { return cursor_; }

        [[nodiscard]] size_t read(void* buffer, size_t length)
        {
            const size_t read = stream_->read(buffer, cursor_, length);
            cursor_ += read;
            return read;
        }

        [[nodiscard]] size_t seekg(size_t offset)
        {
            return seekg(static_cast<ptrdiff_t>(offset), std::ios_base::beg);
        }

        [[nodiscard]] size_t seekg(ptrdiff_t offset, int whence)
        {
            const size_t size = stream_->size();
            switch (whence)
            {
            case std::ios_base::beg: return cursor_ = 0 + offset;
            case std::ios_base::cur: return cursor_ = cursor_ + offset;
            case std::ios_base::end: return cursor_ = size + offset;
            default: return cursor_;
            }
        }
    };

    // basic output random access stream
    template <class base_stream_pointer_t = ora_base_stream_interface*>
    class orastream
    {
        base_stream_pointer_t stream_{};
        size_t cursor_{};

    public:
        explicit orastream(base_stream_pointer_t base) : stream_(std::move(base)) { }
        orastream(const orastream& other) = default;
        orastream(orastream&& other) noexcept = default;
        orastream& operator=(const orastream& other) = default;
        orastream& operator=(orastream&& other) noexcept = default;
        ~orastream() = default;

        [[nodiscard]] base_stream_pointer_t get_base_stream() { return stream_; }
        [[nodiscard]] size_t size() const { return stream_->size(); }
        [[nodiscard]] size_t tellp() const { return cursor_; }

        [[nodiscard]] size_t write(const void* data, size_t length)
        {
            const size_t wrote = stream_->write(data, cursor_, length);
            cursor_ += wrote;
            return wrote;
        }

        [[nodiscard]] size_t seekp(size_t offset)
        {
            return seekp(static_cast<ptrdiff_t>(offset), std::ios_base::beg);
        }

        [[nodiscard]] size_t seekp(ptrdiff_t offset, int whence = std::ios_base::beg)
        {
            const size_t size = stream_->size();
            switch (whence)
            {
            case std::ios_base::beg: return cursor_ = 0 + offset;
            case std::ios_base::cur: return cursor_ = cursor_ + offset;
            case std::ios_base::end: return cursor_ = size + offset;
            default: return cursor_;
            }
        }
    };

    // basic input/output random access stream
    template <class base_stream_pointer_t = iora_base_stream_interface*>
    class iorastream
        : private irastream<base_stream_pointer_t>
        , private orastream<base_stream_pointer_t>
    {
    public:
        explicit iorastream(base_stream_pointer_t base)
            : irastream<base_stream_pointer_t>(base)
            , orastream<base_stream_pointer_t>(base) { }

        iorastream(const iorastream& other) = default;
        iorastream(iorastream&& other) noexcept = default;
        iorastream& operator=(const iorastream& other) = default;
        iorastream& operator=(iorastream&& other) noexcept = default;
        ~iorastream() = default;

        using irastream<base_stream_pointer_t>::get_base_stream;
        using irastream<base_stream_pointer_t>::read;
        using irastream<base_stream_pointer_t>::size;
        using irastream<base_stream_pointer_t>::tellg;
        using irastream<base_stream_pointer_t>::seekg;
        using orastream<base_stream_pointer_t>::write;
        using orastream<base_stream_pointer_t>::tellp;
        using orastream<base_stream_pointer_t>::seekp;
    };
}
