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
 * @file test_core.cc
 * @brief The tests of the small tools of core: FixedList and Utf8ToUtf16.
 */

#include "core/fixed_list.h"
#include "core/utf8.h"
#include "test.h"

// A FixedList must stay a constant of the program: a singleton with a list
// must not need a guard (see core/singleton.h). The line does not compile
// when the constructor is not constexpr.
constexpr core::FixedList<int, 4> kConstantList;

TEST(FixedListAddsUntilItIsFull) {
  core::FixedList<int, 3> list;
  CHECK(list.IsEmpty());
  CHECK(list.Add(1));
  CHECK(list.Add(2));
  CHECK(list.Add(3));
  CHECK(list.IsFull());
  CHECK(!list.Add(4));
  CHECK_EQ(list.Count(), 3);
  CHECK_EQ(list[2], 3);
  int sum = 0;
  for (int value : list) sum += value;
  CHECK_EQ(sum, 6);
  list.Clear();
  CHECK(list.IsEmpty());
}

TEST(FixedListFindsAValue) {
  core::FixedList<int, 4> list;
  list.Add(10);
  list.Add(20);
  const int* found = list.Find([](int value) { return value > 15; });
  CHECK(found != nullptr && *found == 20);
  CHECK(list.Find([](int value) { return value > 99; }) == nullptr);
  CHECK_EQ(kConstantList.Count(), 0);
}

TEST(FixedListRemovesValuesAndKeepsTheOrder) {
  core::FixedList<int, 6> list;
  for (int value = 1; value <= 6; value++) list.Add(value);
  CHECK_EQ(list.RemoveIf([](int value) { return value % 2 == 0; }), 3);
  CHECK_EQ(list.Count(), 3);
  CHECK_EQ(list[0], 1);
  CHECK_EQ(list[1], 3);
  CHECK_EQ(list[2], 5);
  CHECK_EQ(list.RemoveIf([](int value) { return value > 99; }), 0);
  CHECK_EQ(list.Count(), 3);
}

TEST(Utf8ToUtf16ReadsOneTwoAndThreeBytes) {
  c16 out[16];
  // "aé€": 1, 2 and 3 bytes.
  const c8 text[] = "a\xC3\xA9\xE2\x82\xAC";
  CHECK_EQ(core::Utf8ToUtf16(text, 6, out, 16), 3);
  CHECK_EQ(out[0], u'a');
  CHECK_EQ(out[1], 0xE9);
  CHECK_EQ(out[2], 0x20AC);
  CHECK_EQ(out[3], 0);
}

TEST(Utf8ToUtf16StopsAtTheCapacity) {
  c16 out[3];
  CHECK_EQ(core::Utf8ToUtf16("abcdef", 6, out, 3), 2);
  CHECK_EQ(out[2], 0);
  // A character that the text cuts is not written.
  CHECK_EQ(core::Utf8ToUtf16("a\xC3", 2, out, 3), 1);
}
