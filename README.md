# C++ Buffer

## Buffer Utils
```c++
namespace ex {

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
static inline T buffer_switch_endian(T t);

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
static inline void buffer_write_le(void *to, T v);

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
static inline void buffer_write_be(void *to, T v);

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
static inline T buffer_read_le(void *from);

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
static inline T buffer_read_be(void *from);

static inline void buffer_write_hex(void *to, std::string hex,
                                    bool skip_splitters_remove = false);

static inline std::string buffer_read_hex(void *from, size_t size,
                                          const std::string &splitter = "");

} // namespace ex
```

## Shared Buffer
```c++
namespace ex {
template <typename, typename = void> constexpr bool is_iterable{};

template <typename T>
constexpr bool is_iterable<T, std::void_t<decltype(std::declval<T>().begin()),
                                          decltype(std::declval<T>().end()),
                                          decltype(std::declval<T>().size()),
                                          decltype(std::declval<T>().data())>> =
    true;

class shared_buffer {
public:
  template <typename Ptr, std::enable_if_t<std::is_pointer_v<Ptr>, bool> = true>
  explicit shared_buffer(Ptr ptr, size_t size);

  template <typename Contaniner,
            std::enable_if_t<is_iterable<Contaniner>, bool> = true>
  explicit shared_buffer(Contaniner &c, size_t offset = 0, size_t size = 0);

  template <typename Arr, size_t N>
  explicit shared_buffer(const Arr (&a)[N], size_t offset = 0,
                         size_t size = 0);

  template <typename Num,
            std::enable_if_t<std::is_arithmetic_v<Num>, bool> = true>
  explicit shared_buffer(const Num &n, size_t offset = 0, size_t size = 0);

  template <typename T> void write_le(T v, size_t offset = 0);
  template <typename T> void write_be(T v, size_t offset = 0);
  template <typename T> T read_le(size_t offset = 0);
  template <typename T> T read_be(size_t offset = 0);

  void fill(std::initializer_list<uint8_t> t, size_t offset = 0);

  template <typename Ptr, std::enable_if_t<std::is_pointer_v<Ptr>, bool> = true>
  void fill(Ptr ptr, size_t offset, size_t size);

  template <typename Container,
            std::enable_if_t<is_iterable<Container>, bool> = true>
  void fill(const Container &t, size_t offset = 0, size_t size = 0)

  template <typename Str,
            std::enable_if_t<std::is_same_v<const char *, Str>, bool> = true>
  void fill(const Str &str, size_t offset = 0, size_t size = 0);

  template <typename Arr, size_t N>
  void fill(const Arr (&a)[N], size_t offset = 0, size_t size = 0);

  template <typename Num,
            std::enable_if_t<std::is_arithmetic_v<Num>, bool> = true>
  void fill(Num &&n, size_t offset = 0);

  void write_hex(std::string hex, size_t offset = 0,
                 bool skip_splitters_remove = false);
  std::string read_hex(size_t offset, size_t size = 0,
                       std::string splitter = "");

  uint8_t operator[](size_t i);

  struct iterator;
  using const_iterator = iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<iterator>;

  iterator begin() noexcept;
  const_iterator begin() const;
  iterator end() noexcept;
  const_iterator end() const noexcept;
  reverse_iterator<ex::shared_buffer::iterator> rbegin() noexcept;
  reverse_iterator<ex::shared_buffer::iterator> rend() noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;
  const_reverse_iterator crbegin() const;
  const_reverse_iterator crend() const;

  size_t size() const;
  uint8_t *data();

  std::string to_string() const;

  virtual std::string to_buffer_string() const;
};
} // namespace ex

inline std::ostream &operator<<(std::ostream &os,
                                const ex::shared_buffer &buffer);
```

## Buffer
```c++
namespace ex {
class buffer : public shared_buffer {
public:
  buffer(size_t size) : shared_buffer((void *)nullptr, 0), m_buffer(size) {
    m_ptr = m_buffer.data();
    m_size = size;
  }

  template <typename T> static buffer from(std::initializer_list<T> t) {
    buffer b(t.size());
    std::copy(t.begin(), t.end(), b.m_buffer.begin());
    return b;
  }
  template <typename T> static buffer from(T *p, size_t size) {
    buffer b(size);
    memcpy(b.m_buffer.data(), p, size);
    return b;
  }
  template <typename T> static buffer from(const T &t) {
    if constexpr (is_iterable<T>) {
      buffer b(t.size());
      std::copy(t.begin(), t.end(), b.m_buffer.begin());
      return b;
    } else {
      buffer b(sizeof(T));
      memcpy(b.m_buffer.data(), &t, sizeof(T));
      return b;
    }
  }
  template <typename T> static buffer from(const T &t, size_t size) {
    if constexpr (is_iterable<T>) {
      buffer b(size);
      memcpy(b.m_buffer.data(), t.data(), size);
      return b;
    } else {
      buffer b(size);
      memcpy(b.m_buffer.data(), &t, size);
      return b;
    }
  }

  template <typename T, size_t N> static buffer from(T (&t)[N]) {
    return from(t, N);
  }

  static buffer from(const char *str);

  static buffer from_hex(const std::string &str);
  
  std::string to_buffer_string() const override;
};

} // namespace ex
```