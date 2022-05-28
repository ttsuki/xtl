/// @file
/// @brief  xtl simple worker thread
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <atomic>
#include <string>
#include <thread>
#include <functional>
#include <future>

#include "xtl_producer_consumer_queue.h"

namespace
XTL_NAMESPACE
{
    template <class TTag>
    class thread_factory
    {
    public:
        using thread_prologue = std::function<void(const std::string& label, std::thread::id thread_id)>;
        using thread_epilogue = std::function<void(const std::string& label, std::thread::id thread_id)>;

    private:
        static inline std::atomic<std::pair<thread_prologue, thread_epilogue>> envelope_function;

    public:
        template <class F, class... Args>
        static std::thread create(const std::string& label, F&& function, Args&&...args)
        {
            return std::thread(
                [
                    f = std::forward<F>(function),
                    e = get_hook_functions(),
                    label = label
                ](Args&&...args) mutable
                {
                    if (e.first) { e.first(label, std::this_thread::get_id()); }
                    f(std::forward<Args>(args)...);
                    if (e.second) { e.second(label, std::this_thread::get_id()); }
                });
        }

        static std::pair<thread_prologue, thread_epilogue> get_hook_functions()
        {
            return envelope_function.load();
        }

        static void set_hook_function(thread_prologue prologue, thread_epilogue epilogue)
        {
            envelope_function.store(std::make_pair(std::move(prologue), std::move(epilogue)));
        }
    };

    struct default_thread_factory_tag;
    using default_thread_factory = thread_factory<default_thread_factory_tag>;

    /// an alternative to packaged_task
    /// returns [body, future]
    template <class Callable, class...Args>
    static auto make_async_task(Callable&& callable, Args&&...args)
    -> std::pair<std::function<void()>, std::future<std::invoke_result_t<Callable, Args...>>>
    {
        using R = std::invoke_result_t<Callable, Args...>;
        auto promise = std::make_shared<std::promise<R>>();
        auto future = promise->get_future();
        auto inner = std::bind(std::forward<Callable>(callable), std::forward<Args>(args)...);

        if constexpr (std::is_same_v<R, void>)
        {
            return {
                [promise = std::move(promise), inner = std::move(inner)]() mutable
                {
                    try { inner(), promise->set_value(); }
                    catch (...) { promise->set_exception(std::current_exception()); }
                },
                std::move(future)
            };
        }
        else
        {
            return {
                [promise = std::move(promise), inner = std::move(inner)]() mutable
                {
                    try { promise->set_value(inner()); }
                    catch (...) { promise->set_exception(std::current_exception()); }
                },
                std::move(future)
            };
        }
    }

    /// worker thread
    class worker_thread final
    {
        std::vector<std::thread> threads_{};
        producer_consumer_queue<std::function<void()>> task_queue_{};

    public:
        worker_thread(const worker_thread& other) = delete;
        worker_thread(worker_thread&& other) noexcept = delete;
        worker_thread& operator=(const worker_thread& other) = delete;
        worker_thread& operator=(worker_thread&& other) noexcept = delete;

        template <class thread_factory = default_thread_factory>
        worker_thread(const std::string& label, size_t thread_count = 1, thread_factory factory = default_thread_factory{})
        {
            for (size_t i = 0; i < thread_count; i++)
                threads_.emplace(factory.create(label, [this]
                {
                    while (auto f = task_queue_.pop_wait())
                        try
                        {
                            (*f)();
                        }
                        catch (...)
                        {
                        }
                }));
        }

        ~worker_thread()
        {
            task_queue_.close();
            for (auto& thread : threads_)
                thread.join();
        }

        void post_and_forget(std::function<void()> f)
        {
            task_queue_.push(std::move(f));
        }

        template <class Callable, class...Args>
        [[nodiscard]] auto async(Callable&& callable, Args&&...args) -> std::future<std::invoke_result_t<Callable, Args...>>
        {
            auto [body, future] = make_async_task(std::forward<Callable>(callable), std::forward<Args>(args)...);
            this->post_and_forget(body);
            return future;
        }
    };
}
