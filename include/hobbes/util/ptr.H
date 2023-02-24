
#ifndef HOBBES_UTIL_PTR_HPP_INCLUDED
#define HOBBES_UTIL_PTR_HPP_INCLUDED

#include <hobbes/util/hash.H>

#include <functional>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace hobbes {

namespace internal {

/**
 * @brief this "map" caches all values of @a Type.
 *  All same genHash(ctor_args_for_Type_in_order) are considered to be the same value
 * (is it risky?)
 */
template <typename Type, typename... Args> class UniqueTypeValueCache {
public:
  using object_type = Type;

  /**
   * @brief get the shared_ptr of a @a Type object.
   * will be created in this cache if it hasn't been created.
   *
   * @param mk functor to creates @a Type
   *
   * @param args arguments for @a Type constructor
   *
   * @return an instance of this @a Type
   */
  const std::shared_ptr<Type>& get(const std::function<Type*(Args...)>& mk, const Args&... args) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto k = std::tuple<Args...>(args...);
    auto& r = this->values[k];
    if (!r) {
      r.reset(mk(args...));
    }
    return r;
  }

  /**
   * @brief remove all values that is not being reference.
   *
   * @return the number of removed values
   */
  size_t compact() {
    size_t c = 0;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    for (auto v = this->values.cbegin(); v != this->values.cend();) {
      if (v->second.use_count() == 1) {
        v = this->values.erase(v);
        ++c;
      } else {
        ++v;
      }
    }
    return c;
  }

private:
  std::recursive_mutex mutex;
  using Values =
      std::unordered_map<std::tuple<Args...>, std::shared_ptr<Type>, genHash<std::tuple<Args...>>>;
  Values values;
};

/**
 * @brief call \c UniqueTypeValueCache.compact() on all @a Caches
 */
template <size_t i, typename... Caches>
inline typename std::enable_if<i == sizeof...(Caches), size_t>::type
compactAll(std::tuple<Caches...>&) {
  return 0;
}

template <size_t i, typename... Caches>
    inline typename std::enable_if <
    i<sizeof...(Caches), size_t>::type compactAll(std::tuple<Caches...>& ms) {
  return std::get<i>(ms).compact() + compactAll<i + 1, Caches...>(ms);
}

/**
 * @brief get the index of a Type in @a Ts
 * Type must exists in Ts
 */
template <std::size_t i, typename... Ts> struct TypeIndex;

template <std::size_t i, typename T, typename... Ts> struct TypeIndex<i, T, T, Ts...> {
  static const std::size_t value = i;
};

template <std::size_t i, typename T, typename U, typename... Ts> struct TypeIndex<i, T, U, Ts...> {
  static const size_t value = TypeIndex<i + 1, T, Ts...>::value;
};

/**
 * @brief it stores all \c UniqueTypeValueCache
 */
template <typename... Caches> class AllUniqueTypeValueCaches {
  std::tuple<Caches...> caches;

public:
  template <typename T>
  [[nodiscard]]
  typename std::tuple_element<TypeIndex<0, T, Caches...>::value, std::tuple<Caches...>>::type&
  at() {
    return std::get<TypeIndex<0, T, Caches...>::value>(this->caches);
  }

  size_t compact() {
    size_t tc = 0;
    while (true) {
      const size_t c = compactAll<0>(this->caches);
      tc += c;
      if (c == 0) {
        break;
      }
    }
    return tc;
  }
};
} // namespace internal

template <typename V, typename U> V scast (U x)        { return static_cast<V>(x); }
template <typename V, typename U> V rcast (U x)        { return reinterpret_cast<V>(x); }
template <typename V, typename U> V crcast(const U* x) { return reinterpret_cast<V>(const_cast<U*>(x)); }

template <typename T>
  T align(T x, T m) {
    if (m == 0 || (x % m) == 0) {
      return x;
    } else {
      return (1 + (x / m)) * m;
    }
  }

template <typename T, typename S>
  const T* is(const S* s) {
    if (s && s->case_id() == T::type_case_id) {
      return rcast<const T*>(s);
    } else {
      return nullptr;
    }
  }

template <typename T, typename S>
  T* isM(S* s) {
    if (s && s->case_id() == T::type_case_id) {
      return rcast<T*>(s);
    } else {
      return nullptr;
    }
  }

template <typename T, typename S>
  T* is(const std::shared_ptr<S>& s) {
    return isM<T, S>(s.get());
  }

}  // namespace hobbes

#endif
