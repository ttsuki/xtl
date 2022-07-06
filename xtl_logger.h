/// @file
/// @brief  xtl::logger
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <type_traits>
#include <mutex>

#include "xtl_ostream.h"
#include "xtl_event_callback.h"

namespace
XTL_NAMESPACE
{
    using logger_char_type = char;

    enum struct log_severity : int
    {
        emergency = 800,
        alert = 700,
        critical = 600,
        error = 500,
        warning = 400,
        notice = 300,
        informational = 200,
        debug = 100,
    };

    static inline bool operator <(log_severity a, log_severity b) noexcept { return static_cast<std::underlying_type_t<log_severity>>(a) < static_cast<std::underlying_type_t<log_severity>>(b); }
    static inline bool operator <=(log_severity a, log_severity b) noexcept { return static_cast<std::underlying_type_t<log_severity>>(a) <= static_cast<std::underlying_type_t<log_severity>>(b); }
    static inline bool operator >(log_severity a, log_severity b) noexcept { return static_cast<std::underlying_type_t<log_severity>>(a) > static_cast<std::underlying_type_t<log_severity>>(b); }
    static inline bool operator >=(log_severity a, log_severity b) noexcept { return static_cast<std::underlying_type_t<log_severity>>(a) >= static_cast<std::underlying_type_t<log_severity>>(b); }

    class logger final
    {
    public:
        using char_type = logger_char_type;
        using stream_type = basic_o_callback_stream<char_type>;
        using callback_type = basic_o_callback_stream_callback<char_type>;

    private:
        struct resource
        {
            std::mutex write_log_mutex_{};
            event_callback<void(log_severity, const char_type*)> listeners_{};
        };

        std::shared_ptr<resource> resource_ = std::make_shared<resource>();

    public:
        logger() = default;

        [[nodiscard]]
        event_callback<void(log_severity, const char_type*)>* listeners() const noexcept
        {
            return &resource_->listeners_;
        }

        [[nodiscard]]
        auto stream(log_severity severity, const char_type* prefix = nullptr) const
        {
            if (prefix == nullptr)
            {
                return stream_type([severity, r = resource_](const char_type* text)
                {
                    std::lock_guard lock(r->write_log_mutex_);
                    r->listeners_.raise(severity, text);
                });
            }
            else
            {
                std::basic_string<char_type> buffer;
                buffer.reserve(4096);
                buffer.append(prefix);

                return stream_type([severity, r = resource_, buffer = std::move(buffer)](const char_type* text) mutable
                {
                    const size_t prefixSize = buffer.size();
                    std::lock_guard lock(r->write_log_mutex_);

                    while (auto c = *text++)
                    {
                        buffer += c;
                        if (c == '\n')
                        {
                            r->listeners_.raise(severity, buffer.c_str());
                            buffer.resize(prefixSize);
                        }
                    }

                    if (buffer.size() != prefixSize)
                    {
                        buffer += '\n';
                        r->listeners_.raise(severity, buffer.c_str());
                        buffer.resize(prefixSize);
                    }
                });
            }
        }

     
        [[nodiscard]] auto emerg(const char_type* prefix = nullptr) const { return stream(log_severity::emergency, prefix); }
        [[nodiscard]] auto alert(const char_type* prefix = nullptr) const { return stream(log_severity::alert, prefix); }
        [[nodiscard]] auto critical(const char_type* prefix = nullptr) const { return stream(log_severity::critical, prefix); }
        [[nodiscard]] auto error(const char_type* prefix = nullptr) const { return stream(log_severity::error, prefix); }
        [[nodiscard]] auto warn(const char_type* prefix = nullptr) const { return stream(log_severity::warning, prefix); }
        [[nodiscard]] auto notice(const char_type* prefix = nullptr) const { return stream(log_severity::notice, prefix); }
        [[nodiscard]] auto info(const char_type* prefix = nullptr) const { return stream(log_severity::informational, prefix); }
        [[nodiscard]] auto debug(const char_type* prefix = nullptr) const { return stream(log_severity::debug, prefix); }

        [[nodiscard]] static logger* get_default_logger()
        {
            static logger default_logger_instance;
            return &default_logger_instance;
        }
    };
}
