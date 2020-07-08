/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include <CppUTest/SimpleString.h>
#include "packages/libawful/include/awful.hpp"
#include "utl/test-types.hh"
#include "utl/utl.hh"
#include "utl/names.hh"
#include <utl/logger.hh>
#include <utl/format.hh>
#include <utl/string.hh>
#include "test-support.hh"
#include <stdio.h>
#include <limits>

TEST_GROUP(Format) {};

using namespace utl::literals;

struct foo {
    int value;
};

constexpr void _format(foo const& arg, utl::fmt::output& out, utl::fmt::field const& f)
{
    utl::maybe_unused(f);
    utl::format_to(out, "{:#06x}", arg.value);
}

TEST(Format,AutomaticNumbering)
{
    constexpr utl::string_view bar = "hi there!";
    auto test = utl::format<60>("{}, {}, {:{}}", bar, 1.0f, 5, 7);
    CHECK_EQUAL("hi there!, {:f},     7"_sv, test);
}

TEST(Format,ManualNumbering)
{
    constexpr utl::string_view bar = "hi there!";
    auto test = utl::format<60>("{3}, {4}, {2}, {1}, {{, }}, {{{0}}}, {", 10u, -5, 1.0f, foo{42}, bar);
    CHECK_EQUAL("0x002a, hi there!, {:f}, -5, {, }, {10}, "_sv, test);
}

TEST(Format,Alignment)
{
    auto test = utl::format<60>("{:_^16,}", 1234567);
    CHECK_EQUAL("___1,234,567____"_sv, test);
}


// // Formatting library for C++ - formatting library tests
// //
// // Copyright (c) 2012 - present, Victor Zverovich
// // All rights reserved.
// //
// // For the license information refer to format.h.

// #include <stdint.h>

// #include <cctype>
// #include <cfloat>
// #include <climits>
// #include <cmath>
// #include <cstring>
// #include <list>
// #include <memory>
// #include <string>

// // Check if fmt/format.h compiles with windows.h included before it.
// #ifdef _WIN32
// #  include <windows.h>
// #endif

// // Check if fmt/format.h compiles with the X11 index macro defined.
// #define index(x, y) no nice things

// #include "fmt/color.h"
// #include "fmt/format.h"

// #undef index

// #include "gmock.h"
// #include "gtest-extra.h"
// #include "mock-allocator.h"
// #include "util.h"

// #undef ERROR

// using fmt::basic_memory_buffer;
// using fmt::format;
// using fmt::format_error;
// using fmt::memory_buffer;
// using fmt::string_view;
// using fmt::wmemory_buffer;
// using fmt::wstring_view;
// using fmt::detail::max_value;

// using testing::Return;
// using testing::StrictMock;

// namespace {

// #if !FMT_GCC_VERSION || FMT_GCC_VERSION >= 408
// template <typename Char, typename T> bool check_enabled_formatter() {
//   static_assert(std::is_default_constructible<fmt::formatter<T, Char>>::value,
//                 "");
//   return true;
// }

// template <typename Char, typename... T> void check_enabled_formatters() {
//   auto dummy = {check_enabled_formatter<Char, T>()...};
//   (void)dummy;
// }

// TEST(Format, TestFormattersEnabled) {
//   check_enabled_formatters<char, bool, char, signed char, unsigned char, short,
//                            unsigned short, int, unsigned, long, unsigned long,
//                            long long, unsigned long long, float, double,
//                            long double, void*, const void*, char*, const char*,
//                            std::string, std::nullptr_t>();
//   check_enabled_formatters<wchar_t, bool, wchar_t, signed char, unsigned char,
//                            short, unsigned short, int, unsigned, long,
//                            unsigned long, long long, unsigned long long, float,
//                            double, long double, void*, const void*, wchar_t*,
//                            const wchar_t*, std::wstring, std::nullptr_t>();
// }
// #endif

// // Format value using the standard library.
// template <typename Char, typename T>
// void std_format(const T& value, std::basic_string<Char>& result) {
//   std::basic_ostringstream<Char> os;
//   os << value;
//   result = os.str();
// }

// #ifdef __MINGW32__
// // Workaround a bug in formatting long double in MinGW.
// void std_format(long double value, std::string& result) {
//   char buffer[100];
//   snprintf(buffer, BUFFER_SIZE, "%Lg", value);
//   result = buffer;
// }
// void std_format(long double value, std::wstring& result) {
//   wchar_t buffer[100];
//   swprintf(buffer, L"%Lg", value);
//   result = buffer;
// }
// #endif
// }  // namespace

// struct uint32_pair {
//   uint32_t u[2];
// };

// TEST(UtilTest, BitCast) {
//   auto s = fmt::detail::bit_cast<uint32_pair>(uint64_t{42});
//   CHECK_EQUAL(fmt::detail::bit_cast<uint64_t>(s), 42ull);
//   s = fmt::detail::bit_cast<uint32_pair>(uint64_t(~0ull));
//   CHECK_EQUAL(fmt::detail::bit_cast<uint64_t>(s), ~0ull);
// }

// TEST(UtilTest, Increment) {
//   char s[10] = "123";
//   increment(s);
//   EXPECT_STREQ("124", s);
//   s[2] = '8';
//   increment(s);
//   EXPECT_STREQ("129", s);
//   increment(s);
//   EXPECT_STREQ("130", s);
//   s[1] = s[2] = '9';
//   increment(s);
//   EXPECT_STREQ("200", s);
// }

// TEST(UtilTest, ParseNonnegativeInt) {
//   if (max_value<int>() != static_cast<int>(static_cast<unsigned>(1) << 31)) {
//     fmt::print("Skipping parse_nonnegative_int test\n");
//     return;
//   }
//   fmt::string_view s = "10000000000";
//   auto begin = s.begin(), end = s.end();
//   EXPECT_THROW_MSG(
//       parse_nonnegative_int(begin, end, fmt::detail::error_handler()),
//       fmt::format_error, "number is too big");
//   s = "2147483649";
//   begin = s.begin();
//   end = s.end();
//   EXPECT_THROW_MSG(
//       parse_nonnegative_int(begin, end, fmt::detail::error_handler()),
//       fmt::format_error, "number is too big");
// }

// TEST(IteratorTest, CountingIterator) {
//   fmt::detail::counting_iterator it;
//   auto prev = it++;
//   CHECK_EQUAL(prev.count(), 0);
//   CHECK_EQUAL(it.count(), 1);
// }

// TEST(IteratorTest, TruncatingIterator) {
//   char* p = nullptr;
//   fmt::detail::truncating_iterator<char*> it(p, 3);
//   auto prev = it++;
//   CHECK_EQUAL(prev.base(), p);
//   CHECK_EQUAL(it.base(), p + 1);
// }

// TEST(IteratorTest, TruncatingBackInserter) {
//   std::string buffer;
//   auto bi = std::back_inserter(buffer);
//   fmt::detail::truncating_iterator<decltype(bi)> it(bi, 2);
//   *it++ = '4';
//   *it++ = '2';
//   *it++ = '1';
//   CHECK_EQUAL(buffer.size(), 2);
//   CHECK_EQUAL(buffer, "42");
// }

// TEST(IteratorTest, IsOutputIterator) {
//   EXPECT_TRUE(fmt::detail::is_output_iterator<char*>::value);
//   EXPECT_FALSE(fmt::detail::is_output_iterator<const char*>::value);
//   EXPECT_FALSE(fmt::detail::is_output_iterator<std::string>::value);
//   EXPECT_TRUE(fmt::detail::is_output_iterator<
//               std::back_insert_iterator<std::string>>::value);
//   EXPECT_TRUE(fmt::detail::is_output_iterator<std::string::iterator>::value);
//   EXPECT_FALSE(
//       fmt::detail::is_output_iterator<std::string::const_iterator>::value);
//   EXPECT_FALSE(fmt::detail::is_output_iterator<std::list<char>>::value);
//   EXPECT_TRUE(
//       fmt::detail::is_output_iterator<std::list<char>::iterator>::value);
//   EXPECT_FALSE(
//       fmt::detail::is_output_iterator<std::list<char>::const_iterator>::value);
//   EXPECT_FALSE(fmt::detail::is_output_iterator<uint32_pair>::value);
// }

// TEST(MemoryBufferTest, Ctor) {
//   basic_memory_buffer<char, 123> buffer;
//   CHECK_EQUAL(static_cast<size_t>(0), buffer.size());
//   CHECK_EQUAL(123u, buffer.capacity());
// }

// static void check_forwarding(mock_allocator<int>& alloc,
//                              allocator_ref<mock_allocator<int>>& ref) {
//   int mem;
//   // Check if value_type is properly defined.
//   allocator_ref<mock_allocator<int>>::value_type* ptr = &mem;
//   // Check forwarding.
//   EXPECT_CALL(alloc, allocate(42)).WillOnce(testing::Return(ptr));
//   ref.allocate(42);
//   EXPECT_CALL(alloc, deallocate(ptr, 42));
//   ref.deallocate(ptr, 42);
// }

// TEST(AllocatorTest, allocator_ref) {
//   StrictMock<mock_allocator<int>> alloc;
//   typedef allocator_ref<mock_allocator<int>> test_allocator_ref;
//   test_allocator_ref ref(&alloc);
//   // Check if allocator_ref forwards to the underlying allocator.
//   check_forwarding(alloc, ref);
//   test_allocator_ref ref2(ref);
//   check_forwarding(alloc, ref2);
//   test_allocator_ref ref3;
//   CHECK_EQUAL(nullptr, ref3.get());
//   ref3 = ref;
//   check_forwarding(alloc, ref3);
// }

// typedef allocator_ref<std::allocator<char>> TestAllocator;

// static void check_move_buffer(
//     const char* str, basic_memory_buffer<char, 5, TestAllocator>& buffer) {
//   std::allocator<char>* alloc = buffer.get_allocator().get();
//   basic_memory_buffer<char, 5, TestAllocator> buffer2(std::move(buffer));
//   // Move shouldn't destroy the inline content of the first buffer.
//   CHECK_EQUAL(str, std::string(&buffer[0], buffer.size()));
//   CHECK_EQUAL(str, std::string(&buffer2[0], buffer2.size()));
//   CHECK_EQUAL(5u, buffer2.capacity());
//   // Move should transfer allocator.
//   CHECK_EQUAL(nullptr, buffer.get_allocator().get());
//   CHECK_EQUAL(alloc, buffer2.get_allocator().get());
// }

// TEST(MemoryBufferTest, MoveCtorInlineBuffer) {
//   std::allocator<char> alloc;
//   basic_memory_buffer<char, 5, TestAllocator> buffer((TestAllocator(&alloc)));
//   const char test[] = "test";
//   buffer.append(test, test + 4);
//   check_move_buffer("test", buffer);
//   // Adding one more character fills the inline buffer, but doesn't cause
//   // dynamic allocation.
//   buffer.push_back('a');
//   check_move_buffer("testa", buffer);
// }

// TEST(MemoryBufferTest, MoveCtorDynamicBuffer) {
//   std::allocator<char> alloc;
//   basic_memory_buffer<char, 4, TestAllocator> buffer((TestAllocator(&alloc)));
//   const char test[] = "test";
//   buffer.append(test, test + 4);
//   const char* inline_buffer_ptr = &buffer[0];
//   // Adding one more character causes the content to move from the inline to
//   // a dynamically allocated buffer.
//   buffer.push_back('a');
//   basic_memory_buffer<char, 4, TestAllocator> buffer2(std::move(buffer));
//   // Move should rip the guts of the first buffer.
//   CHECK_EQUAL(inline_buffer_ptr, &buffer[0]);
//   CHECK_EQUAL("testa", std::string(&buffer2[0], buffer2.size()));
//   EXPECT_GT(buffer2.capacity(), 4u);
// }

// static void check_move_assign_buffer(const char* str,
//                                      basic_memory_buffer<char, 5>& buffer) {
//   basic_memory_buffer<char, 5> buffer2;
//   buffer2 = std::move(buffer);
//   // Move shouldn't destroy the inline content of the first buffer.
//   CHECK_EQUAL(str, std::string(&buffer[0], buffer.size()));
//   CHECK_EQUAL(str, std::string(&buffer2[0], buffer2.size()));
//   CHECK_EQUAL(5u, buffer2.capacity());
// }

// TEST(MemoryBufferTest, MoveAssignment) {
//   basic_memory_buffer<char, 5> buffer;
//   const char test[] = "test";
//   buffer.append(test, test + 4);
//   check_move_assign_buffer("test", buffer);
//   // Adding one more character fills the inline buffer, but doesn't cause
//   // dynamic allocation.
//   buffer.push_back('a');
//   check_move_assign_buffer("testa", buffer);
//   const char* inline_buffer_ptr = &buffer[0];
//   // Adding one more character causes the content to move from the inline to
//   // a dynamically allocated buffer.
//   buffer.push_back('b');
//   basic_memory_buffer<char, 5> buffer2;
//   buffer2 = std::move(buffer);
//   // Move should rip the guts of the first buffer.
//   CHECK_EQUAL(inline_buffer_ptr, &buffer[0]);
//   CHECK_EQUAL("testab", std::string(&buffer2[0], buffer2.size()));
//   EXPECT_GT(buffer2.capacity(), 5u);
// }

// TEST(MemoryBufferTest, Grow) {
//   typedef allocator_ref<mock_allocator<int>> Allocator;
//   typedef basic_memory_buffer<int, 10, Allocator> Base;
//   mock_allocator<int> alloc;
//   struct TestMemoryBuffer : Base {
//     TestMemoryBuffer(Allocator alloc) : Base(alloc) {}
//     void grow(size_t size) { Base::grow(size); }
//   } buffer((Allocator(&alloc)));
//   buffer.resize(7);
//   using fmt::detail::to_unsigned;
//   for (int i = 0; i < 7; ++i) buffer[to_unsigned(i)] = i * i;
//   CHECK_EQUAL(10u, buffer.capacity());
//   int mem[20];
//   mem[7] = 0xdead;
//   EXPECT_CALL(alloc, allocate(20)).WillOnce(Return(mem));
//   buffer.grow(20);
//   CHECK_EQUAL(20u, buffer.capacity());
//   // Check if size elements have been copied
//   for (int i = 0; i < 7; ++i) CHECK_EQUAL(i * i, buffer[to_unsigned(i)]);
//   // and no more than that.
//   CHECK_EQUAL(0xdead, buffer[7]);
//   EXPECT_CALL(alloc, deallocate(mem, 20));
// }

// TEST(MemoryBufferTest, Allocator) {
//   typedef allocator_ref<mock_allocator<char>> TestAllocator;
//   basic_memory_buffer<char, 10, TestAllocator> buffer;
//   CHECK_EQUAL(nullptr, buffer.get_allocator().get());
//   StrictMock<mock_allocator<char>> alloc;
//   char mem;
//   {
//     basic_memory_buffer<char, 10, TestAllocator> buffer2(
//         (TestAllocator(&alloc)));
//     CHECK_EQUAL(&alloc, buffer2.get_allocator().get());
//     size_t size = 2 * fmt::inline_buffer_size;
//     EXPECT_CALL(alloc, allocate(size)).WillOnce(Return(&mem));
//     buffer2.reserve(size);
//     EXPECT_CALL(alloc, deallocate(&mem, size));
//   }
// }

// TEST(MemoryBufferTest, ExceptionInDeallocate) {
//   typedef allocator_ref<mock_allocator<char>> TestAllocator;
//   StrictMock<mock_allocator<char>> alloc;
//   basic_memory_buffer<char, 10, TestAllocator> buffer((TestAllocator(&alloc)));
//   size_t size = 2 * fmt::inline_buffer_size;
//   std::vector<char> mem(size);
//   {
//     EXPECT_CALL(alloc, allocate(size)).WillOnce(Return(&mem[0]));
//     buffer.resize(size);
//     std::fill(&buffer[0], &buffer[0] + size, 'x');
//   }
//   std::vector<char> mem2(2 * size);
//   {
//     EXPECT_CALL(alloc, allocate(2 * size)).WillOnce(Return(&mem2[0]));
//     std::exception e;
//     EXPECT_CALL(alloc, deallocate(&mem[0], size)).WillOnce(testing::Throw(e));
//     EXPECT_THROW(buffer.reserve(2 * size), std::exception);
//     CHECK_EQUAL(&mem2[0], &buffer[0]);
//     // Check that the data has been copied.
//     for (size_t i = 0; i < size; ++i) CHECK_EQUAL('x', buffer[i]);
//   }
//   EXPECT_CALL(alloc, deallocate(&mem2[0], 2 * size));
// }

// TEST(UtilTest, UTF8ToUTF16) {
//   fmt::detail::utf8_to_utf16 u("лошадка");
//   CHECK_EQUAL(L"\x043B\x043E\x0448\x0430\x0434\x043A\x0430", u.str());
//   CHECK_EQUAL(7, u.size());
//   // U+10437 { DESERET SMALL LETTER YEE }
//   CHECK_EQUAL(L"\xD801\xDC37", fmt::detail::utf8_to_utf16("𐐷").str());
//   EXPECT_THROW_MSG(fmt::detail::utf8_to_utf16("\xc3\x28"), std::runtime_error,
//                    "invalid utf8");
//   EXPECT_THROW_MSG(fmt::detail::utf8_to_utf16(fmt::string_view("л", 1)),
//                    std::runtime_error, "invalid utf8");
//   CHECK_EQUAL(L"123456", fmt::detail::utf8_to_utf16("123456").str());
// }

// TEST(UtilTest, UTF8ToUTF16EmptyString) {
//   std::string s = "";
//   fmt::detail::utf8_to_utf16 u(s.c_str());
//   CHECK_EQUAL(L"", u.str());
//   CHECK_EQUAL(s.size(), u.size());
// }

// TEST(UtilTest, FormatSystemError) {
//   fmt::memory_buffer message;
//   fmt::format_system_error(message, EDOM, "test");
//   CHECK_EQUAL(fmt::format("test: {}", get_system_error(EDOM)),
//             to_string(message));
//   message = fmt::memory_buffer();

//   // Check if std::allocator throws on allocating max size_t / 2 chars.
//   size_t max_size = max_value<size_t>() / 2;
//   bool throws_on_alloc = false;
//   try {
//     std::allocator<char> alloc;
//     alloc.deallocate(alloc.allocate(max_size), max_size);
//   } catch (const std::bad_alloc&) {
//     throws_on_alloc = true;
//   }
//   if (!throws_on_alloc) {
//     fmt::print("warning: std::allocator allocates {} chars", max_size);
//     return;
//   }
//   fmt::format_system_error(message, EDOM, fmt::string_view(nullptr, max_size));
//   CHECK_EQUAL(fmt::format("error {}", EDOM), to_string(message));
// }

// TEST(UtilTest, SystemError) {
//   fmt::system_error e(EDOM, "test");
//   CHECK_EQUAL(fmt::format("test: {}", get_system_error(EDOM)), e.what());
//   CHECK_EQUAL(EDOM, e.error_code());

//   fmt::system_error error(0, "");
//   try {
//     throw fmt::system_error(EDOM, "test {}", "error");
//   } catch (const fmt::system_error& e) {
//     error = e;
//   }
//   fmt::memory_buffer message;
//   fmt::format_system_error(message, EDOM, "test error");
//   CHECK_EQUAL(to_string(message), error.what());
//   CHECK_EQUAL(EDOM, error.error_code());
// }

// TEST(UtilTest, ReportSystemError) {
//   fmt::memory_buffer out;
//   fmt::format_system_error(out, EDOM, "test error");
//   out.push_back('\n');
//   EXPECT_WRITE(stderr, fmt::report_system_error(EDOM, "test error"),
//                to_string(out));
// }

// TEST(StringViewTest, Ctor) {
//   EXPECT_STREQ("abc", string_view("abc").data());
//   CHECK_EQUAL(3u, string_view("abc").size());

//   EXPECT_STREQ("defg", string_view(std::string("defg")).data());
//   CHECK_EQUAL(4u, string_view(std::string("defg")).size());
// }

TEST(Format, FormatWithoutArgs) {
    utl::string<10> s;
    utl::format_into(s,"test");
    CHECK_EQUAL("test"_sv, s);
}

TEST(Format, Format) {    
    utl::string<20> s;
    utl::format_into(s, "part{0}", 1);
    CHECK_EQUAL("part1"_sv, s);
    utl::format_into(s, "part{0}", 2);
    CHECK_EQUAL("part2"_sv, s);
}

// TEST(Format, WideString) {
//   std::vector<wchar_t> buf;
//   utl::format_to(std::back_inserter(buf), L"{}{}", 42, L'\0');
//   EXPECT_STREQ(buf.data(), L"42");
// }

TEST(Format, FormatToMemoryBuffer) {
    utl::array<char, 100> buffer;
    utl::format_into(buffer, "{}", "foo");
    CHECK_EQUAL("foo"_sv, utl::string_view{buffer.data()});
//   utl::wmemory_buffer wbuffer;
//   utl::format_into(wbuffer, L"{}", L"foo");
//   CHECK_EQUAL(L"foo", to_string(wbuffer));
}

TEST(Format, Escape) {
    CHECK_EQUAL("{"_sv, utl::format<60>("{{"));
    CHECK_EQUAL("before {"_sv, utl::format<60>("before {{"));
    CHECK_EQUAL("{ after"_sv, utl::format<60>("{{ after"));
    CHECK_EQUAL("before { after"_sv, utl::format<60>("before {{ after"));

    CHECK_EQUAL("}"_sv, utl::format<60>("}}"));
    CHECK_EQUAL("before }"_sv, utl::format<60>("before }}"));
    CHECK_EQUAL("} after"_sv, utl::format<60>("}} after"));
    CHECK_EQUAL("before } after"_sv, utl::format<60>("before }} after"));

    CHECK_EQUAL("{}"_sv, utl::format<60>("{{}}"));
    CHECK_EQUAL("{42}"_sv, utl::format<60>("{{{0}}}", 42));
}

TEST(Format, UnmatchedBraces) {
    CHECK_EQUAL(""_sv, utl::format<10>("{"));
    CHECK_EQUAL(""_sv, utl::format<10>("}"));
    CHECK_EQUAL(""_sv, utl::format<10>("{0{}"));
}

TEST(Format, NoArgs) { CHECK_EQUAL("test"_sv, utl::format<10>("test")); }

TEST(Format, ArgsInDifferentPositions) {
    CHECK_EQUAL("42"_sv, utl::format<20>("{0}", 42));
    CHECK_EQUAL("before 42"_sv, utl::format<20>("before {0}", 42));
    CHECK_EQUAL("42 after"_sv, utl::format<20>("{0} after", 42));
    CHECK_EQUAL("before 42 after"_sv, utl::format<20>("before {0} after", 42));
    CHECK_EQUAL("answer = 42"_sv, utl::format<20>("{0} = {1}", "answer", 42));
    CHECK_EQUAL("42 is the answer"_sv, utl::format<20>("{1} is the {0}", "answer", 42));
    CHECK_EQUAL("abracadabra"_sv, utl::format<20>("{0}{1}{0}", "abra", "cad"));
}

// TEST(Format, ArgErrors) {
//   EXPECT_THROW_MSG(format("{"), format_error, "invalid format string");
//   EXPECT_THROW_MSG(format("{?}"), format_error, "invalid format string");
//   EXPECT_THROW_MSG(format("{0"), format_error, "invalid format string");
//   EXPECT_THROW_MSG(format("{0}"), format_error, "argument not found");
//   EXPECT_THROW_MSG(format("{00}", 42), format_error, "invalid format string");

//   char format_str[BUFFER_SIZE];
//   safe_sprintf(format_str, "{%u", INT_MAX);
//   EXPECT_THROW_MSG(format(format_str), format_error, "invalid format string");
//   safe_sprintf(format_str, "{%u}", INT_MAX);
//   EXPECT_THROW_MSG(format(format_str), format_error, "argument not found");

//   safe_sprintf(format_str, "{%u", INT_MAX + 1u);
//   EXPECT_THROW_MSG(format(format_str), format_error, "number is too big");
//   safe_sprintf(format_str, "{%u}", INT_MAX + 1u);
//   EXPECT_THROW_MSG(format(format_str), format_error, "number is too big");
// }

template <size_t S, int N> struct TestFormat {
  template <typename... Args>
  static utl::string<S> format(utl::string_view format_str, const Args&... args) {
    return TestFormat<S,N - 1>::format(format_str, N - 1, args...);
  }
};

template <size_t S> struct TestFormat<S,0> {
  template <typename... Args>
  static utl::string<S> format(utl::string_view format_str, const Args&... args) {
    return utl::format<S>(format_str, args...);
  }
};

TEST(Format, ManyArgs) {
    auto res = TestFormat<100,20>::format("{19}");
    CHECK_EQUAL("19"_sv, res);
//   EXPECT_THROW_MSG(TestFormat<20>::format("{20}"), format_error,
//                    "argument not found");
//   EXPECT_THROW_MSG(TestFormat<21>::format("{21}"), format_error,
//                    "argument not found");
//   enum { max_packed_args = utl::detail::max_packed_args };
//   std::string format_str = utl::format("{{{}}}", max_packed_args + 1);
//   EXPECT_THROW_MSG(TestFormat<max_packed_args>::format(format_str),
//                    format_error, "argument not found");
}

// TEST(Format, NamedArg) {
//   CHECK_EQUAL("1/a/A"_sv, format("{_1}/{a_}/{A_}", utl::arg("a_", 'a'),
//                             utl::arg("A_", "A"), utl::arg("_1", 1)));
//   EXPECT_THROW_MSG(format("{a}"), format_error, "argument not found");
//   CHECK_EQUAL(" -42"_sv, format("{0:{width}}", -42, utl::arg("width", 4)));
//   CHECK_EQUAL("st"_sv, format("{0:.{precision}}", "str", utl::arg("precision", 2)));
//   CHECK_EQUAL("1 2"_sv, format("{} {two}", 1, utl::arg("two", 2)));
//   CHECK_EQUAL("42"_sv, format("{c}", utl::arg("a", 0), utl::arg("b", 0),
//                          utl::arg("c", 42), utl::arg("d", 0), utl::arg("e", 0),
//                          utl::arg("f", 0), utl::arg("g", 0), utl::arg("h", 0),
//                          utl::arg("i", 0), utl::arg("j", 0), utl::arg("k", 0),
//                          utl::arg("l", 0), utl::arg("m", 0), utl::arg("n", 0),
//                          utl::arg("o", 0), utl::arg("p", 0)));
// }

TEST(Format, AutoArgIndex) {
  CHECK_EQUAL("abc"_sv, utl::format<60>("{}{}{}", 'a', 'b', 'c'));
//   EXPECT_THROW_MSG(utl::format<60>("{0}{}", 'a', 'b'), format_error,
//                    "cannot switch from manual to automatic argument indexing");
//   EXPECT_THROW_MSG(utl::format<60>("{}{0}", 'a', 'b'), format_error,
//                    "cannot switch from automatic to manual argument indexing");
//   CHECK_EQUAL("1.2"_sv, utl::format<60>("{:.{}}", 1.2345, 2));
//   EXPECT_THROW_MSG(utl::format<60>("{0}:.{}", 1.2345, 2), format_error,
//                    "cannot switch from manual to automatic argument indexing");
//   EXPECT_THROW_MSG(utl::format<60>("{:.{0}}", 1.2345, 2), format_error,
//                    "cannot switch from automatic to manual argument indexing");
//   EXPECT_THROW_MSG(utl::format<60>("{}"), format_error, "argument not found");
}

TEST(Format, EmptySpecs) { CHECK_EQUAL("42"_sv, utl::format<60>("{0:}", 42)); }

TEST(Format, LeftAlign) {
  CHECK_EQUAL("42  "_sv, utl::format<60>("{0:<4}", 42));
  CHECK_EQUAL("42  "_sv, utl::format<60>("{0:<4o}", 042));
  CHECK_EQUAL("42  "_sv, utl::format<60>("{0:<4x}", 0x42));
  CHECK_EQUAL("-42  "_sv, utl::format<60>("{0:<5}", -42));
  CHECK_EQUAL("42   "_sv, utl::format<60>("{0:<5}", 42u));
  CHECK_EQUAL("-42  "_sv, utl::format<60>("{0:<5}", -42l));
  CHECK_EQUAL("42   "_sv, utl::format<60>("{0:<5}", 42ul));
  CHECK_EQUAL("-42  "_sv, utl::format<60>("{0:<5}", -42ll));
//   CHECK_EQUAL("42   "_sv, utl::format<60>("{0:<5}", 42ull));
//   CHECK_EQUAL("-42.0  "_sv, utl::format<60>("{0:<7}", -42.0));
//   CHECK_EQUAL("-42.0  "_sv, utl::format<60>("{0:<7}", -42.0l));
  CHECK_EQUAL("c    "_sv, utl::format<60>("{0:<5}", 'c'));
  CHECK_EQUAL("abc  "_sv, utl::format<60>("{0:<5}", "abc"));
  CHECK_EQUAL("0xface  "_sv, utl::format<60>("{0:<8}", reinterpret_cast<void*>(0xface)));
}

TEST(Format, RightAlign) {
  CHECK_EQUAL("  42"_sv, utl::format<10>("{0:>4}", 42));
  CHECK_EQUAL("  42"_sv, utl::format<10>("{0:>4o}", 042));
  CHECK_EQUAL("  42"_sv, utl::format<10>("{0:>4x}", 0x42));
  CHECK_EQUAL("  -42"_sv, utl::format<10>("{0:>5}", -42));
  CHECK_EQUAL("   42"_sv, utl::format<10>("{0:>5}", 42u));
  CHECK_EQUAL("  -42"_sv, utl::format<10>("{0:>5}", -42l));
  CHECK_EQUAL("   42"_sv, utl::format<10>("{0:>5}", 42ul));
  CHECK_EQUAL("  -42"_sv, utl::format<10>("{0:>5}", -42ll));
//   CHECK_EQUAL("   42"_sv, utl::format<10>("{0:>5}", 42ull));
//   CHECK_EQUAL("  -42.0"_sv, utl::format<10>("{0:>7}", -42.0));
//   CHECK_EQUAL("  -42.0"_sv, utl::format<10>("{0:>7}", -42.0l));
  CHECK_EQUAL("    c"_sv, utl::format<10>("{0:>5}", 'c'));
  CHECK_EQUAL("  abc"_sv, utl::format<10>("{0:>5}", "abc"));
  CHECK_EQUAL("  0xface"_sv, utl::format<10>("{0:>8}", reinterpret_cast<void*>(0xface)));
}

// #if FMT_DEPRECATED_NUMERIC_ALIGN
// TEST(Format, NumericAlign) { CHECK_EQUAL("0042"_sv, format("{0:=4}", 42)); }
// #endif

TEST(Format, CenterAlign) {
  CHECK_EQUAL(" 42  "_sv, utl::format<10>("{0:^5}", 42));
  CHECK_EQUAL(" 42  "_sv, utl::format<10>("{0:^5o}", 042));
  CHECK_EQUAL(" 42  "_sv, utl::format<10>("{0:^5x}", 0x42));
  CHECK_EQUAL(" -42 "_sv, utl::format<10>("{0:^5}", -42));
  CHECK_EQUAL(" 42  "_sv, utl::format<10>("{0:^5}", 42u));
  CHECK_EQUAL(" -42 "_sv, utl::format<10>("{0:^5}", -42l));
  CHECK_EQUAL(" 42  "_sv, utl::format<10>("{0:^5}", 42ul));
  CHECK_EQUAL(" -42 "_sv, utl::format<10>("{0:^5}", -42ll));
//   CHECK_EQUAL(" 42  "_sv, utl::format<10>("{0:^5}", 42ull));
//   CHECK_EQUAL(" -42.0 "_sv, utl::format<10>("{0:^7}", -42.0));
//   CHECK_EQUAL(" -42.0 "_sv, utl::format<10>("{0:^7}", -42.0l));
  CHECK_EQUAL("  c  "_sv, utl::format<10>("{0:^5}", 'c'));
  CHECK_EQUAL(" abc  "_sv, utl::format<10>("{0:^6}", "abc"));
  CHECK_EQUAL(" 0xface "_sv, utl::format<10>("{0:^8}", reinterpret_cast<void*>(0xface)));
}

TEST(Format, Fill) {
//   EXPECT_THROW_MSG(utl::format<10>("{0:{<5}", 'c'), format_error,
//                    "invalid fill character '{'");
//   EXPECT_THROW_MSG(utl::format<10>("{0:{<5}}", 'c'), format_error,
//                    "invalid fill character '{'");
  CHECK_EQUAL("**42"_sv, utl::format<10>("{0:*>4}", 42));
  CHECK_EQUAL("**-42"_sv, utl::format<10>("{0:*>5}", -42));
  CHECK_EQUAL("***42"_sv, utl::format<10>("{0:*>5}", 42u));
  CHECK_EQUAL("**-42"_sv, utl::format<10>("{0:*>5}", -42l));
  CHECK_EQUAL("***42"_sv, utl::format<10>("{0:*>5}", 42ul));
  CHECK_EQUAL("**-42"_sv, utl::format<10>("{0:*>5}", -42ll));
//   CHECK_EQUAL("***42"_sv, utl::format<10>("{0:*>5}", 42ull));
//   CHECK_EQUAL("**-42.0"_sv, utl::format<10>("{0:*>7}", -42.0));
//   CHECK_EQUAL("**-42.0"_sv, utl::format<10>("{0:*>7}", -42.0l));
  CHECK_EQUAL("c****"_sv, utl::format<10>("{0:*<5}", 'c'));
  CHECK_EQUAL("abc**"_sv, utl::format<10>("{0:*<5}", "abc"));
  CHECK_EQUAL("**0xface"_sv, utl::format<10>("{0:*>8}", reinterpret_cast<void*>(0xface)));
  CHECK_EQUAL("foo="_sv, utl::format<10>("{:}=", "foo"));
  // CHECK_EQUAL(utl::string_view("\0\0\0*",4), utl::format<10>(utl::string_view("{:\0>4}", 6), '*'));
  // CHECK_EQUAL("жж42"_sv, utl::format<10>("{0:ж>4}", 42));
//   EXPECT_THROW_MSG(utl::format<10>("{:\x80\x80\x80\x80\x80>}", 0), format_error,
//                    "invalid fill");
}

TEST(Format, PlusSign) {
  CHECK_EQUAL("+42"_sv, utl::format<10>("{0:+}", 42));
  CHECK_EQUAL("-42"_sv, utl::format<10>("{0:+}", -42));
  CHECK_EQUAL("+42"_sv, utl::format<10>("{0:+}", 42));
//   EXPECT_THROW_MSG(utl::format<10>("{0:+}", 42u), format_error,
//                    "format specifier requires signed argument");
  CHECK_EQUAL("+42"_sv, utl::format<10>("{0:+}", 42l));
//   EXPECT_THROW_MSG(utl::format<10>("{0:+}", 42ul), format_error,
//                    "format specifier requires signed argument");
  CHECK_EQUAL("+42"_sv, utl::format<10>("{0:+}", 42ll));
//   EXPECT_THROW_MSG(utl::format<10>("{0:+}", 42ull), format_error,
//                    "format specifier requires signed argument");
//   CHECK_EQUAL("+42.0"_sv, utl::format<10>("{0:+}", 42.0));
//   CHECK_EQUAL("+42.0"_sv, utl::format<10>("{0:+}", 42.0l));
//   EXPECT_THROW_MSG(utl::format<10>("{0:+", 'c'), format_error,
//                    "missing '}' in format string");
//   EXPECT_THROW_MSG(utl::format<10>("{0:+}", 'c'), format_error,
//                    "invalid format specifier for char");
//   EXPECT_THROW_MSG(utl::format<10>("{0:+}", "abc"), format_error,
//                    "format specifier requires numeric argument");
//   EXPECT_THROW_MSG(utl::format<10>("{0:+}", reinterpret_cast<void*>(0x42)), format_error,
//                    "format specifier requires numeric argument");
}

TEST(Format, MinusSign) {
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:-}", 42));
  CHECK_EQUAL("-42"_sv, utl::format<10>("{0:-}", -42));
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:-}", 42));
//   EXPECT_THROW_MSG(utl::format<10>("{0:-}", 42u), format_error,
//                    "format specifier requires signed argument");
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:-}", 42l));
//   EXPECT_THROW_MSG(utl::format<10>("{0:-}", 42ul), format_error,
//                    "format specifier requires signed argument");
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:-}", 42ll));
//   EXPECT_THROW_MSG(utl::format<10>("{0:-}", 42ull), format_error,
//                    "format specifier requires signed argument");
//   CHECK_EQUAL("42.0"_sv, utl::format<10>("{0:-}", 42.0));
//   CHECK_EQUAL("42.0"_sv, utl::format<10>("{0:-}", 42.0l));
//   EXPECT_THROW_MSG(utl::format<10>("{0:-", 'c'), format_error,
//                    "missing '}' in format string");
//   EXPECT_THROW_MSG(utl::format<10>("{0:-}", 'c'), format_error,
//                    "invalid format specifier for char");
//   EXPECT_THROW_MSG(utl::format<10>("{0:-}", "abc"), format_error,
//                    "format specifier requires numeric argument");
//   EXPECT_THROW_MSG(utl::format<10>("{0:-}", reinterpret_cast<void*>(0x42)), format_error,
//                    "format specifier requires numeric argument");
}

TEST(Format, SpaceSign) {
  CHECK_EQUAL(" 42"_sv, utl::format<10>("{0: }", 42));
  CHECK_EQUAL("-42"_sv, utl::format<10>("{0: }", -42));
  CHECK_EQUAL(" 42"_sv, utl::format<10>("{0: }", 42));
//   EXPECT_THROW_MSG(utl::format<10>("{0: }", 42u), format_error,
//                    "format specifier requires signed argument");
  CHECK_EQUAL(" 42"_sv, utl::format<10>("{0: }", 42l));
//   EXPECT_THROW_MSG(utl::format<10>("{0: }", 42ul), format_error,
//                    "format specifier requires signed argument");
  CHECK_EQUAL(" 42"_sv, utl::format<10>("{0: }", 42ll));
//   EXPECT_THROW_MSG(utl::format<10>("{0: }", 42ull), format_error,
//                    "format specifier requires signed argument");
//   CHECK_EQUAL(" 42.0"_sv, utl::format<10>("{0: }", 42.0));
//   CHECK_EQUAL(" 42.0"_sv, utl::format<10>("{0: }", 42.0l));
//   EXPECT_THROW_MSG(utl::format<10>("{0: ", 'c'), format_error,
//                    "missing '}' in format string");
//   EXPECT_THROW_MSG(utl::format<10>("{0: }", 'c'), format_error,
//                    "invalid format specifier for char");
//   EXPECT_THROW_MSG(utl::format<10>("{0: }", "abc"), format_error,
//                    "format specifier requires numeric argument");
//   EXPECT_THROW_MSG(utl::format<10>("{0: }", reinterpret_cast<void*>(0x42)), format_error,
//                    "format specifier requires numeric argument");
}

TEST(Format, HashFlag) {
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:#}", 42));
  CHECK_EQUAL("-42"_sv, utl::format<10>("{0:#}", -42));
  CHECK_EQUAL("0b101010"_sv, utl::format<10>("{0:#b}", 42));
  CHECK_EQUAL("0B101010"_sv, utl::format<10>("{0:#B}", 42));
  CHECK_EQUAL("-0b101010"_sv, utl::format<10>("{0:#b}", -42));
  CHECK_EQUAL("0x42"_sv, utl::format<10>("{0:#x}", 0x42));
  CHECK_EQUAL("0X42"_sv, utl::format<10>("{0:#X}", 0x42));
  CHECK_EQUAL("-0x42"_sv, utl::format<10>("{0:#x}", -0x42));
  CHECK_EQUAL("0"_sv, utl::format<10>("{0:#o}", 0));
  CHECK_EQUAL("042"_sv, utl::format<10>("{0:#o}", 042));
  CHECK_EQUAL("-042"_sv, utl::format<10>("{0:#o}", -042));
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:#}", 42u));
  CHECK_EQUAL("0x42"_sv, utl::format<10>("{0:#x}", 0x42u));
  CHECK_EQUAL("042"_sv, utl::format<10>("{0:#o}", 042u));

  CHECK_EQUAL("-42"_sv, utl::format<10>("{0:#}", -42l));
  CHECK_EQUAL("0x42"_sv, utl::format<10>("{0:#x}", 0x42l));
  CHECK_EQUAL("-0x42"_sv, utl::format<10>("{0:#x}", -0x42l));
  CHECK_EQUAL("042"_sv, utl::format<10>("{0:#o}", 042l));
  CHECK_EQUAL("-042"_sv, utl::format<10>("{0:#o}", -042l));
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:#}", 42ul));
  CHECK_EQUAL("0x42"_sv, utl::format<10>("{0:#x}", 0x42ul));
  CHECK_EQUAL("042"_sv, utl::format<10>("{0:#o}", 042ul));

  CHECK_EQUAL("-42"_sv, utl::format<10>("{0:#}", -42ll));
  CHECK_EQUAL("0x42"_sv, utl::format<10>("{0:#x}", 0x42ll));
  CHECK_EQUAL("-0x42"_sv, utl::format<10>("{0:#x}", -0x42ll));
  CHECK_EQUAL("042"_sv, utl::format<10>("{0:#o}", 042ll));
  CHECK_EQUAL("-042"_sv, utl::format<10>("{0:#o}", -042ll));
//   CHECK_EQUAL("42"_sv, utl::format<10>("{0:#}", 42ull));
//   CHECK_EQUAL("0x42"_sv, utl::format<10>("{0:#x}", 0x42ull));
//   CHECK_EQUAL("042"_sv, utl::format<10>("{0:#o}", 042ull));

//   CHECK_EQUAL("-42.0"_sv, utl::format<10>("{0:#}", -42.0));
//   CHECK_EQUAL("-42.0"_sv, utl::format<10>("{0:#}", -42.0l));
//   CHECK_EQUAL("4.e+01"_sv, utl::format<10>("{:#.0e}", 42.0));
//   CHECK_EQUAL("0."_sv, utl::format<10>("{:#.0f}", 0.01));
//   auto s = utl::format<10>("{:#.0f}", 0.5);  // MSVC's printf uses wrong rounding mode.
//   EXPECT_TRUE(s == "0." || s == "1.");
//   EXPECT_THROW_MSG(utl::format<10>("{0:#", 'c'), format_error,
//                    "missing '}' in format string");
//   EXPECT_THROW_MSG(utl::format<10>("{0:#}", 'c'), format_error,
//                    "invalid format specifier for char");
//   EXPECT_THROW_MSG(utl::format<10>("{0:#}", "abc"), format_error,
//                    "format specifier requires numeric argument");
//   EXPECT_THROW_MSG(utl::format<10>("{0:#}", reinterpret_cast<void*>(0x42)), format_error,
//                    "format specifier requires numeric argument");
}

TEST(Format, ZeroFlag) {
  CHECK_EQUAL("42"_sv, utl::format<20>("{0:0}", 42));
  CHECK_EQUAL("-0042"_sv, utl::format<20>("{0:05}", -42));
  CHECK_EQUAL("00042"_sv, utl::format<20>("{0:05}", 42u));
  CHECK_EQUAL("-0042"_sv, utl::format<20>("{0:05}", -42l));
  CHECK_EQUAL("00042"_sv, utl::format<20>("{0:05}", 42ul));
  CHECK_EQUAL("-0042"_sv, utl::format<20>("{0:05}", -42ll));
  // CHECK_EQUAL("00042"_sv, utl::format<20>("{0:05}", 42ull));
  // CHECK_EQUAL("-0042.0"_sv, utl::format<20>("{0:07}", -42.0));
  // CHECK_EQUAL("-0042.0"_sv, utl::format<20>("{0:07}", -42.0l));
  // EXPECT_THROW_MSG(utl::format<20>("{0:0", 'c'), format_error,
  //                  "missing '}' in format string");
  // EXPECT_THROW_MSG(utl::format<20>("{0:05}", 'c'), format_error,
  //                  "invalid format specifier for char");
  // EXPECT_THROW_MSG(utl::format<20>("{0:05}", "abc"), format_error,
  //                  "format specifier requires numeric argument");
  // EXPECT_THROW_MSG(utl::format<20>("{0:05}", reinterpret_cast<void*>(0x42)),
  //                  format_error, "format specifier requires numeric argument");
}

TEST(Format, Width) {
  // char format_str[BUFFER_SIZE];
  // safe_sprintf(format_str, "{0:%u", UINT_MAX);
  // increment(format_str + 3);
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");
  // size_t size = std::strlen(format_str);
  // format_str[size] = '}';
  // format_str[size + 1] = 0;
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");

  // safe_sprintf(format_str, "{0:%u", INT_MAX + 1u);
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");
  // safe_sprintf(format_str, "{0:%u}", INT_MAX + 1u);
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");
  CHECK_EQUAL(" -42"_sv, utl::format<20>("{0:4}", -42));
  CHECK_EQUAL("   42"_sv, utl::format<20>("{0:5}", 42u));
  CHECK_EQUAL("   -42"_sv, utl::format<20>("{0:6}", -42l));
  CHECK_EQUAL("     42"_sv, utl::format<20>("{0:7}", 42ul));
  CHECK_EQUAL("   -42"_sv, utl::format<20>("{0:6}", -42ll));
  // CHECK_EQUAL("     42"_sv, utl::format<20>("{0:7}", 42ull));
  // CHECK_EQUAL("   -1.23"_sv, utl::format<20>("{0:8}", -1.23));
  // CHECK_EQUAL("    -1.23"_sv, utl::format<20>("{0:9}", -1.23l));
  CHECK_EQUAL("    0xcafe"_sv, utl::format<20>("{0:10}", reinterpret_cast<void*>(0xcafe)));
  CHECK_EQUAL("x          "_sv, utl::format<20>("{0:11}", 'x'));
  CHECK_EQUAL("str         "_sv, utl::format<20>("{0:12}", "str"));
  // CHECK_EQUAL("**🤡**"_sv, utl::format<20>("{:*^5}", "🤡"));
}

template <typename T> inline T const_check(T value) { return value; }

TEST(Format, RuntimeWidth) {
  // char format_str[BUFFER_SIZE];
  // safe_sprintf(format_str, "{0:{%u", UINT_MAX);
  // increment(format_str + 4);
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");
  // size_t size = std::strlen(format_str);
  // format_str[size] = '}';
  // format_str[size + 1] = 0;
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");
  // format_str[size + 1] = '}';
  // format_str[size + 2] = 0;
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");

  // EXPECT_THROW_MSG(utl::format<20>("{0:{", 0), format_error, "invalid format string");
  // EXPECT_THROW_MSG(utl::format<20>("{0:{}", 0), format_error,
                  //  "cannot switch from manual to automatic argument indexing");
  // EXPECT_THROW_MSG(utl::format<20>("{0:{?}}", 0), format_error, "invalid format string");
  // EXPECT_THROW_MSG(utl::format<20>("{0:{1}}", 0), format_error, "argument not found");

  // EXPECT_THROW_MSG(utl::format<20>("{0:{0:}}", 0), format_error,
                  //  "invalid format string");

  // EXPECT_THROW_MSG(utl::format<20>("{0:{1}}", 0, -1), format_error, "negative width");
  // EXPECT_THROW_MSG(utl::format<20>("{0:{1}}", 0, (INT_MAX + 1u)), format_error,
  //                  "number is too big");
  // EXPECT_THROW_MSG(utl::format<20>("{0:{1}}", 0, -1l), format_error, "negative width");
  // if (const_check(sizeof(long) > sizeof(int))) {
  //   long value = INT_MAX;
  //   EXPECT_THROW_MSG(utl::format<20>("{0:{1}}", 0, (value + 1)), format_error,
  //                    "number is too big");
  // }
  // EXPECT_THROW_MSG(utl::format<20>("{0:{1}}", 0, (INT_MAX + 1ul)), format_error,
  //                  "number is too big");

  // EXPECT_THROW_MSG(utl::format<20>("{0:{1}}", 0, '0'), format_error,
  //                  "width is not integer");
  // EXPECT_THROW_MSG(utl::format<20>("{0:{1}}", 0, 0.0), format_error,
  //                  "width is not integer");

  CHECK_EQUAL(" -42"_sv, utl::format<20>("{0:{1}}", -42, 4));
  CHECK_EQUAL("   42"_sv, utl::format<20>("{0:{1}}", 42u, 5));
  CHECK_EQUAL("   -42"_sv, utl::format<20>("{0:{1}}", -42l, 6));
  CHECK_EQUAL("     42"_sv, utl::format<20>("{0:{1}}", 42ul, 7));
  CHECK_EQUAL("   -42"_sv, utl::format<20>("{0:{1}}", -42ll, 6));
  // CHECK_EQUAL("     42"_sv, utl::format<20>("{0:{1}}", 42ull, 7));
  // CHECK_EQUAL("   -1.23"_sv, utl::format<20>("{0:{1}}", -1.23, 8));
  // CHECK_EQUAL("    -1.23"_sv, utl::format<20>("{0:{1}}", -1.23l, 9));
  CHECK_EQUAL("    0xcafe"_sv,
            utl::format<20>("{0:{1}}", reinterpret_cast<void*>(0xcafe), 10));
  CHECK_EQUAL("x          "_sv, utl::format<20>("{0:{1}}", 'x', 11));
  CHECK_EQUAL("str         "_sv, utl::format<20>("{0:{1}}", "str", 12));
}

TEST(Format, Precision) {
  // char format_str[BUFFER_SIZE];
  // safe_sprintf(format_str, "{0:.%u", UINT_MAX);
  // increment(format_str + 4);
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");
  // size_t size = std::strlen(format_str);
  // format_str[size] = '}';
  // format_str[size + 1] = 0;
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");

  // safe_sprintf(format_str, "{0:.%u", INT_MAX + 1u);
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");
  // safe_sprintf(format_str, "{0:.%u}", INT_MAX + 1u);
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");

  // EXPECT_THROW_MSG(utl::format<20>("{0:.", 0), format_error,
  //                  "missing precision specifier");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.}", 0), format_error,
  //                  "missing precision specifier");

  // EXPECT_THROW_MSG(utl::format<20>("{0:.2", 0), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2}", 42), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2f}", 42), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2}", 42u), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2f}", 42u), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2}", 42l), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2f}", 42l), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2}", 42ul), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2f}", 42ul), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2}", 42ll), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2f}", 42ll), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2}", 42ull), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2f}", 42ull), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:3.0}", 'x'), format_error,
  //                  "precision not allowed for this argument type");
  // CHECK_EQUAL("1.2"_sv, utl::format<20>("{0:.2}", 1.2345));
  // CHECK_EQUAL("1.2"_sv, utl::format<20>("{0:.2}", 1.2345l));
  // CHECK_EQUAL("1.2e+56"_sv, utl::format<20>("{:.2}", 1.234e56));
  // CHECK_EQUAL("1e+00"_sv, utl::format<20>("{:.0e}", 1.0L));
  // CHECK_EQUAL("  0.0e+00"_sv, utl::format<20>("{:9.1e}", 0.0));
  // CHECK_EQUAL(
  //     "4.9406564584124654417656879286822137236505980261432476442558568250067550"
  //     "727020875186529983636163599237979656469544571773092665671035593979639877"
  //     "479601078187812630071319031140452784581716784898210368871863605699873072"
  //     "305000638740915356498438731247339727316961514003171538539807412623856559"
  //     "117102665855668676818703956031062493194527159149245532930545654440112748"
  //     "012970999954193198940908041656332452475714786901472678015935523861155013"
  //     "480352649347201937902681071074917033322268447533357208324319361e-324",
  //     utl::format<550>("{:.494}", 4.9406564584124654E-324));
  // CHECK_EQUAL(
  //     "-0X1.41FE3FFE71C9E000000000000000000000000000000000000000000000000000000"
  //     "000000000000000000000000000000000000000000000000000000000000000000000000"
  //     "000000000000000000000000000000000000000000000000000000000000000000000000"
  //     "000000000000000000000000000000000000000000000000000000000000000000000000"
  //     "000000000000000000000000000000000000000000000000000000000000000000000000"
  //     "000000000000000000000000000000000000000000000000000000000000000000000000"
  //     "000000000000000000000000000000000000000000000000000000000000000000000000"
  //     "000000000000000000000000000000000000000000000000000000000000000000000000"
  //     "000000000000000000000000000000000000000000000000000000000000000000000000"
  //     "000000000000000000000000000000000000000000000000000000000000000000000000"
  //     "000000000000000000000000000000000000000000000000000000000000000000000000"
  //     "000000000000000000000000000000000000000000000000000P+127",
  //     utl::format<20>("{:.838A}", -2.14001164E+38));
  // CHECK_EQUAL("123."_sv, utl::format<20>("{:#.0f}", 123.0));
  // CHECK_EQUAL("1.23"_sv, utl::format<20>("{:.02f}", 1.234));
  // CHECK_EQUAL("0.001"_sv, utl::format<20>("{:.1g}", 0.001));

  // EXPECT_THROW_MSG(utl::format<20>("{0:.2}", reinterpret_cast<void*>(0xcafe)),
  //                  format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.2f}", reinterpret_cast<void*>(0xcafe)),
  //                  format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{:.{}e}", 42.0, utl::detail::max_value<int>()),
  //                  format_error, "number is too big");

  CHECK_EQUAL("st"_sv, utl::format<20>("{0:.2}", "str"));
}

TEST(Format, RuntimePrecision) {
  // char format_str[BUFFER_SIZE];
  // safe_sprintf(format_str, "{0:.{%u", UINT_MAX);
  // increment(format_str + 5);
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");
  // size_t size = std::strlen(format_str);
  // format_str[size] = '}';
  // format_str[size + 1] = 0;
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");
  // format_str[size + 1] = '}';
  // format_str[size + 2] = 0;
  // EXPECT_THROW_MSG(utl::format<20>(format_str, 0), format_error, "number is too big");

  // EXPECT_THROW_MSG(utl::format<20>("{0:.{", 0), format_error, "invalid format string");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{}", 0), format_error,
  //                  "cannot switch from manual to automatic argument indexing");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{?}}", 0), format_error,
  //                  "invalid format string");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}", 0, 0), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 0), format_error, "argument not found");

  // EXPECT_THROW_MSG(utl::format<20>("{0:.{0:}}", 0), format_error,
  //                  "invalid format string");

  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 0, -1), format_error,
  //                  "negative precision");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 0, (INT_MAX + 1u)), format_error,
  //                  "number is too big");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 0, -1l), format_error,
  //                  "negative precision");
  // if (const_check(sizeof(long) > sizeof(int))) {
  //   long value = INT_MAX;
  //   EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 0, (value + 1)), format_error,
  //                    "number is too big");
  // }
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 0, (INT_MAX + 1ul)), format_error,
  //                  "number is too big");

  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 0, '0'), format_error,
  //                  "precision is not integer");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 0, 0.0), format_error,
  //                  "precision is not integer");

  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 42, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}f}", 42, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 42u, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}f}", 42u, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 42l, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}f}", 42l, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 42ul, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}f}", 42ul, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 42ll, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}f}", 42ll, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", 42ull, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}f}", 42ull, 2), format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:3.{1}}", 'x', 0), format_error,
  //                  "precision not allowed for this argument type");
  // CHECK_EQUAL("1.2"_sv, utl::format<20>("{0:.{1}}", 1.2345, 2));
  // CHECK_EQUAL("1.2"_sv, utl::format<20>("{1:.{0}}", 2, 1.2345l));

  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}}", reinterpret_cast<void*>(0xcafe), 2),
  //                  format_error,
  //                  "precision not allowed for this argument type");
  // EXPECT_THROW_MSG(utl::format<20>("{0:.{1}f}", reinterpret_cast<void*>(0xcafe), 2),
  //                  format_error,
  //                  "precision not allowed for this argument type");

  CHECK_EQUAL("st"_sv, utl::format<20>("{0:.{1}}", "str", 2));
}

// template <typename T>
// void check_unknown_types(const T& value, const char* types, const char*) {
//   char format_str[BUFFER_SIZE];
//   const char* special = ".0123456789}";
//   for (int i = CHAR_MIN; i <= CHAR_MAX; ++i) {
//     char c = static_cast<char>(i);
//     if (std::strchr(types, c) || std::strchr(special, c) || !c) continue;
//     safe_sprintf(format_str, "{0:10%c}", c);
//     const char* message = "invalid type specifier";
//     EXPECT_THROW_MSG(format(format_str, value), format_error, message)
//         << format_str << " " << message;
//   }
// }

TEST(Format, FormatBool) {
  CHECK_EQUAL("true"_sv, utl::format<10>("{}", true));
  CHECK_EQUAL("false"_sv, utl::format<10>("{}", false));
  CHECK_EQUAL("1"_sv, utl::format<10>("{:d}", true));
  CHECK_EQUAL("true "_sv, utl::format<10>("{:5}", true));
  // CHECK_EQUAL(L"true"_sv, utl::format<10>(L"{}", true));
}

TEST(Format, FormatShort) {
  short s = 42;
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:d}", s));
  unsigned short us = 42;
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:d}", us));
}

TEST(Format, FormatInt) {
  // EXPECT_THROW_MSG(utl::format<10>("{0:v", 42), format_error,
  //                  "missing '}' in format string");
  // check_unknown_types(42, "bBdoxXnLc", "integer");
  // CHECK_EQUAL("x"_sv, utl::format<10>("{:c}", static_cast<int>('x')));
}

TEST(Format, FormatBin) {
  CHECK_EQUAL("0"_sv, utl::format<10>("{0:b}", 0));
  CHECK_EQUAL("101010"_sv, utl::format<10>("{0:b}", 42));
  CHECK_EQUAL("101010"_sv, utl::format<10>("{0:b}", 42u));
  CHECK_EQUAL("-101010"_sv, utl::format<10>("{0:b}", -42));
  CHECK_EQUAL("11000000111001"_sv, utl::format<20>("{0:b}", 12345));
  CHECK_EQUAL("10010001101000101011001111000"_sv, utl::format<50>("{0:b}", 0x12345678));
  CHECK_EQUAL("10010000101010111100110111101111"_sv, utl::format<50>("{0:b}", 0x90ABCDEF));
  CHECK_EQUAL("11111111111111111111111111111111"_sv,
            utl::format<50>("{0:b}", std::numeric_limits<uint32_t>::max()));
}

// #if FMT_USE_INT128
// constexpr auto int128_max = static_cast<__int128_t>(
//     (static_cast<__uint128_t>(1) << ((__SIZEOF_INT128__ * CHAR_BIT) - 1)) - 1);
// constexpr auto int128_min = -int128_max - 1;

// constexpr auto uint128_max = ~static_cast<__uint128_t>(0);
// #endif

static constexpr size_t BUFFER_SIZE = 100;

TEST(Format, FormatDec) {
  CHECK_EQUAL("0"_sv, utl::format<10>("{0}", 0));
  CHECK_EQUAL("42"_sv, utl::format<10>("{0}", 42));
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:d}", 42));
  CHECK_EQUAL("42"_sv, utl::format<10>("{0}", 42u));
  CHECK_EQUAL("-42"_sv, utl::format<10>("{0}", -42));
  CHECK_EQUAL("12345"_sv, utl::format<10>("{0}", 12345));
  CHECK_EQUAL("67890"_sv, utl::format<10>("{0}", 67890));
// #if FMT_USE_INT128
//   CHECK_EQUAL("0"_sv, utl::format<10>("{0}", static_cast<__int128_t>(0)));
//   CHECK_EQUAL("0"_sv, utl::format<10>("{0}", static_cast<__uint128_t>(0)));
//   CHECK_EQUAL("9223372036854775808",
//             utl::format<10>("{0}", static_cast<__int128_t>(INT64_MAX) + 1));
//   CHECK_EQUAL("-9223372036854775809",
//             utl::format<10>("{0}", static_cast<__int128_t>(INT64_MIN) - 1));
//   CHECK_EQUAL("18446744073709551616",
//             utl::format<10>("{0}", static_cast<__int128_t>(UINT64_MAX) + 1));
//   CHECK_EQUAL("170141183460469231731687303715884105727",
//             utl::format<10>("{0}", int128_max));
//   CHECK_EQUAL("-170141183460469231731687303715884105728",
//             utl::format<10>("{0}", int128_min));
//   CHECK_EQUAL("340282366920938463463374607431768211455",
//             utl::format<10>("{0}", uint128_max));
// #endif

  char buffer[BUFFER_SIZE]{};
  snprintf(buffer, BUFFER_SIZE, "%d", INT_MIN);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<20>("{0}", INT_MIN));
  snprintf(buffer, BUFFER_SIZE, "%d", INT_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<20>("{0}", INT_MAX));
  snprintf(buffer, BUFFER_SIZE, "%u", UINT_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<20>("{0}", UINT_MAX));
  snprintf(buffer, BUFFER_SIZE, "%ld", 0 - static_cast<unsigned long>(LONG_MIN));
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<20>("{0}", LONG_MIN));
  snprintf(buffer, BUFFER_SIZE, "%ld", LONG_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<20>("{0}", LONG_MAX));
  snprintf(buffer, BUFFER_SIZE, "%lu", ULONG_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<20>("{0}", ULONG_MAX));
}

TEST(Format, FormatHex) {
  CHECK_EQUAL("0"_sv, utl::format<10>("{0:x}", 0));
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:x}", 0x42));
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:x}", 0x42u));
  CHECK_EQUAL("-42"_sv, utl::format<10>("{0:x}", -0x42));
  CHECK_EQUAL("12345678"_sv, utl::format<10>("{0:x}", 0x12345678));
  CHECK_EQUAL("90abcdef"_sv, utl::format<10>("{0:x}", 0x90abcdef));
  CHECK_EQUAL("12345678"_sv, utl::format<10>("{0:X}", 0x12345678));
  CHECK_EQUAL("90ABCDEF"_sv, utl::format<10>("{0:X}", 0x90ABCDEF));
// #if FMT_USE_INT128
//   CHECK_EQUAL("0"_sv, utl::format<10>("{0:x}", static_cast<__int128_t>(0)));
//   CHECK_EQUAL("0"_sv, utl::format<10>("{0:x}", static_cast<__uint128_t>(0)));
//   CHECK_EQUAL("8000000000000000",
//             utl::format<10>("{0:x}", static_cast<__int128_t>(INT64_MAX) + 1));
//   CHECK_EQUAL("-8000000000000001",
//             utl::format<10>("{0:x}", static_cast<__int128_t>(INT64_MIN) - 1));
//   CHECK_EQUAL("10000000000000000",
//             utl::format<10>("{0:x}", static_cast<__int128_t>(UINT64_MAX) + 1));
//   CHECK_EQUAL("7fffffffffffffffffffffffffffffff"_sv, utl::format<10>("{0:x}", int128_max));
//   CHECK_EQUAL("-80000000000000000000000000000000"_sv, utl::format<10>("{0:x}", int128_min));
//   CHECK_EQUAL("ffffffffffffffffffffffffffffffff"_sv, utl::format<10>("{0:x}", uint128_max));
// #endif

  char buffer[BUFFER_SIZE]{};
  snprintf(buffer, BUFFER_SIZE, "-%x", 0 - static_cast<unsigned>(INT_MIN));
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<10>("{0:x}", INT_MIN));
  snprintf(buffer, BUFFER_SIZE, "%x", INT_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<30>("{0:x}", INT_MAX));
  snprintf(buffer, BUFFER_SIZE, "%x", UINT_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<30>("{0:x}", UINT_MAX));
  snprintf(buffer, BUFFER_SIZE, "-%lx", 0 - static_cast<unsigned long>(LONG_MIN));
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<30>("{0:x}", LONG_MIN));
  snprintf(buffer, BUFFER_SIZE, "%lx", LONG_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<30>("{0:x}", LONG_MAX));
  snprintf(buffer, BUFFER_SIZE, "%lx", ULONG_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<30>("{0:x}", ULONG_MAX));
}

TEST(Format, FormatOct) {
  CHECK_EQUAL("0"_sv, utl::format<10>("{0:o}", 0));
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:o}", 042));
  CHECK_EQUAL("42"_sv, utl::format<10>("{0:o}", 042u));
  CHECK_EQUAL("-42"_sv, utl::format<10>("{0:o}", -042));
  CHECK_EQUAL("12345670"_sv, utl::format<10>("{0:o}", 012345670));
// #if FMT_USE_INT128
//   CHECK_EQUAL("0"_sv, utl::format<10>("{0:o}", static_cast<__int128_t>(0)));
//   CHECK_EQUAL("0"_sv, utl::format<10>("{0:o}", static_cast<__uint128_t>(0)));
//   CHECK_EQUAL("1000000000000000000000",
//             utl::format<10>("{0:o}", static_cast<__int128_t>(INT64_MAX) + 1));
//   CHECK_EQUAL("-1000000000000000000001",
//             utl::format<10>("{0:o}", static_cast<__int128_t>(INT64_MIN) - 1));
//   CHECK_EQUAL("2000000000000000000000",
//             utl::format<10>("{0:o}", static_cast<__int128_t>(UINT64_MAX) + 1));
//   CHECK_EQUAL("1777777777777777777777777777777777777777777",
//             utl::format<10>("{0:o}", int128_max));
//   CHECK_EQUAL("-2000000000000000000000000000000000000000000",
//             utl::format<10>("{0:o}", int128_min));
//   CHECK_EQUAL("3777777777777777777777777777777777777777777",
//             utl::format<10>("{0:o}", uint128_max));
// #endif

  char buffer[BUFFER_SIZE];
  snprintf(buffer, BUFFER_SIZE, "-%o", 0 - static_cast<unsigned>(INT_MIN));
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<50>("{0:o}", INT_MIN));
  snprintf(buffer, BUFFER_SIZE, "%o", INT_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<50>("{0:o}", INT_MAX));
  snprintf(buffer, BUFFER_SIZE, "%o", UINT_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<50>("{0:o}", UINT_MAX));
  snprintf(buffer, BUFFER_SIZE, "-%lo", 0 - static_cast<unsigned long>(LONG_MIN));
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<50>("{0:o}", LONG_MIN));
  snprintf(buffer, BUFFER_SIZE, "%lo", LONG_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<50>("{0:o}", LONG_MAX));
  snprintf(buffer, BUFFER_SIZE, "%lo", ULONG_MAX);
  CHECK_EQUAL(utl::string_view{buffer}, utl::format<50>("{0:o}", ULONG_MAX));
}

TEST(Format, FormatIntLocale) {
  CHECK_EQUAL("1234"_sv, utl::format<10>("{:L}", 1234));
}

// struct ConvertibleToLongLong {
//   operator long long() const { return 1LL << 32; }
// };

// TEST(Format, FormatConvertibleToLongLong) {
//   CHECK_EQUAL("100000000"_sv, utl::format<10>("{:x}", ConvertibleToLongLong()));
// }

// TEST(Format, FormatFloat) {
//   CHECK_EQUAL("392.500000"_sv, utl::format<10>("{0:f}", 392.5f));
// }

// TEST(Format, FormatDouble) {
//   check_unknown_types(1.2, "eEfFgGaAnL%", "double");
//   CHECK_EQUAL("0.0"_sv, utl::format<10>("{:}", 0.0));
//   CHECK_EQUAL("0.000000"_sv, utl::format<10>("{:f}", 0.0));
//   CHECK_EQUAL("0"_sv, utl::format<10>("{:g}", 0.0));
//   CHECK_EQUAL("392.65"_sv, utl::format<10>("{:}", 392.65));
//   CHECK_EQUAL("392.65"_sv, utl::format<10>("{:g}", 392.65));
//   CHECK_EQUAL("392.65"_sv, utl::format<10>("{:G}", 392.65));
//   CHECK_EQUAL("392.650000"_sv, utl::format<10>("{:f}", 392.65));
//   CHECK_EQUAL("392.650000"_sv, utl::format<10>("{:F}", 392.65));
//   CHECK_EQUAL("42"_sv, utl::format<10>("{:L}", 42.0));
//   char buffer[BUFFER_SIZE];
//   snprintf(buffer, BUFFER_SIZE, "%e", 392.65);
//   CHECK_EQUAL(utl::string_view{buffer}, utl::format<10>("{0:e}", 392.65));
//   snprintf(buffer, BUFFER_SIZE, "%E", 392.65);
//   CHECK_EQUAL(utl::string_view{buffer}, utl::format<10>("{0:E}", 392.65));
//   CHECK_EQUAL("+0000392.6"_sv, utl::format<10>("{0:+010.4g}", 392.65));
//   snprintf(buffer, BUFFER_SIZE, "%a", -42.0);
//   CHECK_EQUAL(utl::string_view{buffer}, utl::format<10>("{:a}", -42.0));
//   snprintf(buffer, BUFFER_SIZE, "%A", -42.0);
//   CHECK_EQUAL(utl::string_view{buffer}, utl::format<10>("{:A}", -42.0));
// }

// TEST(Format, PrecisionRounding) {
//   CHECK_EQUAL("0"_sv, utl::format<10>("{:.0f}", 0.0));
//   CHECK_EQUAL("0"_sv, utl::format<10>("{:.0f}", 0.01));
//   CHECK_EQUAL("0"_sv, utl::format<10>("{:.0f}", 0.1));
//   CHECK_EQUAL("0.000"_sv, utl::format<10>("{:.3f}", 0.00049));
//   CHECK_EQUAL("0.001"_sv, utl::format<10>("{:.3f}", 0.0005));
//   CHECK_EQUAL("0.001"_sv, utl::format<10>("{:.3f}", 0.00149));
//   CHECK_EQUAL("0.002"_sv, utl::format<10>("{:.3f}", 0.0015));
//   CHECK_EQUAL("1.000"_sv, utl::format<10>("{:.3f}", 0.9999));
//   CHECK_EQUAL("0.00123"_sv, utl::format<10>("{:.3}", 0.00123));
//   CHECK_EQUAL("0.1"_sv, utl::format<10>("{:.16g}", 0.1));
//   // Trigger rounding error in Grisu by a carefully chosen number.
//   auto n = 3788512123356.985352;
//   char buffer[64];
//   snprintf(buffer, BUFFER_SIZE, "%f", n);
//   CHECK_EQUAL(utl::string_view{buffer}, utl::format<10>("{:f}", n));
// }

// TEST(Format, FormatNaN) {
//   double nan = std::numeric_limits<double>::quiet_NaN();
//   CHECK_EQUAL("nan"_sv, utl::format<10>("{}", nan));
//   CHECK_EQUAL("+nan"_sv, utl::format<10>("{:+}", nan));
//   CHECK_EQUAL(" nan"_sv, utl::format<10>("{: }", nan));
//   CHECK_EQUAL("NAN"_sv, utl::format<10>("{:F}", nan));
//   CHECK_EQUAL("nan    "_sv, utl::format<10>("{:<7}", nan));
//   CHECK_EQUAL("  nan  "_sv, utl::format<10>("{:^7}", nan));
//   CHECK_EQUAL("    nan"_sv, utl::format<10>("{:>7}", nan));
// }

// TEST(Format, FormatInfinity) {
//   double inf = std::numeric_limits<double>::infinity();
//   CHECK_EQUAL("inf"_sv, utl::format<10>("{}", inf));
//   CHECK_EQUAL("+inf"_sv, utl::format<10>("{:+}", inf));
//   CHECK_EQUAL("-inf"_sv, utl::format<10>("{}", -inf));
//   CHECK_EQUAL(" inf"_sv, utl::format<10>("{: }", inf));
//   CHECK_EQUAL("INF"_sv, utl::format<10>("{:F}", inf));
//   CHECK_EQUAL("inf    "_sv, utl::format<10>("{:<7}", inf));
//   CHECK_EQUAL("  inf  "_sv, utl::format<10>("{:^7}", inf));
//   CHECK_EQUAL("    inf"_sv, utl::format<10>("{:>7}", inf));
// }

// TEST(Format, FormatLongDouble) {
//   CHECK_EQUAL("0.0"_sv, utl::format<10>("{0:}", 0.0l));
//   CHECK_EQUAL("0.000000"_sv, utl::format<10>("{0:f}", 0.0l));
//   CHECK_EQUAL("392.65"_sv, utl::format<10>("{0:}", 392.65l));
//   CHECK_EQUAL("392.65"_sv, utl::format<10>("{0:g}", 392.65l));
//   CHECK_EQUAL("392.65"_sv, utl::format<10>("{0:G}", 392.65l));
//   CHECK_EQUAL("392.650000"_sv, utl::format<10>("{0:f}", 392.65l));
//   CHECK_EQUAL("392.650000"_sv, utl::format<10>("{0:F}", 392.65l));
//   char buffer[BUFFER_SIZE];
//   snprintf(buffer, BUFFER_SIZE, "%Le", 392.65l);
//   CHECK_EQUAL(utl::string_view{buffer}, utl::format<10>("{0:e}", 392.65l));
//   CHECK_EQUAL("+0000392.6"_sv, utl::format<10>("{0:+010.4g}", 392.64l));
//   snprintf(buffer, BUFFER_SIZE, "%La", 3.31l);
//   CHECK_EQUAL(utl::string_view{buffer}, utl::format<10>("{:a}", 3.31l));
// }

TEST(Format, FormatChar) {
  // const char types[] = "cbBdoxXL";
  const utl::string types{"bBdoxXL"};
  // check_unknown_types('a', types, "char");
  CHECK_EQUAL("a"_sv, utl::format<10>("{0}", 'a'));
  CHECK_EQUAL("z"_sv, utl::format<10>("{0:c}", 'z'));
  CHECK_EQUAL("x"_sv, utl::format<10>("{:c}", 'x'));
  // CHECK_EQUAL(L"a"_sv, utl::format<10>(L"{0}", 'a'));
  int n = 'x';
  for(const char type : types) {
    auto format_str = utl::format<10>("{{:{}}}", type); //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    CHECK_EQUAL(utl::format<10>(format_str, n), utl::format<10>(format_str, 'x'));
  }
  CHECK_EQUAL(utl::format<10>("{:02X}", n), utl::format<10>("{:02X}", 'x'));
}

TEST(Format, FormatVolatileChar) {
  volatile char c = 'x';
  CHECK_EQUAL("x"_sv, utl::format<10>("{}", c));
}

TEST(Format, FormatUnsignedChar) {
  CHECK_EQUAL("42"_sv, utl::format<10>("{}", static_cast<unsigned char>(42)));
  CHECK_EQUAL("42"_sv, utl::format<10>("{}", static_cast<uint8_t>(42)));
}

TEST(Format, FormatWChar) {
  // CHECK_EQUAL(L"a"_sv, utl::format<10>(L"{0}", L'a'));
  // This shouldn't compile:
  // utl::format<10>("{}", L'a');
}

TEST(Format, FormatCString) {
  // check_unknown_types("test", "sp", "string");
  CHECK_EQUAL("test"_sv, utl::format<10>("{0}", "test"));
  CHECK_EQUAL("test"_sv, utl::format<10>("{0:s}", "test"));
  char nonconst[] = "nonconst"; //NOLINT(cppcoreguidelines-avoid-c-arrays)
  CHECK_EQUAL("nonconst"_sv, utl::format<10>("{0}", nonconst));
  // EXPECT_THROW_MSG(utl::format<10>("{0}", static_cast<const char*>(nullptr)),
  //                  format_error, "string pointer is null");
}

TEST(Format, FormatSCharString) {
  signed char str[] = "test"; //NOLINT(cppcoreguidelines-avoid-c-arrays)
  CHECK_EQUAL("test"_sv, utl::format<10>("{0:s}", str));
  const signed char* const_str = str;
  //NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  CHECK_EQUAL("test"_sv, utl::format<10>("{0:s}", const_str)); 
}

TEST(Format, FormatUCharString) {
  unsigned char str[] = "test";
  CHECK_EQUAL("test"_sv, utl::format<10>("{0:s}", str));
  const unsigned char* const_str = str;
  //NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  CHECK_EQUAL("test"_sv, utl::format<10>("{0:s}", const_str));
  unsigned char* ptr = str;
  //NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  CHECK_EQUAL("test"_sv, utl::format<10>("{0:s}", ptr));
}

TEST(Format, FormatPointer) {
  // check_unknown_types(reinterpret_cast<void*>(0x1234), "p", "pointer");
  CHECK_EQUAL("0x0"_sv, utl::format<10>("{0}", static_cast<void*>(nullptr)));
  CHECK_EQUAL("0x1234"_sv, utl::format<10>("{0}", reinterpret_cast<void*>(0x1234)));
  CHECK_EQUAL("0x1234"_sv, utl::format<10>("{0:p}", reinterpret_cast<void*>(0x1234)));
  CHECK_EQUAL(utl::format<50>("0x{}", utl::string<50>(sizeof(void*) * CHAR_BIT / 4, 'f')),
            utl::format<50>("{0}", reinterpret_cast<void*>(~uintptr_t())));
  // CHECK_EQUAL("0x1234"_sv, utl::format<10>("{}", utl::ptr(reinterpret_cast<int*>(0x1234))));
  // std::unique_ptr<int> up(new int(1));
  // CHECK_EQUAL(utl::format<10>("{}", utl::ptr(up.get())), utl::format<10>("{}", utl::ptr(up)));
  // std::shared_ptr<int> sp(new int(1));
  // CHECK_EQUAL(utl::format<10>("{}", utl::ptr(sp.get())), utl::format<10>("{}", utl::ptr(sp)));
  CHECK_EQUAL("0x0"_sv, utl::format<10>("{}", nullptr));
}

TEST(Format, FormatString) {
  CHECK_EQUAL("test"_sv, utl::format<10>("{0}", utl::string("test")));
}

TEST(Format, FormatStringView) {
  CHECK_EQUAL("test"_sv, utl::format<10>("{}", "test"_sv));
  CHECK_EQUAL(""_sv, utl::format<10>("{}", ""_sv));
}

// #ifdef FMT_USE_STRING_VIEW
struct string_viewable {};

// FMT_BEGIN_NAMESPACE
constexpr void _format(string_viewable, utl::fmt::output& out, utl::fmt::field const&)
{
  out("foo");
}
// FMT_END_NAMESPACE

TEST(Format, FormatStdStringView) {
  CHECK_EQUAL("test"_sv, utl::format<10>("{}", utl::string_view("test")));
  CHECK_EQUAL("foo"_sv, utl::format<10>("{}", string_viewable()));
}

struct explicitly_convertible_to_std_string_view {
  explicit operator utl::string_view() const { return "foo"; }
};

constexpr void _format(explicitly_convertible_to_std_string_view const& arg, utl::fmt::output& out, 
  utl::fmt::field const& f)
{
  utl::maybe_unused(f);
  format_to(out, "'{}'", utl::string_view{arg});
}

TEST(Format, FormatExplicitlyConvertibleToStdStringView) {
  CHECK_EQUAL("'foo'"_sv,
            utl::format<10>("{}", explicitly_convertible_to_std_string_view()));
}
// #endif

// // std::is_constructible is broken in MSVC until version 2015.
// #if !FMT_MSC_VER || FMT_MSC_VER >= 1900
// struct explicitly_convertible_to_wstring_view {
//   explicit operator fmt::wstring_view() const { return L"foo"; }
// };

// TEST(FormatTest, FormatExplicitlyConvertibleToWStringView) {
//   CHECK_EQUAL(L"foo",
//             fmt::format(L"{}", explicitly_convertible_to_wstring_view()));
// }
// #endif

// namespace fake_qt {
// class QString {
//  public:
//   QString(const wchar_t* s) : s_(std::make_shared<std::wstring>(s)) {}
//   const wchar_t* utf16() const FMT_NOEXCEPT { return s_->data(); }
//   int size() const FMT_NOEXCEPT { return static_cast<int>(s_->size()); }

//  private:
//   std::shared_ptr<std::wstring> s_;
// };

// fmt::basic_string_view<wchar_t> to_string_view(const QString& s) FMT_NOEXCEPT {
//   return {s.utf16(), static_cast<size_t>(s.size())};
// }
// }  // namespace fake_qt

// TEST(FormatTest, FormatForeignStrings) {
//   using fake_qt::QString;
//   CHECK_EQUAL(fmt::format(QString(L"{}"), 42), L"42");
//   CHECK_EQUAL(fmt::format(QString(L"{}"), QString(L"42")), L"42");
// }

// FMT_BEGIN_NAMESPACE
// template <> struct formatter<Date> {
//   template <typename ParseContext>
//   FMT_CONSTEXPR auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
//     auto it = ctx.begin();
//     if (it != ctx.end() && *it == 'd') ++it;
//     return it;
//   }

//   auto format(const Date& d, format_context& ctx) -> decltype(ctx.out()) {
//     format_to(ctx.out(), "{}-{}-{}", d.year(), d.month(), d.day());
//     return ctx.out();
//   }
// };
// FMT_END_NAMESPACE

// TEST(Format, FormatCustom) {
//   Date date(2012, 12, 9);
//   EXPECT_THROW_MSG(fmt::format("{:s}", date), format_error,
//                    "unknown format specifier");
// }

class Answer {};

// FMT_BEGIN_NAMESPACE
// template <> struct formatter<Answer> : formatter<int> {
//   template <typename FormatContext>
//   auto format(Answer, FormatContext& ctx) -> decltype(ctx.out()) {
//     return formatter<int>::format(42, ctx);
//   }
// };
// FMT_END_NAMESPACE

constexpr void _format(Answer, utl::fmt::output& out, utl::fmt::field const& f)
{
  _format(42,out,f);
}

TEST(Format, CustomFormat) {
  CHECK_EQUAL("42"_sv, utl::format<10>("{0}", Answer()));
  CHECK_EQUAL("0042"_sv, utl::format<10>("{:04}", Answer()));
}

TEST(Format, CustomFormatTo) {
  //TODO: format_into should return an iterator from the container pointing to the next free position.
  //   then, there should be an overload of format_into that accepts an iterator.

  char buf[10] = {};
  auto end = &*utl::format_into(utl::span{buf, 10}, "{}", Answer());
  CHECK_EQUAL(end, buf + 2);
  auto view = utl::string_view{utl::ranges::begin(buf),end};
  CHECK_EQUAL("42"_sv, view);
}

// TEST(Format, WideFormatString) {
//   CHECK_EQUAL(L"42"_sv, format(L"{}", 42));
//   CHECK_EQUAL(L"4.2"_sv, format(L"{}", 4.2));
//   CHECK_EQUAL(L"abc"_sv, format(L"{}", L"abc"));
//   CHECK_EQUAL(L"z"_sv, format(L"{}", L'z'));
// }

// TEST(Format, FormatStringFromSpeedTest) {
//   CHECK_EQUAL("1.2340000000:0042:+3.13:str:0x3e8:X:%",
//             format("{0:0.10f}:{1:04}:{2:+g}:{3}:{4}:{5}:%", 1.234, 42, 3.13,
//                    "str", reinterpret_cast<void*>(1000), 'X'));
// }

TEST(Format, FormatExamples) {
  auto message = utl::format<20>("The answer is {}", 42);
  CHECK_EQUAL("The answer is 42"_sv, message);

  CHECK_EQUAL("42"_sv, utl::format<5>("{}", 42));
  CHECK_EQUAL("42"_sv, utl::format<5>(utl::string("{}"), 42));

  utl::array<char,20> out;
  auto end = utl::format_into(out, "The answer is {}.", 42);
  auto view = utl::string_view{utl::ranges::begin(out),end};
  CHECK_EQUAL("The answer is 42."_sv, view);
}

// TEST(Format, Examples) {
//   CHECK_EQUAL("First, thou shalt count to three",
//             format("First, thou shalt count to {0}", "three"));
//   CHECK_EQUAL("Bring me a shrubbery"_sv, format("Bring me a {}", "shrubbery"));
//   CHECK_EQUAL("From 1 to 3"_sv, format("From {} to {}", 1, 3));

//   char buffer[BUFFER_SIZE];
//   snprintf(buffer, BUFFER_SIZE, "%03.2f", -1.2);
//   CHECK_EQUAL(buffer, format("{:03.2f}", -1.2));

//   CHECK_EQUAL("a, b, c"_sv, format("{0}, {1}, {2}", 'a', 'b', 'c'));
//   CHECK_EQUAL("a, b, c"_sv, format("{}, {}, {}", 'a', 'b', 'c'));
//   CHECK_EQUAL("c, b, a"_sv, format("{2}, {1}, {0}", 'a', 'b', 'c'));
//   CHECK_EQUAL("abracadabra"_sv, format("{0}{1}{0}", "abra", "cad"));

//   CHECK_EQUAL("left aligned                  "_sv, format("{:<30}", "left aligned"));
//   CHECK_EQUAL("                 right aligned",
//             format("{:>30}", "right aligned"));
//   CHECK_EQUAL("           centered           "_sv, format("{:^30}", "centered"));
//   CHECK_EQUAL("***********centered***********"_sv, format("{:*^30}", "centered"));

//   CHECK_EQUAL("+3.140000; -3.140000"_sv, format("{:+f}; {:+f}", 3.14, -3.14));
//   CHECK_EQUAL(" 3.140000; -3.140000"_sv, format("{: f}; {: f}", 3.14, -3.14));
//   CHECK_EQUAL("3.140000; -3.140000"_sv, format("{:-f}; {:-f}", 3.14, -3.14));

//   CHECK_EQUAL("int: 42;  hex: 2a;  oct: 52",
//             format("int: {0:d};  hex: {0:x};  oct: {0:o}", 42));
//   CHECK_EQUAL("int: 42;  hex: 0x2a;  oct: 052",
//             format("int: {0:d};  hex: {0:#x};  oct: {0:#o}", 42));

//   CHECK_EQUAL("The answer is 42"_sv, format("The answer is {}", 42));
//   EXPECT_THROW_MSG(format("The answer is {:d}", "forty-two"), format_error,
//                    "invalid type specifier");

//   CHECK_EQUAL(L"Cyrillic letter \x42e"_sv, format(L"Cyrillic letter {}", L'\x42e'));

//   EXPECT_WRITE(
//       stdout, fmt::print("{}", std::numeric_limits<double>::infinity()), "inf");
// }

// TEST(FormatIntTest, Data) {
//   fmt::format_int format_int(42);
//   CHECK_EQUAL("42", std::string(format_int.data(), format_int.size()));
// }

// TEST(FormatIntTest, FormatInt) {
//   CHECK_EQUAL("42", fmt::format_int(42).str());
//   CHECK_EQUAL(2u, fmt::format_int(42).size());
//   CHECK_EQUAL("-42", fmt::format_int(-42).str());
//   CHECK_EQUAL(3u, fmt::format_int(-42).size());
//   CHECK_EQUAL("42", fmt::format_int(42ul).str());
//   CHECK_EQUAL("-42", fmt::format_int(-42l).str());
//   CHECK_EQUAL("42", fmt::format_int(42ull).str());
//   CHECK_EQUAL("-42", fmt::format_int(-42ll).str());
//   std::ostringstream os;
//   os << max_value<int64_t>();
//   CHECK_EQUAL(os.str(), fmt::format_int(max_value<int64_t>()).str());
// }

// TEST(FormatTest, Print) {
// #if FMT_USE_FCNTL
//   EXPECT_WRITE(stdout, fmt::print("Don't {}!", "panic"), "Don't panic!");
//   EXPECT_WRITE(stderr, fmt::print(stderr, "Don't {}!", "panic"),
//                "Don't panic!");
// #endif
//   // Check that the wide print overload compiles.
//   if (fmt::detail::const_check(false)) fmt::print(L"test");
// }

// TEST(FormatTest, Variadic) {
//   CHECK_EQUAL("abc1"_sv, format("{}c{}", "ab", 1));
//   CHECK_EQUAL(L"abc1"_sv, format(L"{}c{}", L"ab", 1));
// }

// TEST(FormatTest, Dynamic) {
//   typedef fmt::format_context ctx;
//   std::vector<fmt::basic_format_arg<ctx>> args;
//   args.emplace_back(fmt::detail::make_arg<ctx>(42));
//   args.emplace_back(fmt::detail::make_arg<ctx>("abc1"));
//   args.emplace_back(fmt::detail::make_arg<ctx>(1.5f));

//   std::string result = fmt::vformat(
//       "{} and {} and {}",
//       fmt::basic_format_args<ctx>(args.data(), static_cast<int>(args.size())));

//   CHECK_EQUAL("42 and abc1 and 1.5", result);
// }

// TEST(FormatTest, Bytes) {
//   auto s = fmt::format("{:10}", fmt::bytes("ёжик"));
//   CHECK_EQUAL("ёжик  ", s);
//   CHECK_EQUAL(10, s.size());
// }

// TEST(FormatTest, JoinArg) {
//   using fmt::join;
//   int v1[3] = {1, 2, 3};
//   std::vector<float> v2;
//   v2.push_back(1.2f);
//   v2.push_back(3.4f);
//   void* v3[2] = {&v1[0], &v1[1]};

//   CHECK_EQUAL("(1, 2, 3)"_sv, format("({})", join(v1, v1 + 3, ", ")));
//   CHECK_EQUAL("(1)"_sv, format("({})", join(v1, v1 + 1, ", ")));
//   CHECK_EQUAL("()"_sv, format("({})", join(v1, v1, ", ")));
//   CHECK_EQUAL("(001, 002, 003)"_sv, format("({:03})", join(v1, v1 + 3, ", ")));
//   CHECK_EQUAL("(+01.20, +03.40)",
//             format("({:+06.2f})", join(v2.begin(), v2.end(), ", ")));

//   CHECK_EQUAL(L"(1, 2, 3)"_sv, format(L"({})", join(v1, v1 + 3, L", ")));
//   CHECK_EQUAL("1, 2, 3"_sv, format("{0:{1}}", join(v1, v1 + 3, ", "), 1));

//   CHECK_EQUAL(format("{}, {}", v3[0], v3[1]),
//             format("{}", join(v3, v3 + 2, ", ")));

// #if !FMT_GCC_VERSION || FMT_GCC_VERSION >= 405
//   CHECK_EQUAL("(1, 2, 3)"_sv, format("({})", join(v1, ", ")));
//   CHECK_EQUAL("(+01.20, +03.40)"_sv, format("({:+06.2f})", join(v2, ", ")));
// #endif
// }

// template <typename T> std::string str(const T& value) {
//   return fmt::format("{}", value);
// }

// TEST(StrTest, Convert) {
//   CHECK_EQUAL("42", str(42));
//   std::string s = str(Date(2012, 12, 9));
//   CHECK_EQUAL("2012-12-9", s);
// }

// std::string vformat_message(int id, const char* format, fmt::format_args args) {
//   fmt::memory_buffer buffer;
//   format_to(buffer, "[{}] ", id);
//   vformat_to(buffer, format, args);
//   return to_string(buffer);
// }

// template <typename... Args>
// std::string format_message(int id, const char* format, const Args&... args) {
//   auto va = fmt::make_format_args(args...);
//   return vformat_message(id, format, va);
// }

// TEST(FormatTest, FormatMessageExample) {
//   CHECK_EQUAL("[42] something happened",
//             format_message(42, "{} happened", "something"));
// }

// template <typename... Args>
// void print_error(const char* file, int line, const char* format,
//                  const Args&... args) {
//   fmt::print("{}: {}: ", file, line);
//   fmt::print(format, args...);
// }

// TEST(FormatTest, UnpackedArgs) {
//   CHECK_EQUAL("0123456789abcdefg",
//             fmt::format("{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}", 0, 1, 2, 3, 4, 5,
//                         6, 7, 8, 9, 'a', 'b', 'c', 'd', 'e', 'f', 'g'));
// }

// struct string_like {};
// fmt::string_view to_string_view(string_like) { return "foo"; }

// constexpr char with_null[3] = {'{', '}', '\0'};
// constexpr char no_null[2] = {'{', '}'};

// TEST(FormatTest, CompileTimeString) {
//   CHECK_EQUAL("42", fmt::format(FMT_STRING("{}"), 42));
//   CHECK_EQUAL(L"42", fmt::format(FMT_STRING(L"{}"), 42));
//   CHECK_EQUAL("foo", fmt::format(FMT_STRING("{}"), string_like()));
//   (void)with_null;
//   (void)no_null;
// #if __cplusplus >= 201703L
//   CHECK_EQUAL("42", fmt::format(FMT_STRING(with_null), 42));
//   CHECK_EQUAL("42", fmt::format(FMT_STRING(no_null), 42));
// #endif
// #if defined(FMT_USE_STRING_VIEW) && __cplusplus >= 201703L
//   CHECK_EQUAL("42", fmt::format(FMT_STRING(std::string_view("{}")), 42));
//   CHECK_EQUAL(L"42", fmt::format(FMT_STRING(std::wstring_view(L"{}")), 42));
// #endif
// }

// TEST(FormatTest, CustomFormatCompileTimeString) {
//   CHECK_EQUAL("42", fmt::format(FMT_STRING("{}"), Answer()));
//   Answer answer;
//   CHECK_EQUAL("42", fmt::format(FMT_STRING("{}"), answer));
//   char buf[10] = {};
//   fmt::format_to(buf, FMT_STRING("{}"), answer);
//   const Answer const_answer = Answer();
//   CHECK_EQUAL("42", fmt::format(FMT_STRING("{}"), const_answer));
// }

// #if FMT_USE_USER_DEFINED_LITERALS
// // Passing user-defined literals directly to CHECK_EQUAL causes problems
// // with macro argument stringification (#) on some versions of GCC.
// // Workaround: Assing the UDL result to a variable before the macro.

// using namespace fmt::literals;

// TEST(LiteralsTest, Format) {
//   auto udl_format = "{}c{}"_format("ab", 1);
//   CHECK_EQUAL(format("{}c{}", "ab", 1), udl_format);
//   auto udl_format_w = L"{}c{}"_format(L"ab", 1);
//   CHECK_EQUAL(format(L"{}c{}", L"ab", 1), udl_format_w);
// }

// TEST(LiteralsTest, NamedArg) {
//   auto udl_a = format("{first}{second}{first}{third}", "first"_a = "abra",
//                       "second"_a = "cad", "third"_a = 99);
//   CHECK_EQUAL(format("{first}{second}{first}{third}", fmt::arg("first", "abra"),
//                    fmt::arg("second", "cad"), fmt::arg("third", 99)),
//             udl_a);
//   auto udl_a_w = format(L"{first}{second}{first}{third}", L"first"_a = L"abra",
//                         L"second"_a = L"cad", L"third"_a = 99);
//   CHECK_EQUAL(
//       format(L"{first}{second}{first}{third}", fmt::arg(L"first", L"abra"),
//              fmt::arg(L"second", L"cad"), fmt::arg(L"third", 99)),
//       udl_a_w);
// }

// TEST(FormatTest, UdlTemplate) {
//   CHECK_EQUAL("foo", "foo"_format());
//   CHECK_EQUAL("        42", "{0:10}"_format(42));
// }

// TEST(FormatTest, UdlPassUserDefinedObjectAsLvalue) {
//   Date date(2015, 10, 21);
//   CHECK_EQUAL("2015-10-21", "{}"_format(date));
// }
// #endif  // FMT_USE_USER_DEFINED_LITERALS

// enum TestEnum { A };

// TEST(FormatTest, Enum) { CHECK_EQUAL("0", fmt::format("{}", A)); }

// TEST(FormatTest, FormatterNotSpecialized) {
//   static_assert(
//       !fmt::has_formatter<fmt::formatter<TestEnum>, fmt::format_context>::value,
//       "");
// }

// #if FMT_HAS_FEATURE(cxx_strong_enums)
// enum big_enum : unsigned long long { big_enum_value = 5000000000ULL };

// TEST(FormatTest, StrongEnum) {
//   CHECK_EQUAL("5000000000", fmt::format("{}", big_enum_value));
// }
// #endif

// using buffer_iterator = fmt::format_context::iterator;

// class mock_arg_formatter
//     : public fmt::detail::arg_formatter_base<buffer_iterator, char> {
//  private:
// #if FMT_USE_INT128
//   MOCK_METHOD1(call, void(__int128_t value));
// #else
//   MOCK_METHOD1(call, void(long long value));
// #endif

//  public:
//   using base = fmt::detail::arg_formatter_base<buffer_iterator, char>;

//   mock_arg_formatter(fmt::format_context& ctx, fmt::format_parse_context*,
//                      fmt::format_specs* s = nullptr, const char* = nullptr)
//       : base(ctx.out(), s, ctx.locale()) {
//     EXPECT_CALL(*this, call(42));
//   }

//   template <typename T>
//   typename std::enable_if<fmt::detail::is_integral<T>::value, iterator>::type
//   operator()(T value) {
//     call(value);
//     return base::operator()(value);
//   }

//   template <typename T>
//   typename std::enable_if<!fmt::detail::is_integral<T>::value, iterator>::type
//   operator()(T value) {
//     return base::operator()(value);
//   }

//   iterator operator()(fmt::basic_format_arg<fmt::format_context>::handle) {
//     return base::operator()(fmt::monostate());
//   }
// };

// static void custom_vformat(fmt::string_view format_str, fmt::format_args args) {
//   fmt::memory_buffer buffer;
//   fmt::detail::buffer<char>& base = buffer;
//   fmt::vformat_to<mock_arg_formatter>(std::back_inserter(base), format_str,
//                                       args);
// }

// template <typename... Args>
// void custom_format(const char* format_str, const Args&... args) {
//   auto va = fmt::make_format_args(args...);
//   return custom_vformat(format_str, va);
// }

// TEST(FormatTest, CustomArgFormatter) { custom_format("{}", 42); }

// TEST(FormatTest, NonNullTerminatedFormatString) {
//   CHECK_EQUAL("42"_sv, format(string_view("{}foo", 2), 42));
// }

// struct variant {
//   enum { INT, STRING } type;
//   explicit variant(int) : type(INT) {}
//   explicit variant(const char*) : type(STRING) {}
// };

// FMT_BEGIN_NAMESPACE
// template <> struct formatter<variant> : dynamic_formatter<> {
//   auto format(variant value, format_context& ctx) -> decltype(ctx.out()) {
//     if (value.type == variant::INT) return dynamic_formatter<>::format(42, ctx);
//     return dynamic_formatter<>::format("foo", ctx);
//   }
// };
// FMT_END_NAMESPACE

// TEST(FormatTest, DynamicFormatter) {
//   auto num = variant(42);
//   auto str = variant("foo");
//   CHECK_EQUAL("42"_sv, format("{:d}", num));
//   CHECK_EQUAL("foo"_sv, format("{:s}", str));
//   CHECK_EQUAL(" 42 foo "_sv, format("{:{}} {:{}}", num, 3, str, 4));
//   EXPECT_THROW_MSG(format("{0:{}}", num), format_error,
//                    "cannot switch from manual to automatic argument indexing");
//   EXPECT_THROW_MSG(format("{:{0}}", num), format_error,
//                    "cannot switch from automatic to manual argument indexing");
// #if FMT_DEPRECATED_NUMERIC_ALIGN
//   EXPECT_THROW_MSG(format("{:=}", str), format_error,
//                    "format specifier requires numeric argument");
// #endif
//   EXPECT_THROW_MSG(format("{:+}", str), format_error,
//                    "format specifier requires numeric argument");
//   EXPECT_THROW_MSG(format("{:-}", str), format_error,
//                    "format specifier requires numeric argument");
//   EXPECT_THROW_MSG(format("{: }", str), format_error,
//                    "format specifier requires numeric argument");
//   EXPECT_THROW_MSG(format("{:#}", str), format_error,
//                    "format specifier requires numeric argument");
//   EXPECT_THROW_MSG(format("{:0}", str), format_error,
//                    "format specifier requires numeric argument");
//   EXPECT_THROW_MSG(format("{:.2}", num), format_error,
//                    "precision not allowed for this argument type");
// }

// namespace adl_test {
// namespace fmt {
// namespace detail {
// struct foo {};
// template <typename, typename OutputIt> void write(OutputIt, foo) = delete;
// }  // namespace detail
// }  // namespace fmt
// }  // namespace adl_test

// FMT_BEGIN_NAMESPACE
// template <>
// struct formatter<adl_test::fmt::detail::foo> : formatter<std::string> {
//   template <typename FormatContext>
//   auto format(adl_test::fmt::detail::foo, FormatContext& ctx)
//       -> decltype(ctx.out()) {
//     return formatter<std::string>::format("foo", ctx);
//   }
// };
// FMT_END_NAMESPACE

// TEST(FormatTest, ToString) {
//   CHECK_EQUAL("42", fmt::to_string(42));
//   CHECK_EQUAL("0x1234", fmt::to_string(reinterpret_cast<void*>(0x1234)));
//   CHECK_EQUAL("foo", fmt::to_string(adl_test::fmt::detail::foo()));
// }

// TEST(FormatTest, ToWString) { CHECK_EQUAL(L"42", fmt::to_wstring(42)); }

// TEST(FormatTest, OutputIterators) {
//   std::list<char> out;
//   fmt::format_to(std::back_inserter(out), "{}", 42);
//   CHECK_EQUAL("42", std::string(out.begin(), out.end()));
//   std::stringstream s;
//   fmt::format_to(std::ostream_iterator<char>(s), "{}", 42);
//   CHECK_EQUAL("42", s.str());
// }

// TEST(FormatTest, FormattedSize) {
//   CHECK_EQUAL(2u, fmt::formatted_size("{}", 42));
// }

// TEST(FormatTest, FormatToN) {
//   char buffer[4];
//   buffer[3] = 'x';
//   auto result = fmt::format_to_n(buffer, 3, "{}", 12345);
//   CHECK_EQUAL(5u, result.size);
//   CHECK_EQUAL(buffer + 3, result.out);
//   CHECK_EQUAL("123x", fmt::string_view(buffer, 4));
//   result = fmt::format_to_n(buffer, 3, "{:s}", "foobar");
//   CHECK_EQUAL(6u, result.size);
//   CHECK_EQUAL(buffer + 3, result.out);
//   CHECK_EQUAL("foox", fmt::string_view(buffer, 4));
//   buffer[0] = 'x';
//   buffer[1] = 'x';
//   buffer[2] = 'x';
//   result = fmt::format_to_n(buffer, 3, "{}", 'A');
//   CHECK_EQUAL(1u, result.size);
//   CHECK_EQUAL(buffer + 1, result.out);
//   CHECK_EQUAL("Axxx", fmt::string_view(buffer, 4));
//   result = fmt::format_to_n(buffer, 3, "{}{} ", 'B', 'C');
//   CHECK_EQUAL(3u, result.size);
//   CHECK_EQUAL(buffer + 3, result.out);
//   CHECK_EQUAL("BC x", fmt::string_view(buffer, 4));
// }

// TEST(FormatTest, WideFormatToN) {
//   wchar_t buffer[4];
//   buffer[3] = L'x';
//   auto result = fmt::format_to_n(buffer, 3, L"{}", 12345);
//   CHECK_EQUAL(5u, result.size);
//   CHECK_EQUAL(buffer + 3, result.out);
//   CHECK_EQUAL(L"123x", fmt::wstring_view(buffer, 4));
//   buffer[0] = L'x';
//   buffer[1] = L'x';
//   buffer[2] = L'x';
//   result = fmt::format_to_n(buffer, 3, L"{}", L'A');
//   CHECK_EQUAL(1u, result.size);
//   CHECK_EQUAL(buffer + 1, result.out);
//   CHECK_EQUAL(L"Axxx", fmt::wstring_view(buffer, 4));
//   result = fmt::format_to_n(buffer, 3, L"{}{} ", L'B', L'C');
//   CHECK_EQUAL(3u, result.size);
//   CHECK_EQUAL(buffer + 3, result.out);
//   CHECK_EQUAL(L"BC x", fmt::wstring_view(buffer, 4));
// }

// struct test_output_iterator {
//   char* data;

//   using iterator_category = std::output_iterator_tag;
//   using value_type = void;
//   using difference_type = void;
//   using pointer = void;
//   using reference = void;

//   test_output_iterator& operator++() {
//     ++data;
//     return *this;
//   }
//   test_output_iterator operator++(int) {
//     auto tmp = *this;
//     ++data;
//     return tmp;
//   }
//   char& operator*() { return *data; }
// };

// TEST(FormatTest, FormatToNOutputIterator) {
//   char buf[10] = {};
//   fmt::format_to_n(test_output_iterator{buf}, 10, "{}", 42);
//   EXPECT_STREQ(buf, "42");
// }

// #if FMT_USE_CONSTEXPR
// struct test_arg_id_handler {
//   enum result { NONE, EMPTY, INDEX, NAME, ERROR };
//   result res = NONE;
//   int index = 0;
//   string_view name;

//   FMT_CONSTEXPR void operator()() { res = EMPTY; }

//   FMT_CONSTEXPR void operator()(int i) {
//     res = INDEX;
//     index = i;
//   }

//   FMT_CONSTEXPR void operator()(string_view n) {
//     res = NAME;
//     name = n;
//   }

//   FMT_CONSTEXPR void on_error(const char*) { res = ERROR; }
// };

// template <size_t N>
// FMT_CONSTEXPR test_arg_id_handler parse_arg_id(const char (&s)[N]) {
//   test_arg_id_handler h;
//   fmt::detail::parse_arg_id(s, s + N, h);
//   return h;
// }

// TEST(FormatTest, ConstexprParseArgID) {
//   static_assert(parse_arg_id(":").res == test_arg_id_handler::EMPTY, "");
//   static_assert(parse_arg_id("}").res == test_arg_id_handler::EMPTY, "");
//   static_assert(parse_arg_id("42:").res == test_arg_id_handler::INDEX, "");
//   static_assert(parse_arg_id("42:").index == 42, "");
//   static_assert(parse_arg_id("foo:").res == test_arg_id_handler::NAME, "");
//   static_assert(parse_arg_id("foo:").name.size() == 3, "");
//   static_assert(parse_arg_id("!").res == test_arg_id_handler::ERROR, "");
// }

// struct test_format_specs_handler {
//   enum Result { NONE, PLUS, MINUS, SPACE, HASH, ZERO, ERROR };
//   Result res = NONE;

//   fmt::align_t align = fmt::align::none;
//   char fill = 0;
//   int width = 0;
//   fmt::detail::arg_ref<char> width_ref;
//   int precision = 0;
//   fmt::detail::arg_ref<char> precision_ref;
//   char type = 0;

//   // Workaround for MSVC2017 bug that results in "expression did not evaluate
//   // to a constant" with compiler-generated copy ctor.
//   FMT_CONSTEXPR test_format_specs_handler() {}
//   FMT_CONSTEXPR test_format_specs_handler(
//       const test_format_specs_handler& other)
//       : res(other.res),
//         align(other.align),
//         fill(other.fill),
//         width(other.width),
//         width_ref(other.width_ref),
//         precision(other.precision),
//         precision_ref(other.precision_ref),
//         type(other.type) {}

//   FMT_CONSTEXPR void on_align(fmt::align_t a) { align = a; }
//   FMT_CONSTEXPR void on_fill(fmt::string_view f) { fill = f[0]; }
//   FMT_CONSTEXPR void on_plus() { res = PLUS; }
//   FMT_CONSTEXPR void on_minus() { res = MINUS; }
//   FMT_CONSTEXPR void on_space() { res = SPACE; }
//   FMT_CONSTEXPR void on_hash() { res = HASH; }
//   FMT_CONSTEXPR void on_zero() { res = ZERO; }

//   FMT_CONSTEXPR void on_width(int w) { width = w; }
//   FMT_CONSTEXPR void on_dynamic_width(fmt::detail::auto_id) {}
//   FMT_CONSTEXPR void on_dynamic_width(int index) { width_ref = index; }
//   FMT_CONSTEXPR void on_dynamic_width(string_view) {}

//   FMT_CONSTEXPR void on_precision(int p) { precision = p; }
//   FMT_CONSTEXPR void on_dynamic_precision(fmt::detail::auto_id) {}
//   FMT_CONSTEXPR void on_dynamic_precision(int index) { precision_ref = index; }
//   FMT_CONSTEXPR void on_dynamic_precision(string_view) {}

//   FMT_CONSTEXPR void end_precision() {}
//   FMT_CONSTEXPR void on_type(char t) { type = t; }
//   FMT_CONSTEXPR void on_error(const char*) { res = ERROR; }
// };

// template <size_t N>
// FMT_CONSTEXPR test_format_specs_handler parse_test_specs(const char (&s)[N]) {
//   test_format_specs_handler h;
//   fmt::detail::parse_format_specs(s, s + N, h);
//   return h;
// }

// TEST(FormatTest, ConstexprParseFormatSpecs) {
//   typedef test_format_specs_handler handler;
//   static_assert(parse_test_specs("<").align == fmt::align::left, "");
//   static_assert(parse_test_specs("*^").fill == '*', "");
//   static_assert(parse_test_specs("+").res == handler::PLUS, "");
//   static_assert(parse_test_specs("-").res == handler::MINUS, "");
//   static_assert(parse_test_specs(" ").res == handler::SPACE, "");
//   static_assert(parse_test_specs("#").res == handler::HASH, "");
//   static_assert(parse_test_specs("0").res == handler::ZERO, "");
//   static_assert(parse_test_specs("42").width == 42, "");
//   static_assert(parse_test_specs("{42}").width_ref.val.index == 42, "");
//   static_assert(parse_test_specs(".42").precision == 42, "");
//   static_assert(parse_test_specs(".{42}").precision_ref.val.index == 42, "");
//   static_assert(parse_test_specs("d").type == 'd', "");
//   static_assert(parse_test_specs("{<").res == handler::ERROR, "");
// }

// struct test_parse_context {
//   typedef char char_type;

//   FMT_CONSTEXPR int next_arg_id() { return 11; }
//   template <typename Id> FMT_CONSTEXPR void check_arg_id(Id) {}

//   FMT_CONSTEXPR const char* begin() { return nullptr; }
//   FMT_CONSTEXPR const char* end() { return nullptr; }

//   void on_error(const char*) {}
// };

// struct test_context {
//   using char_type = char;
//   using format_arg = fmt::basic_format_arg<test_context>;
//   using parse_context_type = fmt::format_parse_context;

//   template <typename T> struct formatter_type {
//     typedef fmt::formatter<T, char_type> type;
//   };

//   template <typename Id>
//   FMT_CONSTEXPR fmt::basic_format_arg<test_context> arg(Id id) {
//     return fmt::detail::make_arg<test_context>(id);
//   }

//   void on_error(const char*) {}

//   FMT_CONSTEXPR test_context error_handler() { return *this; }
// };

// template <size_t N>
// FMT_CONSTEXPR fmt::format_specs parse_specs(const char (&s)[N]) {
//   auto specs = fmt::format_specs();
//   auto parse_ctx = test_parse_context();
//   auto ctx = test_context();
//   fmt::detail::specs_handler<test_parse_context, test_context> h(
//       specs, parse_ctx, ctx);
//   parse_format_specs(s, s + N, h);
//   return specs;
// }

// TEST(FormatTest, ConstexprSpecsHandler) {
//   static_assert(parse_specs("<").align == fmt::align::left, "");
//   static_assert(parse_specs("*^").fill[0] == '*', "");
//   static_assert(parse_specs("+").sign == fmt::sign::plus, "");
//   static_assert(parse_specs("-").sign == fmt::sign::minus, "");
//   static_assert(parse_specs(" ").sign == fmt::sign::space, "");
//   static_assert(parse_specs("#").alt, "");
//   static_assert(parse_specs("0").align == fmt::align::numeric, "");
//   static_assert(parse_specs("42").width == 42, "");
//   static_assert(parse_specs("{}").width == 11, "");
//   static_assert(parse_specs("{22}").width == 22, "");
//   static_assert(parse_specs(".42").precision == 42, "");
//   static_assert(parse_specs(".{}").precision == 11, "");
//   static_assert(parse_specs(".{22}").precision == 22, "");
//   static_assert(parse_specs("d").type == 'd', "");
// }

// template <size_t N>
// FMT_CONSTEXPR fmt::detail::dynamic_format_specs<char> parse_dynamic_specs(
//     const char (&s)[N]) {
//   fmt::detail::dynamic_format_specs<char> specs;
//   test_parse_context ctx{};
//   fmt::detail::dynamic_specs_handler<test_parse_context> h(specs, ctx);
//   parse_format_specs(s, s + N, h);
//   return specs;
// }

// TEST(FormatTest, ConstexprDynamicSpecsHandler) {
//   static_assert(parse_dynamic_specs("<").align == fmt::align::left, "");
//   static_assert(parse_dynamic_specs("*^").fill[0] == '*', "");
//   static_assert(parse_dynamic_specs("+").sign == fmt::sign::plus, "");
//   static_assert(parse_dynamic_specs("-").sign == fmt::sign::minus, "");
//   static_assert(parse_dynamic_specs(" ").sign == fmt::sign::space, "");
//   static_assert(parse_dynamic_specs("#").alt, "");
//   static_assert(parse_dynamic_specs("0").align == fmt::align::numeric, "");
//   static_assert(parse_dynamic_specs("42").width == 42, "");
//   static_assert(parse_dynamic_specs("{}").width_ref.val.index == 11, "");
//   static_assert(parse_dynamic_specs("{42}").width_ref.val.index == 42, "");
//   static_assert(parse_dynamic_specs(".42").precision == 42, "");
//   static_assert(parse_dynamic_specs(".{}").precision_ref.val.index == 11, "");
//   static_assert(parse_dynamic_specs(".{42}").precision_ref.val.index == 42, "");
//   static_assert(parse_dynamic_specs("d").type == 'd', "");
// }

// template <size_t N>
// FMT_CONSTEXPR test_format_specs_handler check_specs(const char (&s)[N]) {
//   fmt::detail::specs_checker<test_format_specs_handler> checker(
//       test_format_specs_handler(), fmt::detail::type::double_type);
//   parse_format_specs(s, s + N, checker);
//   return checker;
// }

// TEST(FormatTest, ConstexprSpecsChecker) {
//   typedef test_format_specs_handler handler;
//   static_assert(check_specs("<").align == fmt::align::left, "");
//   static_assert(check_specs("*^").fill == '*', "");
//   static_assert(check_specs("+").res == handler::PLUS, "");
//   static_assert(check_specs("-").res == handler::MINUS, "");
//   static_assert(check_specs(" ").res == handler::SPACE, "");
//   static_assert(check_specs("#").res == handler::HASH, "");
//   static_assert(check_specs("0").res == handler::ZERO, "");
//   static_assert(check_specs("42").width == 42, "");
//   static_assert(check_specs("{42}").width_ref.val.index == 42, "");
//   static_assert(check_specs(".42").precision == 42, "");
//   static_assert(check_specs(".{42}").precision_ref.val.index == 42, "");
//   static_assert(check_specs("d").type == 'd', "");
//   static_assert(check_specs("{<").res == handler::ERROR, "");
// }

// struct test_format_string_handler {
//   FMT_CONSTEXPR void on_text(const char*, const char*) {}

//   FMT_CONSTEXPR int on_arg_id() { return 0; }

//   template <typename T> FMT_CONSTEXPR int on_arg_id(T) { return 0; }

//   FMT_CONSTEXPR void on_replacement_field(int, const char*) {}

//   FMT_CONSTEXPR const char* on_format_specs(int, const char* begin,
//                                             const char*) {
//     return begin;
//   }

//   FMT_CONSTEXPR void on_error(const char*) { error = true; }

//   bool error = false;
// };

// template <size_t N> FMT_CONSTEXPR bool parse_string(const char (&s)[N]) {
//   test_format_string_handler h;
//   fmt::detail::parse_format_string<true>(fmt::string_view(s, N - 1), h);
//   return !h.error;
// }

// TEST(FormatTest, ConstexprParseFormatString) {
//   static_assert(parse_string("foo"), "");
//   static_assert(!parse_string("}"), "");
//   static_assert(parse_string("{}"), "");
//   static_assert(parse_string("{42}"), "");
//   static_assert(parse_string("{foo}"), "");
//   static_assert(parse_string("{:}"), "");
// }

// struct test_error_handler {
//   const char*& error;

//   FMT_CONSTEXPR test_error_handler(const char*& err) : error(err) {}

//   FMT_CONSTEXPR test_error_handler(const test_error_handler& other)
//       : error(other.error) {}

//   FMT_CONSTEXPR void on_error(const char* message) {
//     if (!error) error = message;
//   }
// };

// FMT_CONSTEXPR size_t len(const char* s) {
//   size_t len = 0;
//   while (*s++) ++len;
//   return len;
// }

// FMT_CONSTEXPR bool equal(const char* s1, const char* s2) {
//   if (!s1 || !s2) return s1 == s2;
//   while (*s1 && *s1 == *s2) {
//     ++s1;
//     ++s2;
//   }
//   return *s1 == *s2;
// }

// template <typename... Args>
// FMT_CONSTEXPR bool test_error(const char* fmt, const char* expected_error) {
//   const char* actual_error = nullptr;
//   string_view s(fmt, len(fmt));
//   fmt::detail::format_string_checker<char, test_error_handler, Args...> checker(
//       s, test_error_handler(actual_error));
//   fmt::detail::parse_format_string<true>(s, checker);
//   return equal(actual_error, expected_error);
// }

// #  define EXPECT_ERROR_NOARGS(fmt, error) \
//     static_assert(test_error(fmt, error), "")
// #  define EXPECT_ERROR(fmt, error, ...) \
//     static_assert(test_error<__VA_ARGS__>(fmt, error), "")

// TEST(FormatTest, FormatStringErrors) {
//   EXPECT_ERROR_NOARGS("foo", nullptr);
//   EXPECT_ERROR_NOARGS("}", "unmatched '}' in format string");
//   EXPECT_ERROR("{0:s", "unknown format specifier", Date);
// #  if !FMT_MSC_VER || FMT_MSC_VER >= 1916
//   // This causes an detail compiler error in MSVC2017.
//   EXPECT_ERROR("{:{<}", "invalid fill character '{'", int);
//   EXPECT_ERROR("{:10000000000}", "number is too big", int);
//   EXPECT_ERROR("{:.10000000000}", "number is too big", int);
//   EXPECT_ERROR_NOARGS("{:x}", "argument not found");
// #    if FMT_DEPRECATED_NUMERIC_ALIGN
//   EXPECT_ERROR("{0:=5", "unknown format specifier", int);
//   EXPECT_ERROR("{:=}", "format specifier requires numeric argument",
//                const char*);
// #    endif
//   EXPECT_ERROR("{:+}", "format specifier requires numeric argument",
//                const char*);
//   EXPECT_ERROR("{:-}", "format specifier requires numeric argument",
//                const char*);
//   EXPECT_ERROR("{:#}", "format specifier requires numeric argument",
//                const char*);
//   EXPECT_ERROR("{: }", "format specifier requires numeric argument",
//                const char*);
//   EXPECT_ERROR("{:0}", "format specifier requires numeric argument",
//                const char*);
//   EXPECT_ERROR("{:+}", "format specifier requires signed argument", unsigned);
//   EXPECT_ERROR("{:-}", "format specifier requires signed argument", unsigned);
//   EXPECT_ERROR("{: }", "format specifier requires signed argument", unsigned);
//   EXPECT_ERROR("{:{}}", "argument not found", int);
//   EXPECT_ERROR("{:.{}}", "argument not found", double);
//   EXPECT_ERROR("{:.2}", "precision not allowed for this argument type", int);
//   EXPECT_ERROR("{:s}", "invalid type specifier", int);
//   EXPECT_ERROR("{:s}", "invalid type specifier", bool);
//   EXPECT_ERROR("{:s}", "invalid type specifier", char);
//   EXPECT_ERROR("{:+}", "invalid format specifier for char", char);
//   EXPECT_ERROR("{:s}", "invalid type specifier", double);
//   EXPECT_ERROR("{:d}", "invalid type specifier", const char*);
//   EXPECT_ERROR("{:d}", "invalid type specifier", std::string);
//   EXPECT_ERROR("{:s}", "invalid type specifier", void*);
// #  else
//   fmt::print("warning: constexpr is broken in this version of MSVC\n");
// #  endif
//   EXPECT_ERROR("{foo", "compile-time checks don't support named arguments",
//                int);
//   EXPECT_ERROR_NOARGS("{10000000000}", "number is too big");
//   EXPECT_ERROR_NOARGS("{0x}", "invalid format string");
//   EXPECT_ERROR_NOARGS("{-}", "invalid format string");
//   EXPECT_ERROR("{:{0x}}", "invalid format string", int);
//   EXPECT_ERROR("{:{-}}", "invalid format string", int);
//   EXPECT_ERROR("{:.{0x}}", "invalid format string", int);
//   EXPECT_ERROR("{:.{-}}", "invalid format string", int);
//   EXPECT_ERROR("{:.x}", "missing precision specifier", int);
//   EXPECT_ERROR_NOARGS("{}", "argument not found");
//   EXPECT_ERROR("{1}", "argument not found", int);
//   EXPECT_ERROR("{1}{}",
//                "cannot switch from manual to automatic argument indexing", int,
//                int);
//   EXPECT_ERROR("{}{1}",
//                "cannot switch from automatic to manual argument indexing", int,
//                int);
// }

// TEST(FormatTest, VFormatTo) {
//   typedef fmt::format_context context;
//   fmt::basic_format_arg<context> arg = fmt::detail::make_arg<context>(42);
//   fmt::basic_format_args<context> args(&arg, 1);
//   std::string s;
//   fmt::vformat_to(std::back_inserter(s), "{}", args);
//   CHECK_EQUAL("42", s);
//   s.clear();
//   fmt::vformat_to(std::back_inserter(s), FMT_STRING("{}"), args);
//   CHECK_EQUAL("42", s);

//   typedef fmt::wformat_context wcontext;
//   fmt::basic_format_arg<wcontext> warg = fmt::detail::make_arg<wcontext>(42);
//   fmt::basic_format_args<wcontext> wargs(&warg, 1);
//   std::wstring w;
//   fmt::vformat_to(std::back_inserter(w), L"{}", wargs);
//   CHECK_EQUAL(L"42", w);
//   w.clear();
//   fmt::vformat_to(std::back_inserter(w), FMT_STRING(L"{}"), wargs);
//   CHECK_EQUAL(L"42", w);
// }

// template <typename T> static std::string FmtToString(const T& t) {
//   return fmt::format(FMT_STRING("{}"), t);
// }

// TEST(FormatTest, FmtStringInTemplate) {
//   CHECK_EQUAL(FmtToString(1), "1");
//   CHECK_EQUAL(FmtToString(0), "0");
// }

// #endif  // FMT_USE_CONSTEXPR

// TEST(FormatTest, EmphasisNonHeaderOnly) {
//   // Ensure this compiles even if FMT_HEADER_ONLY is not defined.
//   CHECK_EQUAL(fmt::format(fmt::emphasis::bold, "bold error"),
//             "\x1b[1mbold error\x1b[0m");
// }

// TEST(FormatTest, CharTraitsIsNotAmbiguous) {
//   // Test that we don't inject detail names into the std namespace.
//   using namespace std;
//   char_traits<char>::char_type c;
//   (void)c;
// #if __cplusplus >= 201103L
//   std::string s;
//   auto lval = begin(s);
//   (void)lval;
// #endif
// }

// struct mychar {
//   int value;
//   mychar() = default;

//   template <typename T> mychar(T val) : value(static_cast<int>(val)) {}

//   operator int() const { return value; }
// };

// FMT_BEGIN_NAMESPACE
// template <> struct is_char<mychar> : std::true_type {};
// FMT_END_NAMESPACE

// TEST(FormatTest, FormatCustomChar) {
//   const mychar format[] = {'{', '}', 0};
//   auto result = fmt::format(format, mychar('x'));
//   CHECK_EQUAL(result.size(), 1);
//   CHECK_EQUAL(result[0], mychar('x'));
// }

// // Convert a char8_t string to std::string. Otherwise GTest will insist on
// // inserting `char8_t` NTBS into a `char` stream which is disabled by P1423.
// template <typename S> std::string from_u8str(const S& str) {
//   return std::string(str.begin(), str.end());
// }

// TEST(FormatTest, FormatUTF8Precision) {
//   using str_type = std::basic_string<fmt::detail::char8_type>;
//   str_type format(reinterpret_cast<const fmt::detail::char8_type*>(u8"{:.4}"));
//   str_type str(reinterpret_cast<const fmt::detail::char8_type*>(
//       u8"caf\u00e9s"));  // cafés
//   auto result = fmt::format(format, str);
//   CHECK_EQUAL(fmt::detail::count_code_points(result), 4);
//   CHECK_EQUAL(result.size(), 5);
//   CHECK_EQUAL(from_u8str(result), from_u8str(str.substr(0, 5)));
// }
