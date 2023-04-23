/// @file
/// @brief  xtl::stdc++ - includes all standard C++ headers.
/// @author ttsuki

#pragma once

#ifdef _MSVC_LANG
#define XTL_cplusplus (_MSVC_LANG)
#else
#define XTL_cplusplus (__cplusplus)
#endif

#if XTL_cplusplus >= 201402L
#define XTL_cplusplus14 XTL_cplusplus
#endif

#if XTL_cplusplus >= 201703L
#define XTL_cplusplus17 XTL_cplusplus
#endif

#if XTL_cplusplus >= 202002L
#define XTL_cplusplus20 XTL_cplusplus
#endif

// C library wrappers
#include <cstddef>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cfloat>
#include <climits>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cuchar>
#include <cwchar>
#include <cwctype>
#include <clocale>
#include <ctime>
#include <cassert>
#include <cerrno>

// Language support library
#include <limits>
#include <new>
#include <typeinfo>
#include <exception>
#include <initializer_list>
#ifdef XTL_cplusplus20
#include <compare> // C++20
#include <source_location> // C++20
#include <version> // C++20
#include <coroutine> // C++20
#endif

// Concepts library
#ifdef XTL_cplusplus20
#include <concepts> // C++20
#endif

// Diagnostics library
#include <stdexcept>
#include <system_error>

// General utilities library
#include <utility>
#include <typeindex>
#include <type_traits>
#include <functional>
#include <memory>
#include <scoped_allocator>
#include <ratio>
#include <chrono>
#include <bitset>
#include <tuple>
#ifdef XTL_cplusplus17
#include <optional> // C++17
#include <variant> // C++17
#include <any> // C++17
#include <memory_resource> //C++17
#include <execution> // C++17
#endif

// Strings library
#include <string>
#ifdef XTL_cplusplus17
#include <charconv> // C++17
#include <string_view> // C++17
#endif
#ifdef XTL_cplusplus20
#include <format>
#endif
#include <locale>
#include <regex>

// Containers library
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#ifdef XTL_cplusplus20
#include <span> // C++20
#endif

// Algorithms library
#include <iterator>
#include <algorithm>
#ifdef XTL_cplusplus20
#include <ranges> // C++20
#endif

// Numerics library
#include <cmath>
#include <numeric>
#include <complex>
#include <random>
#include <valarray>
#ifdef XTL_cplusplus20
#include <bit> // C++20
#include <numbers> // C++20
#endif

// Input/output library
#include <ios>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#ifdef XTL_cplusplus17
#include <filesystem> // C++17
#endif
#ifdef XTL_cplusplus20
#include <syncstream> // C++20
#endif

// Thread support library
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <future>
#ifdef XTL_cplusplus14
#include <shared_mutex> // C++14
#endif
#ifdef XTL_cplusplus20
#include <barrier> // C++20
#include <latch> // C++20
#include <semaphore> // C++20
#include <stop_token> // C++20
#endif
