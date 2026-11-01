/*
 * Copyright (C) 2026  David Darras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file test.h
 * @brief A small test framework for the tests on a computer.
 *
 * @code
 * TEST(AddsTwoNumbers) {
 *   CHECK_EQ(1 + 1, 2);
 * }
 * @endcode
 *
 * Run the tests with `make -C tests`.
 */

#pragma once

#include <cstdio>

namespace test {

/// A test: its name and its function.
struct Case {
  const char* name;
  void (*function)();
  Case* next;
};

/// The list of the tests, and the number of the failed checks.
Case*& First();
int& Failures();

/// Adds a test to the list. TEST() uses it.
struct Registration {
  Registration(Case* test) {
    test->next = First();
    First() = test;
  }
};

} // namespace test

/// Declares a test. The tests run in an order that is not defined.
#define TEST(name)                                                    \
  static void name();                                                 \
  static test::Case name##_case = {#name, name, nullptr};             \
  static test::Registration name##_registration(&name##_case);        \
  static void name()

/// Fails the test when `condition` is false.
#define CHECK(condition)                                              \
  do {                                                                \
    if (!(condition)) {                                               \
      std::printf("%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__,    \
                  #condition);                                        \
      test::Failures()++;                                             \
    }                                                                 \
  } while (0)

/// Fails the test when `a` and `b` are not equal. They are numbers or enums.
#define CHECK_EQ(a, b)                                                \
  do {                                                                \
    const long long check_a = static_cast<long long>(a);              \
    const long long check_b = static_cast<long long>(b);              \
    if (check_a != check_b) {                                         \
      std::printf("%s:%d: CHECK_EQ(%s, %s) failed: %lld != %lld\n",   \
                  __FILE__, __LINE__, #a, #b, check_a, check_b);      \
      test::Failures()++;                                             \
    }                                                                 \
  } while (0)
