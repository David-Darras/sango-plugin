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

#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "common.h"

namespace parser {
struct Stats {
  u8 hp = 0;
  u8 atk = 0;
  u8 def = 0;
  u8 spa = 0;
  u8 spd = 0;
  u8 spe = 0;

  // Constructeur par défaut (initialise à ce que tu veux)
  Stats(u8 default_val = 0)
    : hp(default_val), atk(default_val), def(default_val),
      spa(default_val), spd(default_val), spe(default_val) {
  }

  // Constructeur complet au cas où
  Stats(u8 h, u8 a, u8 d, u8 sa, u8 sd, u8 sp)
    : hp(h), atk(a), def(d), spa(sa), spd(sd), spe(sp) {
  }
};

struct PokemonNode {
  u16 species = 0;
  bool has_nickname = false;
  c16 nickname[13] = {0};
  u8 gender = 0;
  u16 item = 0;
  u8 ability = 0;
  bool is_shiny = false;
  u8 level = 100;
  u8 happiness = 255;
  u8 ball = 0;

  Stats evs{0, 0, 0, 0, 0, 0};
  Stats ivs{31, 31, 31, 31, 31, 31};

  u8 nature = 0;
  std::vector<u16> moves = {0, 0, 0, 0};

  void SetNickname(const std::string& str) {
    size_t len = std::min(str.length(), static_cast<size_t>(12));
    for (size_t i = 0; i < len; ++i) {
      nickname[i] = static_cast<char16_t>(str[i]);
    }
    nickname[len] = u'\0';
    has_nickname = true;
  }
};

using AST = std::vector<PokemonNode>;
} // namespace parser