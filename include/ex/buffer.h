#pragma once

#include <algorithm>
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
#include <vector>

namespace ex {
using vector_u8 = std::vector<uint8_t>;
class buffer : public std::vector<uint8_t> {
public:
  using vector_u8::vector_u8;
  template <typename T> static T switch_endian(T t) {
    auto p = reinterpret_cast<uint8_t *>(&t);
    std::reverse(p, p + sizeof(T));
    return t;
  }

  template <typename T> static buffer from(std::initializer_list<T> t) {
    return from(t.begin(), t.size());
  }
  template <typename T> static buffer from(T *p, size_t size) {
    buffer b(size);
    for (size_t i = 0; i < size; ++i)
      b[i] = *(p + i);
    return b;
  }
  template <typename T> static buffer from(T &t) { return from(t, t.size()); }
  template <typename T> static buffer from(T &t, size_t size) {
    buffer b(size);
    auto p = t.begin();
    for (size_t i = 0; i < size; ++i)
      b[i] = *(p + i);
    return b;
  }

  template <typename T, size_t N> static buffer from(T (&t)[N]) {
    return from(t, N);
  }

  static buffer from(const char *str) { return from(str, strlen(str)); }

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
    return switch_endian(read_le<T>(offset));
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

    auto data = hex.data();
    auto hlen = hex.size();
    auto len = hlen / 2;
    for (size_t i = 0; i < len; ++i) {
      (*this)[offset + i] =
          static_cast<uint8_t>(std::stoi(hex.substr(i * 2, 2), nullptr, 16));
    }
  }

  std::string read_hex(size_t offset, size_t size = 0,
                       std::string splitter = "") {
    if (!size)
      size = this->size() - offset;
    size_t splen = splitter.size();
    size_t slen = size * 2 + (size - 1) * splen;
    auto ss = splitter.begin();
    auto es = splitter.end();
    char *c = new char[slen];
    std::string fmt = "%02x";
    for (size_t i = 0; i < size; ++i) {
      auto p = c + i * (2 + splen);
      std::sprintf(p, fmt.c_str(), at(i));
      std::copy(ss, es, p + 2);
    }
    std::string s(c, slen);
    delete[] c;
    return s;
  }

  std::string to_string() { return std::string(begin(), end()); }
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