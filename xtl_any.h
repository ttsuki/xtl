/// @file
/// @brief  xtl move only any
/// @author (C) 2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <new>
#include <cstddef>
#include <array>
#include <memory>
#include <variant>
#include <typeindex>
#include <stdexcept>

namespace
XTL_NAMESPACE
{
    // move only any with soo: small object optimization
    class any
    {
        using alignment_t = void*;
        static constexpr inline size_t soo_threshold = sizeof(alignment_t) * 2;
        static inline std::type_index empty_type_index_ = std::type_index(typeid(void));

        struct empty_memory
        {
            empty_memory() = default;
            empty_memory(const empty_memory& other) = delete;
            empty_memory(empty_memory&& other) noexcept = delete;
            empty_memory& operator=(const empty_memory& other) = delete;
            empty_memory& operator=(empty_memory&& other) noexcept = delete;
            ~empty_memory() = default;
        };

        using soo_memory = std::array<alignment_t, soo_threshold / sizeof(alignment_t)>;
        using heap_memory = std::unique_ptr<alignment_t[]>;
        using memory = std::variant<empty_memory, soo_memory, heap_memory>;

        memory memory_{};
        std::type_index type_ = std::type_index{typeid(void)};
        void (*move_constructor_)(memory&, memory&&) noexcept = nullptr;
        void (*destructor_)(memory&) noexcept = nullptr;

        template <class T>
        struct memory_helper
        {
            static inline constexpr bool soo = sizeof(T) <= sizeof(soo_memory);

            static T* pointer(memory& this_)
            {
                if constexpr (soo) return reinterpret_cast<T*>(std::get<soo_memory>(this_).data());
                else return reinterpret_cast<T*>(std::get<heap_memory>(this_).get());
            }

            static const T* pointer(const memory& this_)
            {
                if constexpr (soo) return reinterpret_cast<T*>(std::get<soo_memory>(this_).data());
                else return reinterpret_cast<T*>(std::get<heap_memory>(this_).get());
            }

            template <class U, std::enable_if_t<std::is_constructible_v<T, U&&>>* = nullptr>
            static void construct(memory& this_, U&& object)
            {
                if (!std::holds_alternative<empty_memory>(this_)) throw std::logic_error("precondition failure.");

                // allocate memory
                if constexpr (soo) this_.emplace<soo_memory>();
                else this_.emplace<heap_memory>(std::make_unique<alignment_t[]>((sizeof(T) + sizeof(alignment_t) - 1) / sizeof(alignment_t)));

                // move construct with placement new.
                new(pointer(this_)) T(std::forward<U>(object));
            }

            static void move_construct(memory& this_, memory&& other) noexcept
            {
                if constexpr (soo)
                {
                    memory_helper::construct(this_, std::move(*pointer(other)));
                    memory_helper::destruct(other);
                }
                else
                {
                    this_.emplace<heap_memory>(std::move(std::get<heap_memory>(other)));
                    other.emplace<empty_memory>();
                }
            }

            static void destruct(memory& this_) noexcept
            {
                assert(!std::holds_alternative<empty_memory>(this_));
                pointer(this_)->~T();
                this_.emplace<empty_memory>();
            }
        };

    public:
        constexpr any() noexcept = default;
        any(const any& other) = delete;
        any(any&& other) noexcept { this->reset(std::move(other)); }
        any& operator=(const any& other) = delete;
        any& operator=(any&& other) noexcept { return this->reset(std::move(other)); }

        ~any()
        {
            if (this->destructor_)
            {
                this->destructor_(this->memory_);
                this->type_ = std::type_index(typeid(void));
                this->move_constructor_ = nullptr;
                this->destructor_ = nullptr;
            }
        }

        template <class T, std::enable_if_t<
                      std::is_nothrow_move_constructible_v<T>
                      && !std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, any> // forbid nest any
                  > * = nullptr>
        any(T&& object)
        {
            memory_helper<T>::construct(memory_, std::forward<T>(object));
            type_ = std::type_index(typeid(T));
            move_constructor_ = memory_helper<T>::move_construct;
            destructor_ = memory_helper<T>::destruct;
        }

        [[nodiscard]] std::type_index type_index() const noexcept
        {
            return type_;
        }

        template <class T>
        [[nodiscard]] bool has() const noexcept
        {
            return std::type_index(typeid(T)) == type_;
        }

        template <class T>
        [[nodiscard]] T* get() noexcept
        {
            return has<T>()
                       ? memory_helper<T>::pointer(memory_)
                       : nullptr;
        }

        template <class T>
        [[nodiscard]] const T* get() const noexcept
        {
            return has<T>()
                       ? memory_helper<T>::pointer(memory_)
                       : nullptr;
        }


        any& reset(any&& other = {}) noexcept
        {
            if (std::addressof(other) != this)
            {
                if (this->destructor_)
                {
                    this->destructor_(this->memory_);
                    this->type_ = std::type_index(typeid(void));
                    this->move_constructor_ = nullptr;
                    this->destructor_ = nullptr;
                }

                if (other.move_constructor_)
                {
                    other.move_constructor_(this->memory_, std::move(other.memory_));
                    this->type_ = std::exchange(other.type_, std::type_index(typeid(void)));
                    this->move_constructor_ = std::exchange(other.move_constructor_, nullptr);
                    this->destructor_ = std::exchange(other.destructor_, nullptr);
                }
            }

            return *this;
        }
    };
}
