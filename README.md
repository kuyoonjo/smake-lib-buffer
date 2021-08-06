# C++ Buffer 类
```c++

namespace ex {
class buffer : public std::vector<uint8_t> {
public:
  using vector_u8::vector_u8;
  template <typename T> static T switch_endian(T t);

  template <typename T> static buffer from(std::initializer_list<T> t);
  template <typename T> static buffer from(T *p, size_t size);
  template <typename T> static buffer from(T &t);
  template <typename T> static buffer from(T &t, size_t size);

  template <typename T> void write_le(T v, size_t offset = 0);
  template <typename T> void write_be(T v, size_t offset = 0);

  template <typename T> T read_le(size_t offset = 0);

  template <typename T> T read_be(size_t offset = 0);

  template <typename T>
  void fill(std::initializer_list<T> t, size_t offset = 0);
  template <typename T> void fill(T *p, size_t offset, size_t size);
  template <typename T> void fill(T &t);
  template <typename T> void fill(T &t, size_t offset);
  template <typename T> void fill(T &t, size_t offset, size_t size);

  void write_hex(std::string hex, size_t offset = 0,
                 bool skip_splitters_remove = false);

  std::string read_hex(size_t offset, size_t size = 0,
                       std::string splitter = "");
};
} // namespace ex

inline std::ostream &operator<<(std::ostream &os, ex::buffer &buffer);
```