#pragma once

#include "buffer_utils.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

namespace ex {
namespace _buffer_ {
template <typename, typename = void> constexpr bool is_iterable{};
template <typename T>
constexpr bool is_iterable<
    T,
    std::void_t<
        decltype(std::declval<T>().begin()), decltype(std::declval<T>().end()),
        decltype(std::declval<T>().size()), decltype(std::declval<T>().data()),
        typename std::remove_reference<T>::type::value_type>> = true;

template <typename, typename = void> constexpr bool has_value_type{};
template <typename T>
constexpr bool has_value_type<T, std::void_t<typename T::value_type>> = true;

constexpr auto vxxx = has_value_type<std::vector<uint8_t>>;
constexpr auto xxxx = has_value_type<std::string::value_type>;

} // namespace _buffer_

using vector_u8 = std::vector<uint8_t>;
class buffer : public vector_u8 {
public:
  using vector_u8::vector_u8;

  static buffer from(std::initializer_list<uint8_t> t) {
    return from(t.begin(), t.size());
  }

  template <typename Ptr, std::enable_if_t<std::is_pointer_v<Ptr>, bool> = true>
  static buffer from(Ptr p, size_t size) {
    buffer b(size);
    std::copy(p, p + size, b.begin());
    return b;
  }

  template <typename Container,
            std::enable_if_t<_buffer_::is_iterable<Container>, bool> = true>
  static buffer from(Container &&c) {
    using type = typename std::remove_reference<Container>::type::value_type;
    constexpr auto size = sizeof(type);
    buffer b(c.size() * size);
    std::copy(c.begin(), c.end(), b.begin());
    return b;
  }
  template <typename Container,
            std::enable_if_t<_buffer_::is_iterable<Container>, bool> = true>
  static buffer from(Container &&c, size_t byte_length) {
    buffer b(byte_length);
    auto p = (uint8_t *)(c.data());
    std::copy(p, p + byte_length, b.data());
    return b;
  }

  template <typename Arr, size_t N> static buffer from(Arr (&a)[N]) {
    if constexpr (std::is_same_v<Arr, const char>) {
      auto size = N - 1;
      buffer b(size);
      memcpy(b.data(), a, size);
      return b;
    } else {
      auto size = sizeof(Arr[N]);
      buffer b(size);
      memcpy(b.data(), a, size);
      return b;
    }
  }

  template <typename Str,
            std::enable_if_t<std::is_same_v<const char *, Str>, bool> = true>
  static buffer from(Str str) {
    return from(str, strlen(str));
  }

  template <typename Num,
            std::enable_if_t<std::is_arithmetic_v<Num>, bool> = true>
  static buffer from(Num n) {
    buffer b(sizeof(Num));
    auto p = (uint8_t *)&n;
    std::copy(p, p + sizeof(Num), b.data());
    return b;
  }

  static buffer from_hex(const std::string &str) {
    auto len = str.size() / 2;
    if (len % 2)
      ++len;
    buffer v(len);
    v.write_hex(str);
    return v;
  }

  template <typename T> void write_le(T v, size_t offset = 0) {
    *reinterpret_cast<T *>(data() + offset) = v;
  }

  template <typename T> void write_be(T v, size_t offset = 0) {
    auto p = data() + offset;
    *reinterpret_cast<T *>(p) = v;
    std::reverse(p, p + sizeof(T));
  }

  template <typename T> T read_le(size_t offset = 0) {
    return *reinterpret_cast<T *>(data() + offset);
  }

  template <typename T> T read_be(size_t offset = 0) {
    return buffer_switch_endian(read_le<T>(offset));
  }

  template <typename T> void fill(T *p, size_t offset, size_t size) {
    std::copy(p, p + size, begin() + offset);
  }

  template <typename T>
  void fill(std::initializer_list<T> t, size_t offset = 0) {
    return fill(t.begin(), offset, t.size());
  }
  template <typename T> void fill(T &t) { fill(t, 0, t.size()); }
  template <typename T> void fill(T &t, size_t offset) {
    fill(t, offset, t.size());
  }
  template <typename T> void fill(T &t, size_t offset, size_t size) {
    auto p = t.begin();
    std::copy(p, p + size, begin() + offset);
  }
  template <typename T, size_t N> void fill(T (&t)[N]) { fill(t, 0, N); }
  void fill(const char *str) { fill(str, 0, strlen(str)); }

  void write_hex(std::string hex, size_t offset = 0,
                 bool skip_splitters_remove = false) {
    if (!skip_splitters_remove)
      hex.erase(std::remove_if(hex.begin(), hex.end(),
                               [](char c) {
                                 if (c < '0')
                                   return true;
                                 if (c <= '9')
                                   return false;
                                 if (c < 'A')
                                   return true;
                                 if (c <= 'F')
                                   return false;
                                 if (c < 'a')
                                   return true;
                                 if (c <= 'f')
                                   return false;
                                 return true;
                               }),
                hex.end());

    auto hlen = hex.size();
    auto len = hlen / 2;
    auto odd = hlen % 2;
    for (size_t i = 0; i < len; ++i) {
      (*this)[offset + len - i - 1 + odd] = static_cast<uint8_t>(
          std::stoi(hex.substr(hlen - 2 - i * 2, 2), nullptr, 16));
    }
    if (odd)
      (*this)[offset] =
          static_cast<uint8_t>(std::stoi(hex.substr(0, 1), nullptr, 16));
  }

  std::string read_hex(size_t offset, size_t size = 0,
                       std::string splitter = "") {
    if (!size)
      size = this->size() - offset;
    size_t splen = splitter.size();
    size_t slen = size * 2 + (size - 1) * splen;
    auto spblen = splen + 1;
    char *c = new char[slen + 1];
    const char *fmt = "%02x";
    for (size_t i = 0; i < size; ++i) {
      auto p = c + i * (2 + splen);
      std::snprintf(p, 3, "%02x", at(i + offset));
      std::snprintf(p + 2, spblen, "%s", splitter.c_str());
    }
    std::string s(c, slen);
    delete[] c;
    return s;
  }

  std::string to_string() { return std::string(begin(), end()); }
  auto to_hex_string(const std::string &splitter = "") const {
    return buffer_read_hex((void *)data(), size(), splitter);
  }

  void write_hex(std::string hex, size_t offset = 0,
                 bool skip_splitters_remove = false) const {
    buffer_write_hex((void *)(data() + offset), hex, skip_splitters_remove);
  }

  std::string read_hex(size_t offset, size_t size = 0,
                       std::string splitter = "") const {
    return buffer_read_hex((void *)(data() + offset), size, splitter);
  }
};
} // namespace ex

inline std::ostream &operator<<(std::ostream &os, ex::buffer &buffer) {
  os << "Buffer { ";
  for (auto u : buffer) {
    os << std::setfill('0') << std::setw(2) << std::hex << (int)u << ' ';
  }
  os << '}';
  return os;
}