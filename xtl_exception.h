/// @file
/// @brief  xtl exception helper
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <string>
#include <exception>
#include <stdexcept>

namespace
XTL_NAMESPACE
{
    /// @returns ex.what following nested.what recursively if nested.
    static inline std::string what_is_current_exception() noexcept
    {
        std::string resultMessage;
        try
        {
            std::rethrow_exception(std::current_exception());
        }
        catch (const std::exception& ex)
        {
            resultMessage += ex.what();
            try
            {
                std::rethrow_if_nested(ex);
            }
            catch (...)
            {
                resultMessage += "\n  inner: " + what_is_current_exception();
            }
        }
        catch (...)
        {
            resultMessage += "unknown type, which is not std::exception, is thrown.";
        }

        return resultMessage;
    }

    /// Represents failure
    struct failure : std::runtime_error
    {
        failure(const char* message)
            : std::runtime_error(message)
        {
        }

        void rethrow_inner_exception() const
        {
            std::rethrow_if_nested(*this);
        }
    };

    /// <pre>
    /// try
    /// {
    ///   xtl::wrap_failure_if_thrown("InitializeSomething", [&] {
    ///      DoInitializeSomething(...);
    ///   });
    ///
    ///   xtl::wrap_failure_if_thrown("DoSomething", [&] {
    ///      DoSomething(...);
    ///   });
    /// }
    /// catch (const failure& f)
    /// {
    ///   // Logs it.
    ///   std::cerr << "Exception is thrown: " << xtl::what_is_current_exception() << std::endl;
    ///
    ///   // Extracts inner exception
    ///   try { f.rethrow_inner_exception(); }
    ///   catch (const SomeException& inner) // gets real exception.
    ///   {
    ///     ... Write code for handling SomeException type here ...
    ///   }
    ///   // Other exception are thrown to outer.
    /// }
    /// </pre>
    template <class F>
    static inline auto wrap_failure_if_thrown(const char* section, F&& f) -> std::invoke_result_t<F>
    {
        try
        {
            return f();
        }
        catch (...)
        {
            std::throw_with_nested(failure((std::string("failure on ") + section).c_str()));
        }
    }
}
