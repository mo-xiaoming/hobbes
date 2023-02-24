/*
 * hash : utilities for hashing data
 */

#ifndef HOBBES_UTIL_HASH_H_INCLUDED
#define HOBBES_UTIL_HASH_H_INCLUDED

#include <cstdint>
#include <map>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace hobbes {

template <typename T> struct genHash {
  [[nodiscard]] inline std::size_t operator()(const T& x) const {
    static const std::hash<T> h;
    return h(x);
  }
};

template <typename T> inline void hashAppend(std::size_t& seed, const T& x) {
  // implementation is from boost::hash_combine,
  // https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
  seed ^= genHash<T>()(x) + 0x9e3779b9 + (seed << 6U) + (seed >> 2U);
}

namespace internal {
template <std::size_t i, typename... Ts>
[[nodiscard]] inline typename std::enable_if<i == sizeof...(Ts), void>::type
hashTuple(std::size_t*, const std::tuple<Ts...>&) {}

template <std::size_t i, typename... Ts>
    [[nodiscard]] inline typename std::enable_if <
    i<sizeof...(Ts), void>::type hashTuple(std::size_t* r, const std::tuple<Ts...>& ms) {
  hashAppend(*r, std::get<i>(ms));
  hashTuple<i + 1, Ts...>(r, ms);
}
} // namespace internal

// hash tuples
template <typename... Ts> struct genHash<std::tuple<Ts...>> {
  [[nodiscard]] inline std::size_t operator()(const std::tuple<Ts...>& xs) const {
    std::size_t r = 0;
    internal::hashTuple<0, Ts...>(&r, xs);
    return r;
  }
};

// hash pairs
template <typename U, typename V> struct genHash<std::pair<U, V>> {
  [[nodiscard]] inline std::size_t operator()(const std::pair<U, V>& x) const {
    std::size_t r = 0;
    hashAppend(r, x.first);
    hashAppend(r, x.second);
    return r;
  }
};

// hash vectors
template <typename T> struct genHash<std::vector<T>> {
  [[nodiscard]] inline std::size_t operator()(const std::vector<T>& xs) const {
    std::size_t r = 0;
    hashAppend(r, xs.size());
    for (const auto& x : xs) {
      hashAppend<T>(r, x);
    }
    return r;
  }
};
} // namespace hobbes

#endif
