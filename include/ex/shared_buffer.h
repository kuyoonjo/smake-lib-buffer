#pragma once

#include "buffer.h"
#include <cstddef>
#include <iterator>

namespace ex {
class shared_buffer {
public:
  explicit shared_buffer(ex::buffer &buffer, size_t offset = 0, size_t size = 0)
      : m_buffer(buffer), m_offset(offset) {
    m_size = size ? size : m_buffer.size() - offset;
  }

  shared_buffer(const shared_buffer &sb)
      : m_buffer(sb.m_buffer), m_offset(sb.m_offset), m_size(sb.m_size) {}
  shared_buffer &operator=(const shared_buffer &sb) {
    m_buffer = sb.m_buffer;
    m_offset = sb.m_offset;
    m_size = sb.m_size;
    return *this;
  }

  template <typename T> void write_le(T v, size_t offset = 0) {
    m_buffer.write_le(v, offset + m_offset);
  }

  template <typename T> void write_be(T v, size_t offset = 0) {
    m_buffer.write_be(v, offset + m_offset);
  }

  template <typename T> T read_le(size_t offset = 0) {
    return m_buffer.read_le<T>(offset + m_offset);
  }

  template <typename T> T read_be(size_t offset = 0) {
    return m_buffer.read_be<T>(offset + m_offset);
  }

  template <typename T> void fill(T *p, size_t offset, size_t size) {
    m_buffer.fill(p, offset + m_offset, size);
  }

  template <typename T>
  void fill(std::initializer_list<T> t, size_t offset = 0) {
    m_buffer.fill(t, offset + m_offset);
  }
  template <typename T> void fill(T &t) { fill(t, 0, t.size()); }
  template <typename T> void fill(T &t, size_t offset) {
    m_buffer.fill(t, offset + m_offset);
  }
  template <typename T> void fill(T &t, size_t offset, size_t size) {
    auto p = t.begin();
    m_buffer.fill(t, offset + m_offset, size);
  }
  template <typename T, size_t N> void fill(T (&t)[N]) { fill(t, 0, N); }
  void fill(const char *str) { fill(str, 0, strlen(str)); }

  void write_hex(std::string hex, size_t offset = 0,
                 bool skip_splitters_remove = false) {
    m_buffer.write_hex(hex, offset + m_offset, skip_splitters_remove);
  }

  std::string read_hex(size_t offset, size_t size = 0,
                       std::string splitter = "") {
    return m_buffer.read_hex(offset + m_offset, size, splitter);
  }

  uint8_t operator[](size_t i) { return m_buffer[i + m_offset]; }

  ex::buffer::iterator begin() noexcept { return m_buffer.begin() + m_offset; }
  ex::buffer::const_iterator begin() const noexcept { return m_buffer.begin() + m_offset; }
  ex::buffer::iterator end() noexcept {
    return m_buffer.begin() + m_offset + m_size;
  }
  ex::buffer::const_iterator end() const noexcept {
    return m_buffer.begin() + m_offset + m_size;
  }
  ex::buffer::reverse_iterator rbegin() noexcept {
    return ex::buffer::reverse_iterator(end());
  }
  ex::buffer::reverse_iterator rend() noexcept {
    return ex::buffer::reverse_iterator(begin());
  }
  ex::buffer::const_iterator cbegin() const noexcept {
    return m_buffer.cbegin() + m_offset;
  }
  ex::buffer::const_iterator cend() const noexcept {
    return m_buffer.cbegin() + m_offset + m_size;
  }
  ex::buffer::const_reverse_iterator crbegin() const {
    return ex::buffer::const_reverse_iterator(cend());
  }
  ex::buffer::const_reverse_iterator crend() const {
    return ex::buffer::const_reverse_iterator(cbegin());
  }

  size_t size() { return m_size; }
  ex::buffer::value_type* data() { return m_buffer.data() + m_offset; }

  std::string to_string() {
    return std::string(begin(), end());
  }

private:
  ex::buffer &m_buffer;
  size_t m_offset;
  size_t m_size;
};
} // namespace ex

inline std::ostream &operator<<(std::ostream &os, const ex::shared_buffer &buffer) {
  os << "SharedBuffer { ";
  for (auto u : buffer) {
    os << std::setfill('0') << std::setw(2) << std::hex << (int)u << ' ';
  }
  os << '}';
  return os;
}
