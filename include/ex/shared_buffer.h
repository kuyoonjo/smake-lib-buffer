#pragma once

#include "buffer_utils.h"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <type_traits>

namespace ex {
namespace _shared_buffer_ {
template <typename, typename = void> constexpr bool is_iterable{};

template <typename T>
constexpr bool is_iterable<T, std::void_t<decltype(std::declval<T>().begin()),
                                          decltype(std::declval<T>().end()),
                                          decltype(std::declval<T>().size()),
                                          decltype(std::declval<T>().data())>> =
    true;
} // namespace _shared_buffer_

class shared_buffer {
public:
  template <typename Ptr, std::enable_if_t<std::is_pointer_v<Ptr>, bool> = true>
  explicit shared_buffer(Ptr ptr, size_t size)
      : m_ptr((uint8_t *)ptr), m_size(size) {}

  template <
      typename Container,
      std::enable_if_t<_shared_buffer_::is_iterable<Container>, bool> = true>
  explicit shared_buffer(Container &c, size_t offset = 0, size_t size = 0) {
    using T = typename std::remove_reference<
        decltype(std::declval<Container>().front())>::type;
    if (size == 0)
      size = sizeof(T) * c.size();
    m_ptr = (uint8_t *)std::data(c) + offset;
    m_size = size;
  }

  template <typename Arr, size_t N>
  explicit shared_buffer(const Arr (&a)[N], size_t offset = 0,
                         size_t size = 0) {
    m_ptr = (uint8_t *)a;
    m_size = size ? size : sizeof(Arr[N]) - offset;
  }

  template <typename Num,
            std::enable_if_t<std::is_arithmetic_v<Num>, bool> = true>
  explicit shared_buffer(const Num &n, size_t offset = 0, size_t size = 0) {
    m_ptr = (uint8_t *)&n;
    m_size = size ? size : sizeof(Num) - offset;
  }

  template <typename T> void write_le(T v, size_t offset = 0) const {
    buffer_write_le(m_ptr + offset, v);
  }

  template <typename T> void write_be(T v, size_t offset = 0) const {
    buffer_write_be(m_ptr + offset, v);
  }

  template <typename T> T read_le(size_t offset = 0) const {
    return buffer_read_le<T>(m_ptr + offset);
  }

  template <typename T> T read_be(size_t offset = 0) const {
    return buffer_read_be<T>(m_ptr + offset);
  }

  void fill(std::initializer_list<uint8_t> t, size_t offset = 0) const {
    std::copy(t.begin(), t.end(), m_ptr + offset);
  }

  template <typename Ptr, std::enable_if_t<std::is_pointer_v<Ptr>, bool> = true>
  void fill(Ptr ptr, size_t offset, size_t size) {
    memcpy(m_ptr + offset, ptr, size);
  }

  template <
      typename Container,
      std::enable_if_t<_shared_buffer_::is_iterable<Container>, bool> = true>
  void fill(const Container &c, size_t offset = 0, size_t size = 0) const {
    using T = typename std::remove_reference<
        decltype(std::declval<Container>().front())>::type;
    if (size == 0)
      size = sizeof(T) * c.size();
    memcpy(m_ptr + offset, c.data(), size);
  }

  template <typename Str,
            std::enable_if_t<std::is_same_v<const char *, Str>, bool> = true>
  void fill(const Str &str, size_t offset = 0, size_t size = 0) const {
    if (size == 0)
      size = strlen(str);
    memcpy(m_ptr + offset, str, size);
  }

  template <typename Arr, size_t N>
  void fill(const Arr (&a)[N], size_t offset = 0, size_t size = 0) const {
    if (size == 0)
      size = sizeof(Arr[N]);
    memcpy(m_ptr + offset, a, size);
  }

  template <typename Num,
            std::enable_if_t<std::is_arithmetic_v<Num>, bool> = true>
  void fill(Num &&n, size_t offset = 0) const {
    memcpy(m_ptr + offset, &n, sizeof(Num));
  }

  void write_hex(std::string hex, size_t offset = 0,
                 bool skip_splitters_remove = false) const {
    buffer_write_hex(m_ptr + offset, hex, skip_splitters_remove);
  }

  std::string read_hex(size_t offset, size_t size = 0,
                       std::string splitter = "") const {
    return buffer_read_hex(m_ptr + offset, size, splitter);
  }

  uint8_t at(size_t i) const { return *(m_ptr + i); }
  uint8_t &operator[](size_t i) const { return *(m_ptr + i); }
  uint8_t front() const { return at(0); }
  uint8_t back() const { return at(m_size - 1); }

  struct iterator {
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = uint8_t;
    using pointer = uint8_t *;
    using reference = uint8_t &;

    iterator(pointer ptr) : m_ptr(ptr) {}

    reference operator*() const { return *m_ptr; }

    iterator operator+(difference_type n) const { return m_ptr + n; }
    iterator operator-(difference_type n) const { return m_ptr - n; }

    iterator &operator++() {
      m_ptr++;
      return *this;
    }
    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    iterator &operator--() {
      m_ptr--;
      return *this;
    }
    iterator operator--(int) {
      iterator tmp = *this;
      --(*this);
      return tmp;
    }

    iterator &operator+=(difference_type n) {
      m_ptr += n;
      return *this;
    }
    iterator &operator-=(difference_type n) {
      m_ptr -= n;
      return *this;
    }

    friend bool operator==(const iterator &a, const iterator &b) {
      return a.m_ptr == b.m_ptr;
    };
    friend bool operator!=(const iterator &a, const iterator &b) {
      return a.m_ptr != b.m_ptr;
    };

  protected:
    pointer m_ptr;
  };

  using const_iterator = iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<iterator>;

  iterator begin() noexcept { return iterator(m_ptr); }
  const_iterator begin() const noexcept { return iterator(m_ptr); }
  iterator end() noexcept { return iterator(m_ptr + m_size); }
  const_iterator end() const noexcept { return iterator(m_ptr + m_size); }
  auto rbegin() noexcept { return std::reverse_iterator(end()); }
  auto rend() noexcept { return std::reverse_iterator(begin()); }
  const_iterator cbegin() const noexcept { return iterator(m_ptr); }
  const_iterator cend() const noexcept { return iterator(m_ptr + m_size); }
  const_reverse_iterator crbegin() const {
    return std::reverse_iterator(end());
  }
  const_reverse_iterator crend() const {
    return std::reverse_iterator(begin());
  }

  auto size() const { return m_size; }
  auto *data() const { return m_ptr; }

  auto to_string() const { return std::string(begin(), end()); }
  auto to_hex_string(const std::string &splitter = "") const {
    return buffer_read_hex(data(), m_size, splitter);
  }

  virtual std::string to_buffer_string() const {
    std::ostringstream os;
    os << "ex::shared_buffer (" << size() << ") { ";
    for (auto u : *this) {
      os << std::setfill('0') << std::setw(2) << std::hex << (int)u << ' ';
    }
    os << '}';
    return os.str();
  }

protected:
  uint8_t *m_ptr;
  size_t m_size;
};
} // namespace ex

inline std::ostream &operator<<(std::ostream &os,
                                const ex::shared_buffer &buffer) {
  os << buffer.to_buffer_string();
  return os;
}
