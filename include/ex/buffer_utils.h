#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

namespace ex {

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
static inline T buffer_switch_endian(T t) {
  auto p = reinterpret_cast<uint8_t *>(&t);
  std::reverse(p, p + sizeof(T));
  return t;
}

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
static inline void buffer_write_le(void *to, T v) {
  *reinterpret_cast<T *>(to) = v;
}

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
static inline void buffer_write_be(void *to, T v) {
  *reinterpret_cast<T *>(to) = v;
  auto p = reinterpret_cast<uint8_t *>(to);
  std::reverse(p, p + sizeof(T));
}

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
static inline T buffer_read_le(void *from) {
  return *reinterpret_cast<T *>(from);
}

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
static inline T buffer_read_be(void *from) {
  return buffer_switch_endian(buffer_read_le<T>(from));
}

static inline void buffer_write_hex(void *to, std::string hex,
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
  auto p = (uint8_t *)to;
  for (size_t i = 0; i < len; ++i) {
    *(p + (len - i - 1 + odd)) = static_cast<uint8_t>(
        std::stoi(hex.substr(hlen - 2 - i * 2, 2), nullptr, 16));
  }
  if (odd)
    *p = static_cast<uint8_t>(std::stoi(hex.substr(0, 1), nullptr, 16));
}

static inline std::string buffer_read_hex(void *from, size_t size,
                                          const std::string &splitter = "") {
  if (!size)
    return "";
  auto splen = splitter.size();
  std::vector<char> str;
  auto fr = (uint8_t *)from;
  auto last = size - 1;
  for (size_t i = 0; i < size; ++i) {
    auto v = *(fr + i);
    auto l = v >> 4;
    auto r = v & 0xf;
    if (l < 10) {
      str.push_back('0' + l);
    } else {
      str.push_back('a' + l - 10);
    }
    if (r < 10) {
      str.push_back('0' + r);
    } else {
      str.push_back('a' + r - 10);
    }

    if (splen && i != last) {
      for (int j = 0; j < splen; ++j) {
        str.push_back(splitter[j]);
      }
    }
  }
  return std::string(str.begin(), str.end());
}

} // namespace ex