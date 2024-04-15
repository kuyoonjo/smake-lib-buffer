#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ex/buffer.h>
#include <ex/buffer_utils.h>
#include <ex/shared_buffer.h>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("buffer_utils") {
  auto b = ex::buffer(6);
  ex::buffer_write_be(b.data(), 3);
  CHECK(b[0] == 0);
  CHECK(b[1] == 0);
  CHECK(b[2] == 0);
  CHECK(b[3] == 3);
  ex::buffer_write_le(b.data(), 3);
  CHECK(b[0] == 3);
  CHECK(b[1] == 0);
  CHECK(b[2] == 0);
  CHECK(b[3] == 0);
  ex::buffer_write_hex(b.data(), "3cfad3b00001");
  CHECK(b[0] == 0x3c);
  CHECK(b[1] == 0xfa);
  CHECK(b[2] == 0xd3);
  CHECK(b[3] == 0xb0);
  CHECK(b[4] == 0x00);
  CHECK(b[5] == 0x01);
  auto mac = ex::buffer_read_hex(b.data(), 6);
  CHECK(mac == "3cfad3b00001");
  mac = ex::buffer_read_hex(b.data(), 6, ":");
  CHECK(mac == "3c:fa:d3:b0:00:01");
  auto empty = ex::buffer_read_hex(b.data(), 0);
  CHECK(empty == "");

  ex::buffer_write_hex(b.data(), "3c/fa/d3/b0/00/02");
  CHECK(b[5] == 0x02);
  ex::buffer_write_hex(b.data(), "3c:fa:d3:b0:00:03");
  CHECK(b[5] == 0x03);
  ex::buffer_write_hex(b.data(), "3c_fa_d3_b0_00_04");
  CHECK(b[5] == 0x04);
  ex::buffer_write_hex(b.data(), "3c|fa|d3|b0|00|05");
  CHECK(b[5] == 0x05);
  ex::buffer_write_hex(b.data(), "3CFAD3B00001");
  CHECK(b[0] == 0x3c);
  CHECK(b[1] == 0xfa);
  CHECK(b[2] == 0xd3);
  CHECK(b[3] == 0xb0);
  CHECK(b[4] == 0x00);
  CHECK(b[5] == 0x01);
}

TEST_CASE("buffer::from") {
  std::array<uint8_t, 4> arr = {1, 2, 3, 4};
  auto b = ex::buffer::from(arr, 2);
  CHECK(b.size() == 2);
  CHECK((b[0] == 1));
  CHECK((b[1] == 2));
  b = ex::buffer::from(arr);
  CHECK(b.size() == 4);
  CHECK((b[0] == 1));
  CHECK((b[1] == 2));
  CHECK((b[2] == 3));
  CHECK((b[3] == 4));

  std::vector<uint8_t> vec = {2, 3, 4, 5};
  b = ex::buffer::from(vec, 2);
  CHECK(b.size() == 2);
  CHECK((b[0] == 2));
  CHECK((b[1] == 3));
  b = ex::buffer::from(vec);
  CHECK(b.size() == 4);
  CHECK((b[0] == 2));
  CHECK((b[1] == 3));
  CHECK((b[2] == 4));
  CHECK((b[3] == 5));

  // from initializer_list
  b = ex::buffer::from({3, 4, 5, 6});
  CHECK(b.size() == 4);
  CHECK((b[0] == 3));
  CHECK((b[1] == 4));
  CHECK((b[2] == 5));
  CHECK((b[3] == 6));

  // from ptr
  b = ex::buffer::from(vec.data(), 3);
  CHECK(b.size() == 3);
  CHECK((b[0] == 2));
  CHECK((b[1] == 3));
  CHECK((b[2] == 4));

  // from raw array
  char ra[] = {4, 5};
  b = ex::buffer::from(ra);
  CHECK(b.size() == 2);
  CHECK((b[0] == 4));
  CHECK((b[1] == 5));
  const char rac[] = {4, 5, 0};
  b = ex::buffer::from(rac);
  CHECK(b.size() == 2);
  CHECK((b[0] == 4));
  CHECK((b[1] == 5));

  // from string
  std::string s = "abc";
  b = ex::buffer::from(s);
  CHECK(b.size() == 3);
  CHECK((b[0] == 0x61));
  CHECK((b[1] == 0x62));
  CHECK((b[2] == 0x63));
  b = ex::buffer::from(std::string("abcd"));
  CHECK(b.size() == 4);
  CHECK((b[0] == 0x61));
  CHECK((b[1] == 0x62));
  CHECK((b[2] == 0x63));
  ;
  CHECK((b[3] == 0x64));

  // from const char*
  b = ex::buffer::from("abc");
  std::cout << b << std::endl;
  CHECK(b.size() == 3);
  CHECK((b[0] == 0x61));
  CHECK((b[1] == 0x62));
  CHECK((b[2] == 0x63));
  auto cstr = "abcd";
  b = ex::buffer::from(cstr);
  CHECK(b.size() == 4);
  CHECK((b[0] == 0x61));
  CHECK((b[1] == 0x62));
  CHECK((b[2] == 0x63));
  ;
  CHECK((b[3] == 0x64));

  const char *ap = new char[4]{'a', 'b', 'c', 0};
  b = ex::buffer::from(ap);
  CHECK(b.size() == 3);
  CHECK((b[0] == 0x61));
  CHECK((b[1] == 0x62));
  CHECK((b[2] == 0x63));

  std::string ss = b.to_string();
  CHECK(ss == s);

  // from int
  b = ex::buffer::from(0x64636261);
  CHECK(b.size() == 4);
  CHECK((b[0] == 0x61));
  CHECK((b[1] == 0x62));
  CHECK((b[2] == 0x63));
  CHECK((b[3] == 0x64));
}

TEST_CASE("shared_buffer::write") {
  ex::buffer b(10);
  b.write_le<uint8_t>(0xff, 0);
  CHECK(b[0] == 0xff);
  for (auto it = b.cbegin() + 1; it != b.cend(); ++it) {
    CHECK(*it == 0);
  }

  b.write_le<uint16_t>(0xddee, 1);
  CHECK(b[0] == 0xff);
  CHECK(b[1] == 0xee);
  CHECK(b[2] == 0xdd);
  for (auto it = b.begin() + 3; it != b.end(); ++it) {
    CHECK(*it == 0);
  }

  b.write_be<uint32_t>(0xccbbaa99, 3);
  CHECK(b[0] == 0xff);
  CHECK(b[1] == 0xee);
  CHECK(b[2] == 0xdd);
  CHECK(b[3] == 0xcc);
  CHECK(b[4] == 0xbb);
  CHECK(b[5] == 0xaa);
  CHECK(b[6] == 0x99);
  for (auto it = b.begin() + 7; it != b.end(); ++it) {
    CHECK(*it == 0);
  }

  b.write_hex("8a8b8c", 7);
  CHECK(b[0] == 0xff);
  CHECK(b[1] == 0xee);
  CHECK(b[2] == 0xdd);
  CHECK(b[3] == 0xcc);
  CHECK(b[4] == 0xbb);
  CHECK(b[5] == 0xaa);
  CHECK(b[6] == 0x99);
  CHECK(b[7] == 0x8a);
  CHECK(b[8] == 0x8b);
  CHECK(b[9] == 0x8c);

  CHECK(ex::buffer::from_hex("abc").read_be<uint16_t>() == 0xabc);
}

TEST_CASE("shared_buffer::read") {
  ex::buffer b = ex::buffer::from({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
  CHECK(b.read_le<uint8_t>() == 0);
  CHECK(b.read_le<uint16_t>() == 0x0100);
  CHECK(b.read_be<uint32_t>() == 0x00010203);
  CHECK(b.read_hex(0, 4, ":") == "00:01:02:03");
}

TEST_CASE("shared_buffer::fill") {
  ex::buffer b(4);
  std::array<uint8_t, 4> tf = {1, 2, 3, 4};
  b.fill(tf);
  CHECK(b[0] == 1);
  CHECK(b[1] == 2);
  CHECK(b[2] == 3);
  CHECK(b[3] == 4);
  b.fill(tf, 2, 2);
  CHECK(b[0] == 1);
  CHECK(b[1] == 2);
  CHECK(b[2] == 1);
  CHECK(b[3] == 2);
  b.fill({2, 3, 4, 5});
  CHECK(b[0] == 2);
  CHECK(b[1] == 3);
  CHECK(b[2] == 4);
  CHECK(b[3] == 5);
  b.fill({7, 8}, 2);
  CHECK(b[0] == 2);
  CHECK(b[1] == 3);
  CHECK(b[2] == 7);
  CHECK(b[3] == 8);
  b.fill(tf.data(), 1, 3);
  CHECK(b[0] == 2);
  CHECK(b[1] == 1);
  CHECK(b[2] == 2);
  CHECK(b[3] == 3);
  char ra[4] = {'a', 'b', 'c', 'd'};
  b.fill(ra);
  CHECK(b[0] == 'a');
  CHECK(b[1] == 'b');
  CHECK(b[2] == 'c');
  CHECK(b[3] == 'd');
  b.fill("efgh");
  CHECK(b[0] == 'e');
  CHECK(b[1] == 'f');
  CHECK(b[2] == 'g');
  CHECK(b[3] == 'h');

  uint8_t arr[4] = {1, 2, 3, 4};
  b.fill(arr);
  std::cout << b << std::endl;
  CHECK(b[0] == 1);
  CHECK(b[1] == 2);
  CHECK(b[2] == 3);
  CHECK(b[3] == 4);
}

TEST_CASE("shared_buffer::shared_buffer") {
  uint8_t arr[8] = {0};
  auto p = &arr[4];
  ex::shared_buffer sb_from_ptr(p, 4);
  sb_from_ptr.fill({1, 2, 3, 4});
  std::string s(arr, arr + 8);
  ex::shared_buffer sb_from_container(s);
  ex::shared_buffer sb_from_arr(arr);
  int num = 3;
  ex::shared_buffer sb_from_num(num);

  ex::buffer b = ex::buffer::from({1, 2, 3, 4, 5});
  ex::shared_buffer sb(b);
  int a[3] = {0};
  ex::shared_buffer asb = ex::shared_buffer(a);
  int i = 1;
  for (auto c : sb) {
    CHECK(c == i);
    ++i;
  }
  CHECK(6 == i);
  sb = ex::shared_buffer(b, 1, 3);
  i = 2;
  for (auto c : sb) {
    CHECK(c == i);
    ++i;
  }
  CHECK(5 == i);

  CHECK(*sb.begin() == 2);
  // CHECK(*sb.rbegin() == 4);
  CHECK(sb[0] == 2);

  sb.fill({0, 0}, 1);
  CHECK(sb[0] == 2);
  CHECK(sb[1] == 0);
  CHECK(sb[2] == 0);

  char ra[2] = {1, 2};
  sb.fill(ra);
  CHECK(sb[0] == 1);
  CHECK(sb[1] == 2);
  CHECK(sb[2] == 0);

  sb.fill("abc");
  CHECK(sb[0] == 'a');
  CHECK(sb[1] == 'b');
  CHECK(sb[2] == 'c');

  CHECK(sb.to_string() == "abc");

  sb[0] = '1';
  CHECK(sb.to_string() == "1bc");
}

TEST_CASE("shared_buffer iterator") {
  uint8_t arr[8] = {0};
  ex::shared_buffer sb(arr);
  sb.write_be(0x01020304);
  CHECK(arr[0] == 0x01);
  CHECK(arr[1] == 0x02);
  CHECK(arr[2] == 0x03);
  CHECK(arr[3] == 0x04);
  auto it = sb.begin();
  CHECK(*it == 0x01);
  it++;
  CHECK(*it == 0x02);
  it += 2;
  CHECK(*it == 0x04);
  it--;
  CHECK(*it == 0x03);
  it -= 2;
  CHECK(*it == 0x01);
  auto rit = sb.rend();
  rit--;
  CHECK(*rit == 0x01);
  --rit;
  CHECK(*rit == 0x02);
  std::reverse(sb.begin(), sb.end());
  CHECK(arr[0] == 0);
  CHECK(arr[1] == 0);
  CHECK(arr[2] == 0);
  CHECK(arr[3] == 0);
  CHECK(arr[4] == 0x04);
  CHECK(arr[5] == 0x03);
  CHECK(arr[6] == 0x02);
  CHECK(arr[7] == 0x01);
  rit = sb.rbegin();
  CHECK(*rit == 0x01);
  rit = rit + 1;
  CHECK(*rit == 0x02);

  CHECK(sb.front() == 0);
  CHECK(sb.back() == 0x01);
  CHECK(*sb.crbegin() == 0x01);
  CHECK(*sb.crend() == 0);

  auto ptr = sb.data();
  *ptr = 1;
  CHECK(arr[0] == 1);
}

TEST_CASE("shared_buffer operator<<") {
  auto mac = ex::buffer::from_hex("3cfad3b00001");
  std::cout << mac << std::endl;
  auto shared = ex::shared_buffer(mac);
  std::cout << shared << std::endl;
  std::cout << 10 << std::endl;
}

TEST_CASE("buffer operator=") {
  auto mac = ex::buffer::from_hex("3cfad3b00001");
  auto lambda = [=] {
    CHECK(mac.to_hex_string() == "3cfad3b00001");
    mac.write_hex("3cfad3b00002");
    CHECK(mac.to_hex_string() == "3cfad3b00002");
  };
  lambda();
  CHECK(mac.to_hex_string() == "3cfad3b00001");
  auto x = mac;
  mac[0] = 1;
  CHECK(mac.to_hex_string() == "01fad3b00001");
  CHECK(x.to_hex_string() == "3cfad3b00001");
  auto y = std::move(mac);
  ex::buffer z(std::move(x));
  CHECK(mac.to_hex_string() == "");
  CHECK(x.to_hex_string() == "");
  CHECK(y.to_hex_string() == "01fad3b00001");
  CHECK(z.to_hex_string() == "3cfad3b00001");
  z = y;
  CHECK(y.to_hex_string() == "01fad3b00001");
  CHECK(z.to_hex_string() == "01fad3b00001");
}

// int main() {
//   testBufferUtils();
//   testBufferFrom();
//   testWrite();
//   testRead();
//   testFill();
//   testSharedBuffer();
//   testSharedBufferIterator();
//   testCout();
//   return 0;
// }