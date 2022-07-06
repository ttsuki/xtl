/// @file
/// @brief  xtl::event_callback
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <mutex>
#include <utility>
#include <functional>

namespace
XTL_NAMESPACE
{
    template <class F> class event_callback;

    template <class...TArgs>
    class event_callback<void(TArgs ...)> final
    {
    public:
        using subscribe_id = const void*;
        using callback = std::function<void(TArgs ...)>;

    private:
        mutable std::mutex mutex_{};

        struct entry
        {
            subscribe_id id{};
            callback callback{};
            int priority{};
            std::unique_ptr<std::byte> idu{};
        };

        std::vector<entry> functions_{};

    public:
        event_callback() = default;
        event_callback(const event_callback& other) = delete;
        event_callback(event_callback&& other) noexcept = delete;
        event_callback& operator=(const event_callback& other) = delete;
        event_callback& operator=(event_callback&& other) noexcept = delete;
        ~event_callback() = default;

        [[nodiscard]] size_t count() const noexcept
        {
            std::lock_guard lock(mutex_);
            return functions_.size();
        }

        [[nodiscard]] bool empty() const noexcept
        {
            std::lock_guard lock(mutex_);
            return functions_.empty();
        }

        void unsubscribe_all() noexcept
        {
            std::lock_guard lock(mutex_);
            return functions_.clear();
        }

        subscribe_id subscribe(callback f, int priority = 0)
        {
            auto p = std::make_unique<std::byte>();
            auto id = p.get();

            std::lock_guard lock(mutex_);
            functions_.emplace_back(entry{id, std::move(f), priority, std::move(p)});
            std::stable_sort(functions_.begin(), functions_.end(), [](auto&& a, auto&& b) { return a.priority < b.priority; });
            return id;
        }

        void subscribe(subscribe_id id, callback f, int priority = 0)
        {
            std::lock_guard lock(mutex_);
            functions_.emplace_back(entry{id, std::move(f), priority, nullptr});
            std::stable_sort(functions_.begin(), functions_.end(), [](auto&& a, auto&& b) { return a.priority < b.priority; });
        }

        bool unsubscribe(subscribe_id id)
        {
            std::lock_guard lock(mutex_);
            for (auto it = functions_.begin(); it != functions_.end(); ++it)
            {
                if (it->id == id)
                {
                    functions_.erase(it);
                    return true;
                }
            }
            return false;
        }

        template <class...Args>
        void raise(Args&&...params) const
        {
            std::lock_guard lock(mutex_);
            if (functions_.empty())
            {
                // do nothing
            }
            else if (functions_.size() == 1)
            {
                // can use move semantic.
                functions_.front().callback(std::forward<Args>(params)...);
            }
            else
            {
                // can't use move semantic.
                for (auto& f : functions_) f.callback(params...);
            }
        }
    };
}
