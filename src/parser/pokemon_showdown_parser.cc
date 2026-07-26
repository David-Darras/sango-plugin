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

#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "common.h"
#include "parser/ability.inc"
#include "parser/ball.inc"
#include "parser/item.inc"
#include "parser/move.inc"
#include "parser/nature.inc"
#include "parser/pokemon_node.h"
#include "parser/species.inc"
#include "system/file.h"

namespace parser {
template <typename T>
T FindIndexInArray(const std::string& name, const char* array[], size_t size) {
  std::string lower_name = name;
  std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  for (size_t i = 0; i < size; ++i) {
    std::string current_item = array[i];
    std::transform(current_item.begin(), current_item.end(),
                   current_item.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lower_name == current_item) {
      return static_cast<T>(i);
    }
  }
  return 0;
}

std::string Trim(const std::string& str) {
  size_t first = str.find_first_not_of(" \t\r\n\xA0");
  if (first == std::string::npos) return "";
  size_t last = str.find_last_not_of(" \t\r\n\xA0");
  return str.substr(first, (last - first + 1));
}

void ParseStatsBlock(const std::string& stats_str, Stats& stats_target) {
  std::stringstream ss(stats_str);
  std::string segment;
  while (std::getline(ss, segment, '/')) {
    std::stringstream chunk(Trim(segment));
    int value;
    std::string stat_name;
    if (chunk >> value >> stat_name) {
      stat_name = Trim(stat_name);
      if (stat_name == "HP") stats_target.hp = value;
      else if (stat_name == "Atk") stats_target.atk = value;
      else if (stat_name == "Def") stats_target.def = value;
      else if (stat_name == "SpA") stats_target.spa = value;
      else if (stat_name == "SpD") stats_target.spd = value;
      else if (stat_name == "Spe") stats_target.spe = value;
    }
  }
}

int ConvertGender(const std::string& gender_str) {
  if (gender_str == "M") return 0;
  if (gender_str == "F") return 1;
  return 2;
}

AST ParsePokemonShowdownString(const std::string& input) {
  AST ast;
  std::stringstream ss(input);
  std::string line;

  PokemonNode current_pokemon;
  bool in_pokemon = false;
  size_t move_index = 0;

  while (std::getline(ss, line)) {
    line = Trim(line);

    if (line.empty()) {
      if (in_pokemon) {
        ast.push_back(current_pokemon);
        current_pokemon = PokemonNode();
        in_pokemon = false;
        move_index = 0;
      }
      continue;
    }

    if (!in_pokemon) {
      in_pokemon = true;

      size_t at_pos = line.find('@');
      std::string identity_part = line;
      std::string raw_item = "";
      if (at_pos != std::string::npos) {
        raw_item = Trim(line.substr(at_pos + 1));
        current_pokemon.item = FindIndexInArray<uint16_t>(
            raw_item, ITEM_NAMES, ITEM_COUNT);
        identity_part = line.substr(0, at_pos);
      }

      std::vector<std::string> brackets;
      size_t pos = 0;
      while ((pos = identity_part.find('(', pos)) != std::string::npos) {
        size_t close_pos = identity_part.find(')', pos);
        if (close_pos != std::string::npos) {
          brackets.push_back(
              Trim(identity_part.substr(pos + 1, close_pos - pos - 1)));
          pos = close_pos + 1;
        } else {
          break;
        }
      }

      size_t first_bracket = identity_part.find('(');
      std::string first_word = Trim(identity_part.substr(0, first_bracket));

      std::string final_name = "";

      if (brackets.size() == 2) {
        current_pokemon.SetNickname(first_word);
        final_name = brackets[0];
        current_pokemon.gender = ConvertGender(brackets[1]);
      } else if (brackets.size() == 1) {
        if (brackets[0] == "M" || brackets[0] == "F") {
          final_name = first_word;
          current_pokemon.gender = ConvertGender(brackets[0]);
        } else {
          current_pokemon.SetNickname(first_word);
          final_name = brackets[0];
          current_pokemon.gender = 2;
        }
      } else {
        final_name = first_word;
        current_pokemon.gender = 2;
      }

      current_pokemon.species = FindIndexInArray<uint16_t>(
          final_name, SPECIES_NAMES, SPECIES_COUNT);
      continue;
    }

    if (line.rfind("Ability:", 0) == 0) {
      std::string raw_ability = Trim(line.substr(8));
      current_pokemon.ability = FindIndexInArray<uint8_t>(
          raw_ability, ABILITY_NAMES, ABILITY_COUNT);
    } else if (line.rfind("Level:", 0) == 0) {
      // Parsing du Level
      current_pokemon.level = static_cast<u8>(std::stoi(Trim(line.substr(6))));
    } else if (line.rfind("Happiness:", 0) == 0) {
      current_pokemon.happiness = static_cast<u8>(std::stoi(
          Trim(line.substr(10))));
    } else if (line.rfind("Shiny:", 0) == 0) {
      std::string shiny_val = Trim(line.substr(6));
      current_pokemon.is_shiny = (shiny_val == "Yes" || shiny_val == "yes");
    } else if (line.rfind("Ball:", 0) == 0) {
      std::string raw_ball = Trim(line.substr(5));
      current_pokemon.ball = FindIndexInArray<uint8_t>(
          raw_ball, BALL_NAMES, BALL_COUNT);
    } else if (line.rfind("EVs:", 0) == 0) {
      ParseStatsBlock(line.substr(4), current_pokemon.evs);
    } else if (line.rfind("IVs:", 0) == 0) {
      ParseStatsBlock(line.substr(4), current_pokemon.ivs);
    } else if (line.find("Nature") != std::string::npos) {
      std::string raw_nature = Trim(line.substr(0, line.find("Nature")));
      current_pokemon.nature = FindIndexInArray<uint8_t>(
          raw_nature, NATURE_NAMES, NATURE_COUNT);
    } else if (line[0] == '-') {
      if (move_index < 4) {
        std::string raw_move = Trim(line.substr(1));
        current_pokemon.moves[move_index] = FindIndexInArray<uint16_t>(
            raw_move, MOVE_NAMES, MOVE_COUNT);
        move_index++;
      }
    }
  }

  if (in_pokemon) {
    ast.push_back(current_pokemon);
  }

  return ast;
}

std::string LoadFileToString(const c16* path) {
  File file(path, false);
  static char buffer[4096];
  std::memset(buffer, 0, sizeof(buffer));
  file.Read(buffer, sizeof(buffer) - 1);
  return std::string(buffer);
}

bool ParsePokemonShowdownFile(const c16* path, AST& ast) {
  std::string content = LoadFileToString(path);
  if (content.empty()) return false;
  ast = ParsePokemonShowdownString(content);
  return true;
}
} // namespace parser