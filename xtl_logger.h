/// @file
/// @brief  xtl logger
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <mutex>
#include <streambuf>
#include <ostream>

#include "xtl_event_callback.h"

namespace
XTL_NAMESPACE
{
    template <class T, class streambuf>
    class ostream_for_streambuf final
        : private streambuf
        , public std::basic_ostream<T>
    {
    public:
        template <class ...TArgs>
        explicit ostream_for_streambuf(TArgs&&... streambuf_constructor_parameter)
            : streambuf(std::forward<TArgs>(streambuf_constructor_parameter)...)
            , std::basic_ostream<T>(static_cast<streambuf*>(this))
        {
        }

        ostream_for_streambuf(const ostream_for_streambuf& other) = delete;
        ostream_for_streambuf(ostream_for_streambuf&& other) noexcept = delete;
        ostream_for_streambuf& operator=(const ostream_for_streambuf& other) = delete;
        ostream_for_streambuf& operator=(ostream_for_streambuf&& other) noexcept = delete;
        ~ostream_for_streambuf() override = default;
    };


    namespace streambufs
    {
        template <class T = char>
        class callback_ostream_streambuf : public std::basic_streambuf<T>
        {
        public:
            using base_type = std::basic_streambuf<T>;
            using char_type = typename base_type::char_type;
            using traits_type = typename base_type::traits_type;
            using int_type = typename base_type::int_type;
            using pos_type = typename base_type::pos_type;
            using off_type = typename base_type::off_type;

            using callback_ostream_flush = std::function<void(const char_type*)>;

            char_type buffer_[1024]{};
            callback_ostream_flush sink_{};

            explicit callback_ostream_streambuf(callback_ostream_flush sink)
                : sink_(std::move(sink))
            {
                base_type::setp(buffer_, buffer_, buffer_ + _countof(buffer_) - 1);
            }

            ~callback_ostream_streambuf() override
            {
                callback_ostream_streambuf::sync();
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

        template <class T = char>
        class null_ostream_streambuf : public std::basic_streambuf<T>
        {
        public:
            using base_type = std::basic_streambuf<T>;
            using char_type = typename std::basic_streambuf<T>::char_type;
            using traits_type = typename std::basic_streambuf<T>::traits_type;
            using int_type = typename std::basic_streambuf<T>::int_type;
            using pos_type = typename std::basic_streambuf<T>::pos_type;
            using off_type = typename std::basic_streambuf<T>::off_type;

            char_type buffer_[4096];

            explicit null_ostream_streambuf()
            {
                base_type::setp(buffer_, buffer_, buffer_ + sizeof(buffer_));
            }

            ~null_ostream_streambuf() override
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

    template <class char_type> using callback_ostream_flush = typename streambufs::callback_ostream_streambuf<char_type>::callback_ostream_flush;
    template <class char_type> using callback_ostream = ostream_for_streambuf<char_type, streambufs::callback_ostream_streambuf<char_type>>;
    template <class char_type> using nullout_ostream = ostream_for_streambuf<char_type, streambufs::null_ostream_streambuf<char_type>>;


    using logger_char_type = char;

    class logger final
    {
    public:
        using char_type = logger_char_type;
        using write_callback = callback_ostream_flush<char_type>;
        using callback_ostream = callback_ostream<char_type>;

    private:
        struct resource
        {
            std::mutex write_log_mutex_{};
            event_callback<const char_type*> listeners_{};
        };

        std::shared_ptr<resource> resource_ = std::make_shared<resource>();

    public:
        logger() = default;

        [[nodiscard]]
        event_callback<const char_type*>* listeners() const noexcept
        {
            return &resource_->listeners_;
        }

        [[nodiscard]]
        auto open() const
        {
            return callback_ostream([r = resource_](const char_type* text)
            {
                std::lock_guard lock(r->write_log_mutex_);
                r->listeners_.raise(text);
            });
        }

        [[nodiscard]]
        auto open(const char_type* prefix) const
        {
            std::basic_string<char_type> buffer;
            buffer.reserve(1024);
            buffer.append(prefix);

            return callback_ostream([r = resource_, buffer = std::move(buffer)](const char_type* text) mutable
            {
                const size_t prefixSize = buffer.size();
                std::lock_guard lock(r->write_log_mutex_);

                while (auto c = *text++)
                {
                    buffer += c;
                    if (c == '\n')
                    {
                        r->listeners_.raise(buffer.c_str());
                        buffer.resize(prefixSize);
                    }
                }

                if (buffer.size() != prefixSize)
                {
                    buffer += '\n';
                    r->listeners_.raise(buffer.c_str());
                    buffer.resize(prefixSize);
                }
            });
        }

        static logger* get_default_logger()
        {
            static logger default_logger_instance;
            return &default_logger_instance;
        }
    };

    static inline auto log() { return logger::get_default_logger()->open(); }
    static inline auto log(const char* section) { return logger::get_default_logger()->open(section); }
}
