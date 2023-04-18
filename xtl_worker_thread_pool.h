/// @file
/// @brief  xtl::worker_thread_pool
/// @author ttsuki

#pragma once

#include <string>
#include <string_view>
#include <thread>
#include <functional>
#include <future>

#include "xtl_delegate.h"
#include "xtl_concurrent_queue.h"

namespace xtl
{
    /// makes move-only packaged_task-like object.
    /// returns the pair [task_body, future]
    template <class Callable, class... Args>
    [[nodiscard]] static auto make_async_task(Callable callable, Args... args)
        -> std::pair<xtl::delegate<void()>, std::future<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>>>
    {
        using R = std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>;

        // packaged_task can be used only if Callable and Args are copyable.
        //std::packaged_task<R()> task{std::bind(std::forward<Callable>(callable), std::forward<Args>(args)...)};
        //std::future<R> future = task.get_future();

        // xtl::delegate supports move-only Callable and Args.
        std::promise<R> promise;
        std::future<R> future = promise.get_future();
        xtl::delegate<void()> task = [promise = std::move(promise), callable = std::forward<Callable>(callable), args = std::make_tuple(std::forward<Args>(args)...)]() mutable
        {
            if constexpr (!std::is_same_v<R, void>)
            {
                try { promise.set_value(std::apply(callable, args)); }
                catch (...) { promise.set_exception(std::current_exception()); }
            }
            else
            {
                try
                {
                    std::apply(callable, args);
                    promise.set_value();
                }
                catch (...) { promise.set_exception(std::current_exception()); }
            }
        };

        return std::pair<xtl::delegate<void()>, std::future<R>>{std::move(task), std::move(future)};
    }

    /// worker thread
    class worker_thread_pool final
    {
        std::vector<std::thread> threads_{};
        concurrent_queue<delegate<void()>> task_queue_{};

    public:
        worker_thread_pool(const worker_thread_pool& other) = delete;
        worker_thread_pool(worker_thread_pool&& other) noexcept = delete;
        worker_thread_pool& operator=(const worker_thread_pool& other) = delete;
        worker_thread_pool& operator=(worker_thread_pool&& other) noexcept = delete;

        static inline constexpr auto default_thread_factory_function = [](std::string_view /*label*/, auto function_body, auto... args) { return std::thread(std::move(function_body), std::move(args)...); };

        template <class thread_factory_function = decltype(default_thread_factory_function)>
        worker_thread_pool(
            size_t thread_count /* = 4 */,
            std::string_view label = "",
            thread_factory_function create_thread_function = default_thread_factory_function)
        {
            for (size_t i = 0; i < thread_count; i++)
            {
                threads_.emplace_back(create_thread_function(label, [this]
                {
                    while (auto f = task_queue_.pop_wait())
                        try { (*f)(); }
                        catch (...) { /* ignore */ }
                }));
            }
        }

        ~worker_thread_pool()
        {
            task_queue_.close();
            for (auto& thread : threads_)
                thread.join();
        }

        template <class Callable, class... Args>
        [[nodiscard]] auto async(Callable&& callable, Args&&... args) -> std::future<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>>
        {
            auto [body, future] = xtl::make_async_task(std::forward<Callable>(callable), std::forward<Args>(args)...);
            task_queue_.push(std::move(body));
            return std::move(future);
        }
    };
}
