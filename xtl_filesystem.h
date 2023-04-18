/// @file
/// @brief  xtl::filesystem
/// @author ttsuki

#pragma once

#include <ios>
#include <string>
#include <filesystem>
#include <fstream>
#include <optional>

#include <functional>
#include <regex>

#include "xtl_value_or_error.h"
#include "xtl_functional.h"
#include "xtl_enum_struct_bitwise_operators.h"

namespace xtl::filesystem
{
    typedef int errno_t;

    [[nodiscard]] static inline value_or_error<std::string, errno_t> read_file_whole(const std::filesystem::path& path)
    {
        std::ifstream ifs(path, std::ios::in | std::ios::binary | std::ios::ate);
        if (ifs.fail()) { return static_cast<errno_t>(errno); }

        const std::ifstream::pos_type file_size = ifs.tellg();
        std::string data(file_size, '\0');
        ifs.seekg(0, std::ios::beg);
        ifs.read(data.data(), static_cast<std::streamsize>(data.size()));
        if (ifs.fail()) { return static_cast<errno_t>(errno); }

        return data;
    }

    [[nodiscard]] static inline value_or_error<decltype(std::ignore), errno_t> write_file_whole(const std::filesystem::path& path, const void* data, size_t length)
    {
        std::ofstream ofs(path, std::ios::out | std::ios::trunc | std::ios::binary);
        if (ofs.fail()) { return static_cast<errno_t>(errno); }
        ofs.write(static_cast<const char*>(data), static_cast<std::streamsize>(length));
        if (ofs.fail()) { return static_cast<errno_t>(errno); }
        return decltype(std::ignore){};
    }

    using std::filesystem::path;
    using std::filesystem::directory_iterator;

    template <class F, std::enable_if_t<std::is_invocable_v<F, directory_iterator>>* = nullptr>
    static void enumerate_entries_in_directory(const path& root, F&& callback, bool recursive = false)
    {
        xtl::with_fixed([&root, &callback, recursive](auto&& f, const path& prefix = {}) -> void
        {
            for (auto it = directory_iterator(prefix);
                 it != directory_iterator();
                 ++it)
            {
                callback(it);
                if (recursive && it->is_directory())
                    f(prefix / it->path().filename());
            }
        })(root);
    }

    template <class F, std::enable_if_t<std::is_invocable_v<F, directory_iterator>>* = nullptr>
    static void enumerate_entries_in_recursive(const path& root, F&& callback)
    {
        return filesystem::enumerate_entries_in_directory(root, std::forward<F>(callback), true);
    }

    enum struct file_type : unsigned int
    {
        none = 0,
        others = 1 << 0,
        regular_file = 1 << 1,
        directory = 1 << 2,
        any = static_cast<unsigned int>(-1),

        regular_file_or_directory = regular_file | directory,
    };

    XTL_enable_enum_struct_bitwise_operators(file_type);

    static inline bool it_is(const directory_iterator& it, file_type types)
    {
        file_type i{};
        if (it->is_regular_file()) i |= file_type::regular_file;
        else if (it->is_directory()) i |= file_type::directory;
        else i |= file_type::others;
        return !!(i & types);
    }

    template <class Char>
    struct path_regex
    {
        using char_type = Char;
        using regex_type = std::basic_regex<Char>;
        regex_type regex;

        [[nodiscard]] bool filename_match(const path& path) const { return std::regex_match(path.filename().generic_string<Char>(), regex); }
        [[nodiscard]] bool filename_match(const directory_iterator& it) const { return filename_match(it->path()); }
    };

    /// Converts file-path-wildcards to regex-expression.
    /// `*.png` -> `.*\.png`
    template <class String, class char_type = typename decltype(std::basic_string_view(std::declval<std::decay_t<String>>()))::value_type>
    [[nodiscard]] static inline path_regex<char_type> make_regex_for_filesystem_wildcards(String&& pattern_string)
    {
        std::basic_string_view pattern_view = pattern_string;
        std::basic_string<char_type> regex;

        regex.reserve(pattern_view.size() * 2);
        regex += '^';
        for (char_type c : pattern_view)
        {
            if (c == '\\') { regex += '\\'; }
            if (c == '.') { regex += '\\'; }
            if (c == '*') { regex += '.'; }
            if (c == '?') { regex += '.'; }
            regex += c;
        }
        regex += '$';
        return path_regex<char_type>{std::basic_regex<char_type>(regex)};
    }

    template <class char_type>
    [[nodiscard]] static inline auto filename_match(const directory_iterator& it, const path_regex<char_type>& pattern_regex)
    {
        return pattern_regex.filename_match(it);
    }
}
