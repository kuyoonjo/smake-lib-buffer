#pragma once

#include "shared_buffer.h"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <vector>

namespace ex {
class buffer : public shared_buffer {
public:
  buffer(size_t size) : shared_buffer((void *)nullptr, 0), m_buffer(size) {
    m_ptr = m_buffer.data();
    m_size = size;
  }

  buffer(const buffer &o)
      : shared_buffer((void *)nullptr, 0), m_buffer(o.m_buffer) {
    m_ptr = m_buffer.data();
    m_size = m_buffer.size();
  }
  buffer(buffer &&o) noexcept
      : shared_buffer((void *)nullptr, 0), m_buffer(std::move(o.m_buffer)) {
    m_ptr = m_buffer.data();
    m_size = m_buffer.size();
    o.m_ptr = nullptr;
    o.m_size = 0;
  }
  buffer &operator=(const buffer &o) {
    m_buffer = o.m_buffer;
    m_ptr = m_buffer.data();
    m_size = m_buffer.size();
    return *this;
  }
  buffer &operator=(buffer &&o) noexcept {
    m_buffer = std::move(o.m_buffer);
    m_ptr = m_buffer.data();
    m_size = m_buffer.size();
    o.m_ptr = nullptr;
    o.m_size = 0;
    return *this;
  }

  static buffer from(std::initializer_list<uint8_t> t) {
    buffer b(t.size());
    std::copy(t.begin(), t.end(), b.m_buffer.begin());
    return b;
  }

  template <typename Ptr, std::enable_if_t<std::is_pointer_v<Ptr>, bool> = true>
  static buffer from(Ptr ptr, size_t size) {
    buffer b(size);
    memcpy(b.m_buffer.data(), ptr, size);
    return b;
  }

  template <
      typename Container,
      std::enable_if_t<_shared_buffer_::is_iterable<Container>, bool> = true>
  static buffer from(const Container &c, size_t size = 0) {
    using T = typename std::remove_reference<
        decltype(std::declval<Container>().front())>::type;
    if (size == 0)
      size = sizeof(T) * c.size();
    buffer b(size);
    memcpy(b.m_buffer.data(), c.data(), size);
    return b;
  }

  template <typename Arr, size_t N>
  static buffer from(Arr (&a)[N], size_t size = 0) {
    if (size == 0) {
      if constexpr (std::is_same_v<Arr, const char>) {
        size = N - 1;
      } else {
        size = sizeof(Arr[N]);
      }
    }
    buffer b(size);
    memcpy(b.m_buffer.data(), a, size);
    return b;
  }

  template <typename Num,
            std::enable_if_t<std::is_arithmetic_v<Num>, bool> = true>
  static buffer from(const Num &n, size_t size = 0) {
    if (size == 0)
      size = sizeof(Num);
    buffer b(size);
    memcpy(b.m_buffer.data(), &n, size);
    return b;
  }

  template <typename Str,
            std::enable_if_t<std::is_same_v<const char *, Str>, bool> = true>
  static buffer from(const Str &str, size_t offset = 0, size_t size = 0) {
    if (size == 0)
      size = strlen(str);
    buffer b(size);
    memcpy(b.m_buffer.data(), str, size);
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

  std::string to_buffer_string() const override {
    std::ostringstream os;
    os << "ex::buffer (" << size() << ") { ";
    for (auto u : *this) {
      os << std::setfill('0') << std::setw(2) << std::hex << (int)u << ' ';
    }
    os << '}';
    return os.str();
  }

private:
  std::vector<uint8_t> m_buffer;
};

} // namespace ex