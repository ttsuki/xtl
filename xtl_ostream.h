/// @file
/// @brief  xtl::ostream
/// @author ttsuki
#pragma once

#include <functional>
#include <string>
#include <streambuf>
#include <ostream>

namespace xtl
{
    template <class T, class streambuf>
    class basic_ostream_for_streambuf final
        : private streambuf
        , public std::basic_ostream<T>
    {
    public:
        template <class... TArgs>
        explicit basic_ostream_for_streambuf(TArgs&&... streambuf_constructor_parameter)
            : streambuf(std::forward<TArgs>(streambuf_constructor_parameter)...)
            , std::basic_ostream<T>(static_cast<streambuf*>(this)) { }

        basic_ostream_for_streambuf(const basic_ostream_for_streambuf& other) = delete;
        basic_ostream_for_streambuf(basic_ostream_for_streambuf&& other) noexcept = delete;
        basic_ostream_for_streambuf& operator=(const basic_ostream_for_streambuf& other) = delete;
        basic_ostream_for_streambuf& operator=(basic_ostream_for_streambuf&& other) noexcept = delete;
        ~basic_ostream_for_streambuf() override = default;
    };

    template <class T = char>
    class basic_null_ostreambuf : public std::basic_streambuf<T>
    {
    public:
        using base_type = std::basic_streambuf<T>;
        using char_type = typename std::basic_streambuf<T>::char_type;
        using traits_type = typename std::basic_streambuf<T>::traits_type;
        using int_type = typename std::basic_streambuf<T>::int_type;
        using pos_type = typename std::basic_streambuf<T>::pos_type;
        using off_type = typename std::basic_streambuf<T>::off_type;

        char_type buffer_[3072];

        explicit basic_null_ostreambuf() { base_type::setp(buffer_, buffer_, buffer_ + std::size(buffer_) - 1); }
        basic_null_ostreambuf(const basic_null_ostreambuf& other) = delete;
        basic_null_ostreambuf(basic_null_ostreambuf&& other) noexcept = delete;
        basic_null_ostreambuf& operator=(const basic_null_ostreambuf& other) = delete;
        basic_null_ostreambuf& operator=(basic_null_ostreambuf&& other) noexcept = delete;
        ~basic_null_ostreambuf() override { }

        int_type overflow(int_type c) override
        {
            base_type::setp(buffer_, buffer_, buffer_ + std::size(buffer_) - 1);
            return traits_type::not_eof(c);
        }

        int sync() override
        {
            base_type::setp(buffer_, buffer_, buffer_ + std::size(buffer_) - 1);
            return 0;
        }
    };

    template <class char_type> using basic_null_ostream = basic_ostream_for_streambuf<char_type, basic_null_ostreambuf<char_type>>;
    using null_ostream = basic_null_ostream<char>;

    template <class T = char>
    class basic_callback_ostreambuf : public std::basic_streambuf<T>
    {
    public:
        using base_type = std::basic_streambuf<T>;
        using char_type = typename base_type::char_type;
        using traits_type = typename base_type::traits_type;
        using int_type = typename base_type::int_type;
        using pos_type = typename base_type::pos_type;
        using off_type = typename base_type::off_type;
        using callback_type = std::function<void(const char_type*)>;

        callback_type sink_;
        char_type buffer_[3072];

        explicit basic_callback_ostreambuf(callback_type sink) : sink_(std::move(sink)) { base_type::setp(buffer_, buffer_ + std::size(buffer_) - 1); }
        basic_callback_ostreambuf(const basic_callback_ostreambuf& other) = delete;
        basic_callback_ostreambuf(basic_callback_ostreambuf&& other) noexcept = delete;
        basic_callback_ostreambuf& operator=(const basic_callback_ostreambuf& other) = delete;
        basic_callback_ostreambuf& operator=(basic_callback_ostreambuf&& other) noexcept = delete;

        ~basic_callback_ostreambuf() override
        {
            basic_callback_ostreambuf::sync();
        }

        int_type overflow(int_type c) override
        {
            sync();

            if (c != traits_type::eof())
            {
                *base_type::pptr() = traits_type::to_char_type(c);
                base_type::pbump(1);
            }
            return traits_type::not_eof(c);
        }

        int sync() override
        {
            if (base_type::pbase() == base_type::pptr()) { return 0; }

            *base_type::pptr() = traits_type::to_char_type('\0');
            sink_(base_type::pbase());
            base_type::pbump(static_cast<int>(base_type::pbase() - base_type::pptr()));
            return 0;
        }
    };

    template <class char_type> using basic_callback_ostream = basic_ostream_for_streambuf<char_type, basic_callback_ostreambuf<char_type>>;
    using callback_ostream = basic_callback_ostream<char>;

    template <class char_type = char, class F, std::enable_if_t<std::is_invocable_v<F, const char_type*>>* = nullptr>
    static basic_callback_ostream<char_type> make_callback_ostream_with_prefix(F callback_per_line, std::basic_string_view<char_type> prefix = {})
    {
        std::basic_string<char_type> buffer;
        buffer.reserve(prefix.size() + 1024);
        buffer.append(prefix);

        return basic_callback_ostream<char_type>([callback_per_line = std::move(callback_per_line), buffer = std::move(buffer)](const char_type* text) mutable
        {
            const size_t prefix_size = buffer.size();

            while (auto c = *text++)
            {
                buffer += c;
                if (c == '\n')
                {
                    callback_per_line(buffer.c_str());
                    buffer.resize(prefix_size);
                }
            }

            if (buffer.size() != prefix_size)
            {
                buffer += '\n';
                callback_per_line(buffer.c_str());
                buffer.resize(prefix_size);
            }
        });
    }
}
