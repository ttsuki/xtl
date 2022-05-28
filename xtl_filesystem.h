/// @file
/// @brief  xtl filesystem
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <ios>
#include <fstream>
#include <sstream>
#include <optional>
#include <string>

#include <filesystem>
#include <functional>
#include <regex>

#include "xtl_value_or_error.h"

namespace
XTL_NAMESPACE::filesystem
{
    static inline value_or_error<std::string, errno_t> read_file_whole(const std::wstring& path)
    {
        std::ifstream ifs(path, std::ios::in | std::ios::binary | std::ios::ate);
        if (ifs.fail()) { return static_cast<errno_t>(errno); }

        std::string data(ifs.tellg(), '\0');
        ifs.seekg(0, std::ios::beg);
        ifs.read(data.data(), static_cast<std::streamsize>(data.size()));
        if (ifs.fail()) { return static_cast<errno_t>(errno); }

        return data;
    }

    static inline value_or_error<decltype(std::ignore), errno_t> write_file_whole(const std::wstring& path, const void* data, size_t length)
    {
        std::ofstream ofs(path, std::ios::out | std::ios::trunc | std::ios::binary);
        if (ofs.fail()) { return static_cast<errno_t>(errno); }
        ofs.write(static_cast<const char*>(data), length);
        if (ofs.fail()) { return static_cast<errno_t>(errno); }
        return decltype(std::ignore){};
    }

    /// Converts file-path-wildcards to regex-expression.
    /// `*.png` -> `.*\.png`
    template <class TChar = char>
    static inline std::basic_regex<TChar> get_regex_for_filesystem_wildcards(std::basic_string_view<TChar> pattern)
    {
        std::basic_string<TChar> regex;
        regex.reserve(pattern.size() * 2);
        regex += '^';
        for (TChar c : pattern)
        {
            if (c == '\\') { regex += '\\'; }
            if (c == '.') { regex += '\\'; }
            if (c == '*') { regex += '.'; }
            if (c == '?') { regex += '.'; }
            regex += c;
        }
        regex += '$';
        return std::basic_regex<TChar>(regex);
    }

    template std::regex get_regex_for_filesystem_wildcards(std::string_view pattern);
    template std::wregex get_regex_for_filesystem_wildcards(std::wstring_view pattern);

    /// Gets file/directory list in specified directory
    template <class TChar>
    [[nodiscard]] static std::vector<std::basic_string<TChar>>
    enumerate_entry_in_directory(
        std::basic_string_view<TChar> directory,
        std::optional<std::basic_string_view<TChar>> pattern = std::nullopt,
        bool regular_file_only = false,
        bool directory_only = false)
    {
        std::vector<std::basic_string<TChar>> result{};

        std::optional<std::basic_regex<TChar>> regex = std::nullopt;
        std::match_results<typename std::basic_string<TChar>::iterator> m{};
        if (pattern) regex = get_regex_for_filesystem_wildcards(*pattern);

        for (auto it = std::filesystem::directory_iterator(directory);
             it != std::filesystem::directory_iterator();
             ++it)
        {
            std::basic_string<TChar> filename = it->path().filename().generic_string<TChar>();
            if (regular_file_only && !it->is_regular_file()) continue;
            if (directory_only && !it->is_directory()) continue;
            if (regex && !std::regex_match(filename.begin(), filename.end(), m, *regex)) continue;
            if (it->is_directory()) filename += static_cast<TChar>('/');
            result.push_back(filename);
        }

        return result;
    }

    template std::vector<std::string> enumerate_entry_in_directory(
        std::string_view dir,
        std::optional<std::string_view> pattern,
        bool regular_file_only,
        bool directory_only);

    template std::vector<std::wstring> enumerate_entry_in_directory(
        std::wstring_view dir,
        std::optional<std::wstring_view> pattern,
        bool regular_file_only,
        bool directory_only);
}
