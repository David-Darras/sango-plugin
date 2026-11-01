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
 * @file main.cc
 * @brief Runs all the tests of tests/.
 */

#include "test.h"

namespace test {
Case*& First() {
  static Case* first = nullptr;
  return first;
}

int& Failures() {
  static int failures = 0;
  return failures;
}
} // namespace test

int main() {
  int count = 0;
  for (test::Case* test = test::First(); test != nullptr; test = test->next) {
    const int before = test::Failures();
    test->function();
    std::printf("%s %s\n", test::Failures() == before ? "ok  " : "FAIL",
                test->name);
    count++;
  }
  std::printf("\n%d tests, %d failed checks\n", count, test::Failures());
  return test::Failures() == 0 ? 0 : 1;
}
