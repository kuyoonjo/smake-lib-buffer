#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <ex/buffer.h>
#include <ex/shared_buffer.h>
#include <iostream>
#include <string>
#include <vector>

void testBufferFrom() {
  std::array<int, 4> arr = {1, 2, 3, 4};
  auto b = ex::buffer::from(arr, 2);
  assert(b.size() == 2);
  assert((b[0] == 1));
  assert((b[1] == 2));
  b = ex::buffer::from(arr);
  assert(b.size() == 4);
  assert((b[0] == 1));
  assert((b[1] == 2));
  assert((b[2] == 3));
  assert((b[3] == 4));

  std::vector<int> vec = {2, 3, 4, 5};
  b = ex::buffer::from(vec, 2);
  assert(b.size() == 2);
  assert((b[0] == 2));
  assert((b[1] == 3));
  b = ex::buffer::from(vec);
  assert(b.size() == 4);
  assert((b[0] == 2));
  assert((b[1] == 3));
  assert((b[2] == 4));
  assert((b[3] == 5));

  // from initializer_list
  b = ex::buffer::from({3, 4, 5, 6});
  assert(b.size() == 4);
  assert((b[0] == 3));
  assert((b[1] == 4));
  assert((b[2] == 5));
  assert((b[3] == 6));

  // from ptr
  b = ex::buffer::from(vec.data(), 3);
  assert(b.size() == 3);
  assert((b[0] == 2));
  assert((b[1] == 3));
  assert((b[2] == 4));

  // from raw array
  int ra[4] = {4, 5};
  b = ex::buffer::from(ra, 2);
  assert(b.size() == 2);
  assert((b[0] == 4));
  assert((b[1] == 5));

  // from string
  std::string s = "abc";
  b = ex::buffer::from(s);
  assert(b.size() == 3);
  assert((b[0] == 0x61));
  assert((b[1] == 0x62));
  assert((b[2] == 0x63));
  std::cout << "buffer::from OK." << std::endl;
}

void testWrite() {
  ex::buffer b(10);
  b.write_le<uint8_t>(0xff, 0);
  assert(b[0] == 0xff);
  for (auto it = b.begin() + 1; it != b.end(); ++it) {
    assert(*it == 0);
  }

  b.write_le<uint16_t>(0xddee, 1);
  assert(b[0] == 0xff);
  assert(b[1] == 0xee);
  assert(b[2] == 0xdd);
  for (auto it = b.begin() + 3; it != b.end(); ++it) {
    assert(*it == 0);
  }

  b.write_be<uint32_t>(0xccbbaa99, 3);
  assert(b[0] == 0xff);
  assert(b[1] == 0xee);
  assert(b[2] == 0xdd);
  assert(b[3] == 0xcc);
  assert(b[4] == 0xbb);
  assert(b[5] == 0xaa);
  assert(b[6] == 0x99);
  for (auto it = b.begin() + 7; it != b.end(); ++it) {
    assert(*it == 0);
  }

  b.write_hex("8a8b8c", 7);
  assert(b[0] == 0xff);
  assert(b[1] == 0xee);
  assert(b[2] == 0xdd);
  assert(b[3] == 0xcc);
  assert(b[4] == 0xbb);
  assert(b[5] == 0xaa);
  assert(b[6] == 0x99);
  assert(b[7] == 0x8a);
  assert(b[8] == 0x8b);
  assert(b[9] == 0x8c);

  std::cout << "buffer write OK." << std::endl;
}

void testRead() {
  ex::buffer b{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  assert(b.read_le<uint8_t>() == 0);
  assert(b.read_le<uint16_t>() == 0x0100);
  assert(b.read_be<uint32_t>() == 0x00010203);
  assert(b.read_hex(0, 4, ":") == "00:01:02:03");

  std::cout << "buffer read OK." << std::endl;
}

void testFill() {
  ex::buffer b(4);
  std::array<int, 4> tf = {1, 2, 3, 4};
  b.fill(tf);
  assert(b[0] == 1);
  assert(b[1] == 2);
  assert(b[2] == 3);
  assert(b[3] == 4);
  b.fill(tf, 2, 2);
  assert(b[0] == 1);
  assert(b[1] == 2);
  assert(b[2] == 1);
  assert(b[3] == 2);
  b.fill({2, 3, 4, 5});
  assert(b[0] == 2);
  assert(b[1] == 3);
  assert(b[2] == 4);
  assert(b[3] == 5);
  b.fill({7, 8}, 2);
  assert(b[0] == 2);
  assert(b[1] == 3);
  assert(b[2] == 7);
  assert(b[3] == 8);
  b.fill(tf.data(), 1, 3);
  assert(b[0] == 2);
  assert(b[1] == 1);
  assert(b[2] == 2);
  assert(b[3] == 3);

  std::cout << "buffer fill OK." << std::endl;
}

void testSharedBuffer() {
  ex::buffer b{1, 2, 3, 4, 5};
  ex::shared_buffer sb(b);
  int i = 1;
  for (auto c : sb) {
    assert(c == i);
    ++i;
  }
  assert(6 == i);
  sb = ex::shared_buffer(b, 1, 3);
  i = 2;
  for (auto c : sb) {
    assert(c == i);
    ++i;
  }
  assert(5 == i);

  assert(*sb.begin() == 2);
  assert(*sb.rbegin() == 4);
  assert(sb[0] == 2);

  sb.fill({0, 0}, 1);
  assert(sb[0] == 2);
  assert(sb[1] == 0);
  assert(sb[2] == 0);
  std::cout << "shared buffer basic OK." << std::endl;
}

int main() {
  testBufferFrom();
  testWrite();
  testRead();
  testFill();
  testSharedBuffer();
  return 0;
}