/// @file
/// @brief  xtl::producer_consumer_queue
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <mutex>
#include <condition_variable>
#include <optional>
#include <queue>
#include <utility>
#include <limits>

namespace
XTL_NAMESPACE
{
    template <class T>
    class producer_consumer_queue final
    {
    public:
        enum struct policy
        {
            block_push_if_full,
            drop_last_if_full,
            drop_first_if_full,
        };

    private:
        mutable std::recursive_mutex mutex_{};
        const size_t limit_;
        const policy dropPolicy_;
        std::condition_variable_any can_produce_;
        std::condition_variable_any can_consume_;
        std::queue<std::optional<T>> queue_{};

    public:
        producer_consumer_queue(
            size_t limit = std::numeric_limits<size_t>::max(),
            policy mode = policy::block_push_if_full)
            : limit_(limit)
            , dropPolicy_(mode)
        {
            switch (mode)
            {
            case policy::block_push_if_full:
            case policy::drop_last_if_full:
            case policy::drop_first_if_full:
                break;
            default:
                throw std::invalid_argument("mode");
            }
        }

        [[nodiscard]] bool closed() const noexcept
        {
            std::lock_guard lock(mutex_);
            return !queue_.empty() && !queue_.front();
        }

        [[nodiscard]] bool empty() const noexcept
        {
            std::lock_guard lock(mutex_);
            return closed() || queue_.empty();
        }

        /// Pushes value.
        template <class... U>
        bool push(U&& ...val) noexcept
        {
            std::unique_lock<decltype(mutex_)> lock(mutex_);
            if (closed()) { return false; }
            if (dropPolicy_ == policy::block_push_if_full) { can_produce_.wait(lock, [&] { return queue_.size() < limit_; }); }
            if (dropPolicy_ == policy::drop_last_if_full) { if (queue_.size() == limit_) { return false; } }
            if (dropPolicy_ == policy::drop_first_if_full) { if (queue_.size() == limit_) { queue_.pop(); } }
            queue_.emplace(std::forward<U>(val)...);
            can_consume_.notify_all();
            return true;
        }

        /// Closes queue.
        void close() noexcept
        {
            std::lock_guard lock(mutex_);
            queue_.emplace(std::nullopt);
            can_consume_.notify_all();
        }

        /// Tries pop value, may returns nullopt if queue is empty or closed.
        [[nodiscard]] std::optional<T> try_pop() noexcept
        {
            std::lock_guard lock(mutex_);
            if (closed() || queue_.empty()) return std::nullopt;

            std::optional<T> ret = std::move(queue_.front());
            queue_.pop();
            can_produce_.notify_all();
            return std::move(ret);
        }

        /// Waits for value.
        /// may return nullopt if queue is closed.
        [[nodiscard]] std::optional<T> pop_wait() noexcept
        {
            std::unique_lock lock(mutex_);
            std::optional<T> ret = std::nullopt;
            can_consume_.wait(lock, [&] { return closed() || (ret = try_pop()).has_value(); });
            return std::move(ret);
        }
    };
}
