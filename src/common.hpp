#ifndef EOMAP_COMMON_HPP
#define EOMAP_COMMON_HPP

#include <array>
#include <cstdint>
#include <experimental/optional>
#include <functional>
#include <memory>
#include <string>

using std::array;
using std::pair;
using std::string;
using std::size_t;

using std::experimental::optional;
using std::experimental::nullopt;

template <class T>
using opt = optional<T>;

template <class T>
using refwrap = std::reference_wrapper<T>;

template <class T>
using optref = optional<refwrap<T>>;

template <class... Args>
using s_ptr = std::shared_ptr<Args...>;

template <class... Args>
using w_ptr = std::weak_ptr<Args...>;

template <class... Args>
using u_ptr = std::unique_ptr<Args...>;

template <class... Args>
using func = std::function<Args...>;

using namespace std::placeholders;

#endif // EOMAP_COMMON_HPP
