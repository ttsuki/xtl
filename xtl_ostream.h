/// @file
/// @brief  xtl ostream
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <streambuf>
#include <ostream>

namespace
XTL_NAMESPACE
{
    template <class T, class streambuf>
    class basic_ostream_for_streambuf final
        : private streambuf
        , public std::basic_ostream<T>
    {
    public:
        template <class ...TArgs>
        explicit basic_ostream_for_streambuf(TArgs&&... streambuf_constructor_parameter)
            : streambuf(std::forward<TArgs>(streambuf_constructor_parameter)...)
            , std::basic_ostream<T>(static_cast<streambuf*>(this))
        {
        }

        basic_ostream_for_streambuf(const basic_ostream_for_streambuf& other) = delete;
        basic_ostream_for_streambuf(basic_ostream_for_streambuf&& other) noexcept = delete;
        basic_ostream_for_streambuf& operator=(const basic_ostream_for_streambuf& other) = delete;
        basic_ostream_for_streambuf& operator=(basic_ostream_for_streambuf&& other) noexcept = delete;
        ~basic_ostream_for_streambuf() override = default;
    };


    namespace streambufs
    {
        template <class T = char>
        class basic_o_null_stream_streambuf : public std::basic_streambuf<T>
        {
        public:
            using base_type = std::basic_streambuf<T>;
            using char_type = typename std::basic_streambuf<T>::char_type;
            using traits_type = typename std::basic_streambuf<T>::traits_type;
            using int_type = typename std::basic_streambuf<T>::int_type;
            using pos_type = typename std::basic_streambuf<T>::pos_type;
            using off_type = typename std::basic_streambuf<T>::off_type;

            char_type buffer_[4096];

            explicit basic_o_null_stream_streambuf()
            {
                base_type::setp(buffer_, buffer_, buffer_ + sizeof(buffer_));
            }

            ~basic_o_null_stream_streambuf() override
            {
            }

            int_type overflow(int_type c) override
            {
                base_type::setp(buffer_, buffer_, buffer_ + sizeof(buffer_));
                return traits_type::not_eof(c);
            }

            int sync() override
            {
                base_type::setp(buffer_, buffer_, buffer_ + sizeof(buffer_));
                return 0;
            }
        };
    }

    template <class char_type> using basic_onullstream = basic_ostream_for_streambuf<char_type, streambufs::basic_o_null_stream_streambuf<char_type>>;
    using onullstream = basic_onullstream<char>;
    using wonullstream = basic_onullstream<wchar_t>;

    namespace streambufs
    {
        template <class T = char>
        class basic_o_callback_streambuf : public std::basic_streambuf<T>
        {
        public:
            using base_type = std::basic_streambuf<T>;
            using char_type = typename base_type::char_type;
            using traits_type = typename base_type::traits_type;
            using int_type = typename base_type::int_type;
            using pos_type = typename base_type::pos_type;
            using off_type = typename base_type::off_type;
            using callback_type = std::function<void(const char_type*)>;

            char_type buffer_[4096];
            callback_type sink_;

            explicit basic_o_callback_streambuf(callback_type sink)
                : sink_(std::move(sink))
            {
                base_type::setp(buffer_, buffer_, buffer_ + _countof(buffer_) - 1);
            }

            ~basic_o_callback_streambuf() override
            {
                basic_o_callback_streambuf::sync();
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
                base_type::pbump(static_cast<int_type>(base_type::pbase() - base_type::pptr()));
                return 0;
            }
        };
    }

    template <class char_type> using basic_o_callback_stream = basic_ostream_for_streambuf<char_type, streambufs::basic_o_callback_streambuf<char_type>>;
    using o_callback_stream = basic_o_callback_stream<char>;
    using wo_callback_stream = basic_o_callback_stream<wchar_t>;

    template <class char_type> using basic_o_callback_stream_callback = typename streambufs::basic_o_callback_streambuf<char_type>::callback_type;
    using o_callback_stream_callback = basic_o_callback_stream_callback<char>;
    using wo_callback_stream_callback = basic_o_callback_stream_callback<wchar_t>;
}
