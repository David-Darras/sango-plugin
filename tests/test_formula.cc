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
 * @file test_formula.cc
 * @brief The tests of core::Formula: the programs that Sango Studio writes.
 */

#include "core/formula.h"
#include "test.h"

namespace {

/// Writes the program of a formula by hand.
struct Program {
  u8 bytes[core::Formula::kMaxSize + 8];
  u32 size = 0;

  Program& Constant(s32 value) {
    bytes[size++] = 0x01;
    for (int i = 0; i < 4; i++) {
      bytes[size++] = static_cast<u8>(static_cast<u32>(value) >> (8 * i));
    }
    return *this;
  }
  Program& Variable(u8 index) {
    bytes[size++] = 0x02;
    bytes[size++] = index;
    return *this;
  }
  Program& Op(u8 op) {
    bytes[size++] = op;
    return *this;
  }
};

constexpr u8 kAdd = 0x10;
constexpr u8 kSubtract = 0x11;
constexpr u8 kMultiply = 0x12;
constexpr u8 kDivide = 0x13;
constexpr u8 kEqual = 0x18;
constexpr u8 kChoose = 0x1B;

/// (2 * base + iv + ev / 4) * level / 100 + 5, with base, iv, ev and level in
/// the variables 0 to 3: the formula of the Attack of the game.
void MakeAttack(Program* p) {
  p->Constant(2).Variable(0).Op(kMultiply).Variable(1).Op(kAdd);
  p->Variable(2).Constant(4).Op(kDivide).Op(kAdd);
  p->Variable(3).Op(kMultiply).Constant(100).Op(kDivide);
  p->Constant(5).Op(kAdd);
}

s64 Attack(s32 base, s32 iv, s32 ev, s32 level) {
  return ((2 * base + iv + ev / 4) * level / 100 + 5);
}

} // namespace

TEST(AFormulaGivesTheStatOfTheGame) {
  Program program;
  MakeAttack(&program);
  core::Formula formula;
  CHECK(formula.Set(program.bytes, program.size));
  for (s32 base = 5; base <= 255; base += 25) {
    for (s32 level = 1; level <= 100; level += 9) {
      const s32 variables[4] = {base, 31, 252, level};
      CHECK_EQ(formula.Evaluate(variables, 4), Attack(base, 31, 252, level));
    }
  }
}

TEST(AFormulaCanChooseAValue) {
  // If the species is 292, then 1, else the level.
  Program program;
  program.Variable(4).Constant(292).Op(kEqual).Constant(1).Variable(3).Op(kChoose);
  core::Formula formula;
  CHECK(formula.Set(program.bytes, program.size));
  const s32 shedinja[5] = {0, 0, 0, 50, 292};
  const s32 other[5] = {0, 0, 0, 50, 25};
  CHECK_EQ(formula.Evaluate(shedinja, 5), 1);
  CHECK_EQ(formula.Evaluate(other, 5), 50);
}

TEST(ADivisionByZeroGivesZeroAndNothingOverflows) {
  Program program;
  program.Constant(7).Constant(0).Op(kDivide);
  core::Formula formula;
  CHECK(formula.Set(program.bytes, program.size));
  CHECK_EQ(formula.Evaluate(nullptr, 0), 0);

  Program big;
  big.Constant(2147483647).Constant(2147483647).Op(kMultiply).Constant(2147483647).Op(kMultiply);
  big.Constant(2147483647).Op(kMultiply).Constant(-1).Op(kDivide);
  CHECK(formula.Set(big.bytes, big.size));
  formula.Evaluate(nullptr, 0); // Wraps around: no crash.
}

TEST(AProgramThatIsNotCorrectIsRefused) {
  core::Formula formula;
  Program ok;
  ok.Constant(3);
  CHECK(formula.Set(ok.bytes, ok.size));

  Program empty;
  CHECK(!formula.Set(empty.bytes, 0));

  Program two;
  two.Constant(1).Constant(2); // Two numbers are left.
  CHECK(!formula.Set(two.bytes, two.size));

  Program missing;
  missing.Constant(1).Op(kAdd); // Not enough numbers.
  CHECK(!formula.Set(missing.bytes, missing.size));

  Program unknown;
  unknown.Op(0x77);
  CHECK(!formula.Set(unknown.bytes, unknown.size));

  Program variable;
  variable.Variable(core::Formula::kMaxVariables);
  CHECK(!formula.Set(variable.bytes, variable.size));

  Program cut;
  cut.Constant(5);
  CHECK(!formula.Set(cut.bytes, 3)); // The constant is cut.

  Program tall;
  for (u32 i = 0; i <= core::Formula::kStackSize; i++) tall.Constant(1);
  for (u32 i = 0; i < core::Formula::kStackSize; i++) tall.Op(kAdd);
  CHECK(!formula.Set(tall.bytes, tall.size)); // The stack is too big.

  // A refused program keeps the old one.
  const s32 none[1] = {0};
  CHECK_EQ(formula.Evaluate(none, 1), 3);
  formula.Clear();
  CHECK(!formula.IsSet());
}

TEST(TheSubtractionTakesTheTopNumberOff) {
  Program program;
  program.Constant(10).Constant(3).Op(kSubtract);
  core::Formula formula;
  CHECK(formula.Set(program.bytes, program.size));
  CHECK_EQ(formula.Evaluate(nullptr, 0), 7);
}

// The programs that Sango Studio writes for the formulas of the game. They are the same bytes as the test
// `the_programs_of_the_game_have_the_bytes_of_the_plugin_tests` of the studio (module/formula/src/expr.rs).
static const u8 kStudioStat[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x12, 0x02, 0x01, 0x10, 0x02,
                                 0x02, 0x01, 0x04, 0x00, 0x00, 0x00, 0x13, 0x10, 0x02, 0x03, 0x12, 0x01,
                                 0x64, 0x00, 0x00, 0x00, 0x13, 0x01, 0x05, 0x00, 0x00, 0x00, 0x10};
static const u8 kStudioHp[] = {0x02, 0x04, 0x01, 0x24, 0x01, 0x00, 0x00, 0x18, 0x01, 0x01, 0x00, 0x00, 0x00,
                               0x01, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x12, 0x02, 0x01, 0x10, 0x02, 0x02,
                               0x01, 0x04, 0x00, 0x00, 0x00, 0x13, 0x10, 0x02, 0x03, 0x12, 0x01, 0x64, 0x00,
                               0x00, 0x00, 0x13, 0x02, 0x03, 0x10, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x10, 0x1B};

TEST(TheProgramsOfTheStudioGiveTheStatsOfTheGame) {
  core::Formula stat;
  core::Formula hp;
  CHECK(stat.Set(kStudioStat, sizeof(kStudioStat)));
  CHECK(hp.Set(kStudioHp, sizeof(kStudioHp)));
  for (s32 base = 5; base <= 255; base += 10) {
    for (s32 level = 1; level <= 100; level++) {
      for (s32 ev = 0; ev <= 252; ev += 84) {
        const s32 variables[5] = {base, 31, ev, level, 25};
        CHECK_EQ(stat.Evaluate(variables, 5), (2 * base + 31 + ev / 4) * level / 100 + 5);
        CHECK_EQ(hp.Evaluate(variables, 5), (2 * base + 31 + ev / 4) * level / 100 + level + 10);
      }
    }
  }
  const s32 shedinja[5] = {1, 31, 0, 50, 292};
  CHECK_EQ(hp.Evaluate(shedinja, 5), 1);
}
