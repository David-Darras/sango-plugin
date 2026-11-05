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
 * @file test_maker_reader.cc
 * @brief The tests of the readers of the Sango Maker files (core::maker).
 */

#include <cstring>
#include <string>
#include <vector>

#include "core/maker_reader.h"
#include "test.h"

namespace {
using namespace core::maker;

// A sink that keeps all the parts that the reader gives.
struct Recorder : Sink {
  struct Table {
    MapId map;
    EncounterMethod method;
    std::vector<overworld::EncounterSlot> slots;
  };

  std::vector<u32> problem_lines;
  std::vector<std::string> problems;
  std::vector<Table> tables;
  std::vector<TrainerId> trainers;
  std::vector<Opponent> opponents;
  std::vector<std::u16string> names;
  std::vector<ItemId> items;
  u32 money = 0;
  AiFlags ai = AiFlags::kNone;
  std::vector<XyTrainerModelId> models;
  std::vector<TrainerClassId> classes;
  std::vector<std::u16string> new_trainer_names;
  std::vector<std::u16string> new_class_titles;
  std::vector<TrainerClassId> new_class_sources;
  std::vector<ModelId> class_models;
  std::vector<TrainerClassId> class_pictures;
  Format format = Format::kSingle;
  u32 trainer_ends = 0;
  std::vector<std::string> species_changes;
  std::vector<pokemon::LevelMove> learnset;
  std::vector<pokemon::EvolutionStep> evolutions;
  std::vector<u16> stories;
  std::vector<Character> characters;
  bool is_character_list_full = false;
  // The addresses that the reader gives: they stay in its memory.
  std::vector<const void*> slot_addresses;
  const void* learnset_address = nullptr;

  void OnProblem(const c8* file, u32 line, const c8* what,
                 const c8* word) override {
    problem_lines.push_back(line);
    problems.push_back(std::string(what) + " " + word);
  }
  void OnEncounterTable(MapId map, EncounterMethod method,
                        const overworld::EncounterSlot* slots,
                        u32 count) override {
    tables.push_back({map, method, std::vector<overworld::EncounterSlot>(
                                       slots, slots + count)});
    slot_addresses.push_back(slots);
  }
  void OnTrainer(TrainerId trainer) override { trainers.push_back(trainer); }
  void OnTrainerFormat(Format value) override { format = value; }
  void OnTrainerName(const c16* name) override { names.push_back(name); }
  void OnTrainerMoney(u8 value) override { money = value; }
  void OnTrainerAi(AiFlags flags) override { ai = flags; }
  void OnTrainerModel(XyTrainerModelId value) override {
    models.push_back(value);
  }
  void OnTrainerClass(TrainerClassId value) override {
    classes.push_back(value);
  }
  TrainerClassId OnNewClass(const c16* title, TrainerClassId from) override {
    new_class_titles.push_back(title);
    new_class_sources.push_back(from);
    return static_cast<TrainerClassId>(100 + new_class_titles.size());
  }
  void OnClassOverworld(TrainerClassId id, ModelId model) override {
    class_models.push_back(model);
  }
  void OnClassPicture(TrainerClassId id, TrainerClassId from) override {
    class_pictures.push_back(from);
  }
  TrainerId OnNewTrainer(const c16* name) override {
    new_trainer_names.push_back(name);
    return static_cast<TrainerId>(50 + new_trainer_names.size());
  }
  void OnTrainerItems(const ItemId* list, u32 count) override {
    items.assign(list, list + count);
  }
  void OnTrainerPokemon(const Opponent& pokemon) override {
    opponents.push_back(pokemon);
  }
  void OnTrainerEnd() override { trainer_ends++; }
  void OnSpeciesStats(SpeciesId, FormId, const u8* stats) override {
    char text[64];
    std::snprintf(text, sizeof(text), "stats %u %u %u %u %u %u", stats[0],
                  stats[1], stats[2], stats[3], stats[4], stats[5]);
    species_changes.push_back(text);
  }
  void OnSpeciesTypes(SpeciesId, FormId, TypeId first,
                      TypeId second) override {
    species_changes.push_back("types " + std::to_string(static_cast<u32>(first)) +
                              " " + std::to_string(static_cast<u32>(second)));
  }
  void OnSpeciesAbilities(SpeciesId, FormId, AbilityId first, AbilityId second,
                          AbilityId hidden) override {
    species_changes.push_back(
        "abilities " + std::to_string(static_cast<u32>(first)) + " " +
        std::to_string(static_cast<u32>(second)) + " " +
        std::to_string(static_cast<u32>(hidden)));
  }
  void OnSpeciesCatchRate(SpeciesId, FormId, u8 rate) override {
    species_changes.push_back("catch " + std::to_string(rate));
  }
  void OnSpeciesMachine(SpeciesId, FormId, u8 number, bool is_hm,
                        bool can_learn) override {
    species_changes.push_back(std::string(is_hm ? "hm " : "tm ") +
                              std::to_string(number) +
                              (can_learn ? " on" : " off"));
  }
  void OnSpeciesLearnset(SpeciesId, FormId, const pokemon::LevelMove* moves,
                         u32 count) override {
    learnset.assign(moves, moves + count);
    learnset_address = moves;
  }
  void OnSpeciesEvolutions(SpeciesId, const pokemon::EvolutionStep* steps,
                           u32 count) override {
    evolutions.assign(steps, steps + count);
  }
  void OnStory(u16 id) override { stories.push_back(id); }
  bool OnCharacter(const Character& character) override {
    if (is_character_list_full) return false;
    characters.push_back(character);
    return true;
  }
  void OnRule(Rule rule, bool is_on) override {
    rules.push_back(std::to_string(static_cast<u32>(rule)) +
                    (is_on ? " on" : " off"));
  }
  void OnMultiplier(Multiplier multiplier, u32 hundredths) override {
    rules.push_back("x" + std::to_string(static_cast<u32>(multiplier)) +
                    " " + std::to_string(hundredths));
  }
  void OnShinyRate(pokemon::ShinyRate rate) override {
    rules.push_back("shiny " + std::to_string(static_cast<u32>(rate)));
  }
  void OnStarters(const SpeciesId* species) override {
    starters.assign(species, species + 3);
  }

  void OnShopItems(u32 shop_id, const pokemon::ShopItem* items,
                   u32 count) override {
    shops.push_back({shop_id, false, count});
    if (count > 0) shop_prices.push_back(items[count - 1].price);
  }
  void OnShopPokemons(u32 shop_id, const pokemon::ShopPokemon* pokemons,
                      u32 count) override {
    shops.push_back({shop_id, true, count});
    if (count > 0) shop_prices.push_back(pokemons[count - 1].price);
  }
  void OnItem(const ItemChange& change) override {
    item_changes.push_back(change);
  }
  void OnMove(const MoveChange& change) override {
    move_changes.push_back(change);
  }
  SpeciesId OnNewSpecies(const c16* name, SpeciesId data_from,
                         SpeciesId model) override {
    new_species.push_back({std::u16string(name), data_from, model});
    return static_cast<SpeciesId>(1020 + new_species.size());
  }

  std::vector<std::string> rules;
  std::vector<SpeciesId> starters;
  struct Shop {
    u32 id;
    bool is_pokemon_shop;
    u32 count;
  };
  std::vector<Shop> shops;
  std::vector<u32> shop_prices;
  std::vector<ItemChange> item_changes;
  std::vector<MoveChange> move_changes;
  struct NewSpecies {
    std::u16string name;
    SpeciesId data_from;
    SpeciesId model;
  };
  std::vector<NewSpecies> new_species;
};

// Reads a text with a reader of core::maker.
template <typename Read>
Recorder ReadText(Read read, const char* text) {
  Clear();
  Recorder recorder;
  read(text, static_cast<u32>(std::strlen(text)), recorder);
  return recorder;
}

std::u16string ScriptText(const Instruction& in) {
  return in.text != nullptr ? std::u16string(in.text) : std::u16string();
}

} // namespace

// The names.

TEST(NamesIgnoreTheCaseTheSpacesAndTheSigns) {
  u32 value = 0;
  CHECK(core::DataNames::Find(core::NameKind::kMove, "Thunder Shock", 13, &value));
  CHECK_EQ(value, MoveId::kThunderShock);
  CHECK(core::DataNames::Find(core::NameKind::kMove, "THUNDER_SHOCK", 13, &value));
  CHECK_EQ(value, MoveId::kThunderShock);
  CHECK(core::DataNames::Find(core::NameKind::kSpecies, " pikachu ", 9, &value));
  CHECK_EQ(value, SpeciesId::kPikachu);
}

TEST(NamesAcceptANumberAndAnAccent) {
  u32 value = 0;
  CHECK(core::DataNames::Find(core::NameKind::kMove, "84", 2, &value));
  CHECK_EQ(value, 84);
  // "Pokémon" in UTF-8: the é becomes an e.
  const char* name = "Pok\xC3\xA9mon Center";
  CHECK(!core::DataNames::Find(core::NameKind::kSpecies, name,
                               static_cast<u32>(std::strlen(name)), &value));
  CHECK(!core::DataNames::Find(core::NameKind::kSpecies, "Pikachuu", 8, &value));
}

// encounters.txt.

TEST(EncountersGiveOneTableForEachSection) {
  const Recorder r = ReadText(ReadEncounters,
                              "# A comment\n"
                              "[Route 101, Walk]\n"
                              "Pikachu, 3, 5\n"
                              "Eevee, 4   # The maximum level is 4.\n"
                              "[Route 102, Surf]\n"
                              "Shellos, 10, 12, Shellos East\n");
  CHECK_EQ(r.problems.size(), 0);
  CHECK_EQ(r.tables.size(), 2);
  CHECK_EQ(r.tables[0].map, MapId::kRoute101);
  CHECK_EQ(r.tables[0].method, EncounterMethod::kWalk);
  CHECK_EQ(r.tables[0].slots.size(), 2);
  CHECK_EQ(r.tables[0].slots[0].species, SpeciesId::kPikachu);
  CHECK_EQ(r.tables[0].slots[0].min_level, 3);
  CHECK_EQ(r.tables[0].slots[0].max_level, 5);
  CHECK_EQ(r.tables[0].slots[1].max_level, 4);
  CHECK_EQ(r.tables[1].method, EncounterMethod::kSurf);
  CHECK_EQ(r.tables[1].slots[0].form, FormId::kShellosEast);
}

TEST(EncountersReportTheMistakesWithTheirLines) {
  const Recorder r = ReadText(ReadEncounters,
                              "[Route 101, Walk]\n"
                              "Pikachuu, 3, 5\n"
                              "Eevee\n"
                              "[Route 999, Walk]\n"
                              "Pikachu, 3, 5\n");
  CHECK_EQ(r.problem_lines.size(), 3);
  CHECK_EQ(r.problem_lines[0], 2);
  CHECK_EQ(r.problem_lines[1], 3);
  CHECK_EQ(r.problem_lines[2], 4);
  // The Pokémon after an unknown map go nowhere.
  CHECK_EQ(r.tables.size(), 0);
}

TEST(EncountersSkipTheMarkOfAUtf8File) {
  const Recorder r = ReadText(ReadEncounters,
                              "\xEF\xBB\xBF[Route 101, Walk]\nPikachu, 3, 5\n");
  CHECK_EQ(r.problems.size(), 0);
  CHECK_EQ(r.tables.size(), 1);
}

// trainers.txt.

TEST(TrainersGiveAllThePartsOfATeam) {
  const Recorder r = ReadText(
      ReadTrainers,
      "[Route 102 Youngster Calvin]\n"
      "format = Double\n"
      "name = Jo\xC3\xA9\n"
      "ai = Casual, Competitive\n"
      "items = Potion, Full Heal\n"
      "money = 8\n"
      "pokemon = Pichu, level 7, item Oran Berry, ability Static, nature "
      "Timid, form 0, ivs 31, evs 0/252/4/0/0/252, moves Thunder Shock / "
      "Charm, shiny\n"
      "pokemon = Riolu\n");
  CHECK_EQ(r.problems.size(), 0);
  CHECK_EQ(r.trainers.size(), 1);
  CHECK_EQ(r.trainers[0], TrainerId::kRoute102YoungsterCalvin);
  CHECK_EQ(r.trainer_ends, 1);
  CHECK_EQ(r.format, Format::kDouble);
  CHECK(r.names.size() == 1 && r.names[0] == u"Joé");
  CHECK_EQ(r.ai, AiFlags::kCasual | AiFlags::kCompetitive);
  CHECK_EQ(r.items.size(), 4);
  CHECK_EQ(r.items[0], ItemId::kPotion);
  CHECK_EQ(r.items[1], ItemId::kFullHeal);
  CHECK_EQ(r.items[2], ItemId::kNone);
  CHECK_EQ(r.money, 8);
  CHECK_EQ(r.opponents.size(), 2);
  const Opponent& pichu = r.opponents[0];
  CHECK_EQ(pichu.species, SpeciesId::kPichu);
  CHECK(pichu.has_level && pichu.level == 7);
  CHECK(pichu.has_item && pichu.item == ItemId::kOranBerry);
  CHECK(pichu.has_ability && pichu.ability == AbilityId::kStatic);
  CHECK(pichu.has_nature && pichu.nature == Nature::kTimid);
  CHECK(pichu.has_ivs && pichu.ivs == 31);
  CHECK(pichu.has_evs && pichu.evs[1] == 252 && pichu.evs[2] == 4);
  CHECK(pichu.has_moves && pichu.moves[0] == MoveId::kThunderShock &&
        pichu.moves[1] == MoveId::kCharm && pichu.moves[2] == MoveId::kNone);
  CHECK(pichu.is_shiny);
  // A Pokémon with only its species keeps the values of the game.
  const Opponent& riolu = r.opponents[1];
  CHECK(!riolu.has_level && !riolu.has_item && !riolu.has_moves &&
        !riolu.is_shiny);
}

TEST(TrainersReportTheMistakes) {
  const Recorder r = ReadText(ReadTrainers,
                              "[Route 102 Youngster Calvin]\n"
                              "colour = blue\n"
                              "pokemon = Pichu, evs 1/2/3\n"
                              "ai = Lazy\n"
                              "no equal sign\n"
                              "[Nobody]\n"
                              "pokemon = Pichu\n");
  CHECK_EQ(r.problem_lines.size(), 5);
  CHECK_EQ(r.problem_lines[0], 2);
  CHECK_EQ(r.problem_lines[4], 6);
  // The Pokémon after an unknown trainer go nowhere.
  CHECK_EQ(r.opponents.size(), 1);
  CHECK_EQ(r.trainer_ends, 1);
}

// species.txt.

TEST(SpeciesGiveEachChange) {
  const Recorder r = ReadText(
      ReadSpecies,
      "[Pikachu]\n"
      "stats = 45, 70, 40, 60, 50, 100\n"
      "types = Electric\n"
      "abilities = Static, Lightning Rod\n"
      "catch rate = 190\n"
      "tms = 24, 25\n"
      "hms off = 5\n"
      "learnset = 1 Thunder Shock, 20 Thunderbolt\n"
      "evolutions = Level Up / 30 / Raichu, Item / Thunder Stone / Raichu\n");
  CHECK_EQ(r.problems.size(), 0);
  CHECK(r.species_changes.size() == 7);
  CHECK(r.species_changes[0] == "stats 45 70 40 60 50 100");
  // One type: the second type is the same.
  const std::string electric =
      std::to_string(static_cast<u32>(TypeId::kElectric));
  CHECK(r.species_changes[1] == "types " + electric + " " + electric);
  // Two abilities: the last one is also the hidden ability.
  const std::string lightning_rod =
      std::to_string(static_cast<u32>(AbilityId::kLightningRod));
  CHECK(r.species_changes[2] ==
        "abilities " + std::to_string(static_cast<u32>(AbilityId::kStatic)) +
            " " + lightning_rod + " " + lightning_rod);
  CHECK(r.species_changes[3] == "catch 190");
  CHECK(r.species_changes[4] == "tm 24 on");
  CHECK(r.species_changes[5] == "tm 25 on");
  CHECK(r.species_changes[6] == "hm 5 off");
  CHECK_EQ(r.learnset.size(), 2);
  CHECK_EQ(r.learnset[1].level, 20);
  CHECK_EQ(r.learnset[1].move, MoveId::kThunderbolt);
  CHECK_EQ(r.evolutions.size(), 2);
  CHECK_EQ(r.evolutions[0].method, EvolutionMethod::kLevelUp);
  CHECK_EQ(r.evolutions[0].value, 30);
  CHECK_EQ(r.evolutions[1].value, ItemId::kThunderStone);
  CHECK_EQ(r.evolutions[1].species, SpeciesId::kRaichu);
}

TEST(SpeciesReportTheMistakes) {
  const Recorder r = ReadText(ReadSpecies,
                              "[Pikachu]\n"
                              "stats = 1, 2, 3\n"
                              "learnset = Thunder Shock\n"
                              "evolutions = Level Up / 30\n"
                              "size = 2\n");
  CHECK_EQ(r.problem_lines.size(), 4);
}

// story.txt.

TEST(StoryLinksTheIfElseAndEnd) {
  const Recorder r = ReadText(ReadStory,
                              "[33000]\n"
                              "face player\n"     // 0
                              "ask Do you want it?\\nYes?\n"  // 1
                              "if yes\n"          // 2
                              "  give item Rare Candy 3\n"  // 3
                              "else\n"            // 4
                              "  say Too bad.\n"  // 5
                              "end\n");           // 6
  CHECK_EQ(r.problems.size(), 0);
  CHECK(r.stories.size() == 1 && r.stories[0] == 33000);
  const Story* story = FindStory(33000);
  CHECK(story != nullptr);
  if (story == nullptr) return;
  CHECK_EQ(story->count, 7);
  const u32 first = story->first;
  CHECK_EQ(GetInstruction(first + 0).op, Op::kFacePlayer);
  // "\n" in a text is a new line.
  CHECK(ScriptText(GetInstruction(first + 1)) == u"Do you want it?\nYes?");
  // A false "if" goes after the "else".
  CHECK_EQ(GetInstruction(first + 2).condition, Condition::kYes);
  CHECK_EQ(GetInstruction(first + 2).jump, first + 5);
  CHECK_EQ(GetInstruction(first + 3).a, ItemId::kRareCandy);
  CHECK_EQ(GetInstruction(first + 3).b, 3);
  // The "else" goes to the "end".
  CHECK_EQ(GetInstruction(first + 4).jump, first + 6);
}

TEST(StoryKeepsTheLineAndTheNameOfEachCommand) {
  const Recorder r = ReadText(ReadStory,
                              "# A comment\n"
                              "[33000]\n"
                              "say Hello.\n"
                              "\n"
                              "give item Potion 1\n");
  CHECK_EQ(r.problems.size(), 0);
  const Story* story = FindStory(33000);
  CHECK(story != nullptr);
  if (story == nullptr) return;
  CHECK_EQ(GetInstruction(story->first).line, 3);
  CHECK_EQ(GetInstruction(story->first + 1).line, 5);
  CHECK(std::string(GetCommandName(Op::kSay)) == "say");
  CHECK(std::string(GetCommandName(Op::kGiveItem)) == "give item");
  CHECK(std::string(GetCommandName(Op::kCallScript)) == "call");
}

TEST(ReaderMemoryHasTheSlotsAndTheLearnsets) {
  Recorder r = ReadText(ReadEncounters,
                        "[Route 101, Walk]\n"
                        "Pikachu, 3, 5\n");
  CHECK_EQ(r.tables.size(), 1);
  CHECK(IsReaderMemory(r.slot_addresses[0]));
  r = ReadText(ReadSpecies,
               "[Pikachu]\n"
               "learnset = 1 Thunder Shock\n");
  CHECK(r.learnset_address != nullptr && IsReaderMemory(r.learnset_address));
  const int local = 0;
  CHECK(!IsReaderMemory(&local));
  CHECK(!IsReaderMemory(nullptr));
}

TEST(StoryReadsAllTheCommands) {
  const Recorder r = ReadText(ReadStory,
                              "[33001]\n"
                              "choose A / B / C\n"
                              "if var 3 >= 10\n"
                              "end\n"
                              "warp Route 101, 10, 12, 2, Left\n"
                              "give money 100000\n"
                              "battle wild Pikachu\n"
                              "fade out white\n"
                              "music end\n");
  CHECK_EQ(r.problems.size(), 0);
  const Story* story = FindStory(33001);
  CHECK(story != nullptr);
  if (story == nullptr) return;
  const u32 first = story->first;
  CHECK_EQ(GetInstruction(first).a, 3);  // Three choices.
  CHECK(ScriptText(GetInstruction(first)) == u"A");
  CHECK_EQ(GetInstruction(first + 1).condition, Condition::kVarAtLeast);
  const Instruction& warp = GetInstruction(first + 3);
  CHECK_EQ(warp.a, MapId::kRoute101);
  CHECK(warp.b == 10 && warp.c == 12 && warp.d == 2 && warp.extra == 2);
  // The money has 32 bits, in a and b.
  const Instruction& money = GetInstruction(first + 4);
  CHECK_EQ(money.a | static_cast<u32>(money.b) << 16, 100000);
  CHECK_EQ(GetInstruction(first + 5).b, 5);  // The level is 5 without a level.
  CHECK_EQ(GetInstruction(first + 6).extra, 1);  // White.
  CHECK_EQ(GetInstruction(first + 7).op, Op::kMusicEnd);
}

TEST(StoryReportsTheMistakes) {
  const Recorder r = ReadText(ReadStory,
                              "[33002]\n"
                              "jump\n"
                              "else\n"
                              "if yes\n"
                              "[40000]\n"
                              "say Lost.\n");
  // "jump", "else" without "if", the "if" without "end" (at the next section)
  // and the script number.
  CHECK_EQ(r.problems.size(), 4);
  CHECK(FindStory(40000) == nullptr);
}

// characters.txt.

TEST(CharactersGiveScriptsAndTrainers) {
  const Recorder r = ReadText(
      ReadCharacters,
      "[Littleroot Town]\n"
      "Researcher Male, 47, 77, 6, Down, script 33000\n"
      "[Route 102]\n"
      "Lass, 14, 8, 0, Left, trainer Route 102 Lass Tiana, sight 4\n"
      "Youngster, 1, 2, 0, Up, trainer Route 102 Youngster Calvin\n");
  CHECK_EQ(r.problems.size(), 0);
  CHECK_EQ(r.characters.size(), 3);
  const Character& researcher = r.characters[0];
  CHECK_EQ(researcher.map, MapId::kLittlerootTown);
  CHECK_EQ(researcher.model, ModelId::kResearcherMale);
  CHECK(researcher.tile_x == 47 && researcher.tile_z == 77 &&
        researcher.height == 6 && researcher.facing == 1);
  CHECK_EQ(researcher.script, 33000);
  CHECK_EQ(researcher.trainer, TrainerId::kNone);
  CHECK_EQ(r.characters[1].trainer, TrainerId::kRoute102LassTiana);
  CHECK_EQ(r.characters[1].sight, 4);
  CHECK_EQ(r.characters[2].sight, 3);  // 3 without a sight.
}

TEST(CharactersReportTheMistakes) {
  Recorder r = ReadText(ReadCharacters,
                        "[Route 101]\n"
                        "Youngster, 1, 2, 0, Down\n"
                        "Nobody, 1, 2, 0, Down, script 33000\n"
                        "Lass, 1, 2, 0, Sideways, script 33000\n"
                        "Lass, 1, 2, 0, Up, sight 4\n"
                        "Lass, 1, 2, 0, Up, trainer Route 102 Lass Tiana, sight 11\n"
                        "[Nowhere]\n");
  CHECK_EQ(r.problem_lines.size(), 6);
  CHECK_EQ(r.characters.size(), 0);

  // A full list of characters.
  Clear();
  Recorder full;
  full.is_character_list_full = true;
  const char* text = "[Route 101]\nLass, 1, 2, 0, Up, script 33000\n";
  ReadCharacters(text, static_cast<u32>(std::strlen(text)), full);
  CHECK(full.problems.size() == 1 &&
        full.problems[0].find("too many characters") == 0);
}

TEST(RulesGiveEachRule) {
  const Recorder r = ReadText(ReadRules,
                              "items in battle = no\n"
                              "One Mega Evolution = yes\n"
                              "experience = 1.5\n"
                              "money = 2\n"
                              "catch rate = 0.25\n"
                              "shiny rate = 1/4096\n"
                              "shiny rate = game\n"
                              "starters = Bulbasaur, Charmander, Squirtle\n");
  CHECK_EQ(r.problems.size(), 0);
  CHECK(r.rules.size() == 7);
  CHECK(r.rules[0] == "0 off");
  CHECK(r.rules[1] == "1 on");
  CHECK(r.rules[2] == "x0 150");
  CHECK(r.rules[3] == "x2 200");
  CHECK(r.rules[4] == "x1 25");
  // 1/4096 is 1 out of 2^12: ShinyRate::k1_4096.
  CHECK(r.rules[5] == "shiny " + std::to_string(static_cast<u32>(
                                    pokemon::ShinyRate::k1_4096)));
  CHECK(r.rules[6] == "shiny 0");
  CHECK(r.starters.size() == 3 && r.starters[1] == SpeciesId::kCharmander);
}

TEST(RulesReportTheMistakes) {
  const Recorder r = ReadText(ReadRules,
                              "[Battle]\n"
                              "items in battle = maybe\n"
                              "experience = 30\n"
                              "money = a lot\n"
                              "shiny rate = 1/3000\n"
                              "starters = Bulbasaur, Charmander\n"
                              "starters = Bulbasaur, Charmander, Nobody\n"
                              "colour = blue\n"
                              "no equal sign\n");
  CHECK_EQ(r.problem_lines.size(), 9);
  CHECK(r.rules.empty() && r.starters.empty());
}

TEST(TrainersGiveTheModelOfXAndY) {
  const Recorder r = ReadText(ReadTrainers,
                              "[Route 102 Youngster Calvin]\n"
                              "model = Lysandre Final\n"
                              "[Route 102 Bug Catcher Rick]\n"
                              "model = Nobody\n");
  CHECK_EQ(r.models.size(), 1);
  if (r.models.size() != 1) return;
  CHECK(r.models[0] == XyTrainerModelId::kLysandreFinal);
  CHECK_EQ(r.problem_lines.size(), 1);
}

TEST(TrainersCanBeNew) {
  const Recorder r = ReadText(ReadTrainers,
                              "[new AZ]\n"
                              "format = Single\n"
                              "[new Route 102 Youngster Calvin]\n"
                              "[new AZ]\n");
  CHECK_EQ(r.new_trainer_names.size(), 1);
  if (r.new_trainer_names.size() != 1) return;
  CHECK(r.new_trainer_names[0] == u"AZ");
  CHECK_EQ(r.problem_lines.size(), 2);
}

TEST(TrainersCanHaveANewClass) {
  const Recorder r = ReadText(ReadTrainers,
                              "[new class Warden, Lass]\n"
                              "title = Warden\n"
                              "overworld = Az\n"
                              "picture = Youngster\n"
                              "colour = Red\n"
                              "[new class Lass]\n"
                              "[new AZ]\n"
                              "class = Warden\n");
  CHECK_EQ(r.new_class_titles.size(), 1);
  if (r.new_class_titles.size() != 1) return;
  CHECK(r.new_class_titles[0] == u"Warden");
  CHECK(r.new_class_sources[0] == TrainerClassId::kLass);
  CHECK_EQ(r.class_models.size(), 1);
  CHECK_EQ(r.class_pictures.size(), 1);
  CHECK_EQ(r.classes.size(), 1);
  CHECK_EQ(r.problem_lines.size(), 2);
}

TEST(TrainersGiveTheClass) {
  const Recorder r = ReadText(ReadTrainers,
                              "[Route 102 Youngster Calvin]\n"
                              "class = Lass\n"
                              "[Route 102 Bug Catcher Rick]\n"
                              "class = Nobody\n");
  CHECK_EQ(r.classes.size(), 1);
  if (r.classes.size() != 1) return;
  CHECK(r.classes[0] == TrainerClassId::kLass);
  CHECK_EQ(r.problem_lines.size(), 1);
}

TEST(ShopsGiveTheListOfEachShop) {
  const Recorder r = ReadText(ReadShops,
                              "[shop 12]\n"
                              "Potion, 100\n"
                              "Rare Candy, 1000\n"
                              "[all shops]\n"
                              "Poke Ball, 200\n"
                              "[Pokemon Shop]\n"
                              "Eevee, 10, 5000\n"
                              "[pokemon shop 3]\n"
                              "Larvitar, 15, 20000\n");
  CHECK_EQ(r.problems.size(), 0);
  CHECK_EQ(r.shops.size(), 4);
  if (r.shops.size() != 4) return;
  CHECK(r.shops[0].id == 12 && !r.shops[0].is_pokemon_shop &&
        r.shops[0].count == 2);
  CHECK(r.shops[1].id == kAllShops && r.shops[1].count == 1);
  CHECK(r.shops[2].id == kAllShops && r.shops[2].is_pokemon_shop);
  CHECK(r.shops[3].id == 3 && r.shops[3].is_pokemon_shop);
  CHECK(r.shop_prices == std::vector<u32>({1000, 200, 5000, 20000}));
}

TEST(ALineBeforeTheFirstSectionIsReported) {
  // The line is not in a [section]: nothing would use it.
  Recorder r = ReadText(ReadShops,
                        "# a comment\n"
                        "Master Ball, 1\n");
  CHECK_EQ(r.problem_lines.size(), 1);
  CHECK_EQ(r.problem_lines[0], 2);
  CHECK_EQ(r.shops.size(), 0);

  // The lines of a section that was reported are not reported again.
  r = ReadText(ReadMoves,
               "[Nobody]\n"
               "power = 50\n");
  CHECK_EQ(r.problem_lines.size(), 1);

  r = ReadText(ReadItems,
               "price = 5\n"
               "[Potion]\n"
               "price = 9\n");
  CHECK_EQ(r.problem_lines.size(), 1);
  CHECK_EQ(r.item_changes.size(), 1);
}

TEST(ShopsReportTheMistakes) {
  const Recorder r = ReadText(ReadShops,
                              "[shop]\n"
                              "[shop 1]\n"
                              "Nothing, 100\n"
                              "Potion\n"
                              "Potion, a lot\n"
                              "[pokemon shop]\n"
                              "Eevee, 101, 10\n"
                              "Nobody, 5, 10\n");
  CHECK_EQ(r.problem_lines.size(), 6);
  CHECK_EQ(r.shops.size(), 2);
}

TEST(ItemsGiveTheChanges) {
  const Recorder r = ReadText(ReadItems,
                              "[Potion]\n"
                              "price = 100\n"
                              "hp restore = 30\n"
                              "[Protein]\n"
                              "evs = 0, 63, 0, 0, 0, 0\n"
                              "fling power = 90\n");
  CHECK_EQ(r.problems.size(), 0);
  CHECK_EQ(r.item_changes.size(), 2);
  if (r.item_changes.size() != 2) return;
  const ItemChange& potion = r.item_changes[0];
  CHECK(potion.item == ItemId::kPotion && potion.has_price &&
        potion.price == 100 && potion.has_hp_restore &&
        potion.hp_restore == 30 && !potion.has_evs);
  const ItemChange& protein = r.item_changes[1];
  CHECK(protein.has_evs && protein.evs[1] == 63 && protein.has_fling_power &&
        protein.fling_power == 90 && !protein.has_price);
}

TEST(ItemsReportTheMistakes) {
  const Recorder r = ReadText(ReadItems,
                              "[Nothing]\n"
                              "[Potion]\n"
                              "price = 70000\n"
                              "fling power = 300\n"
                              "evs = 1, 2, 3\n"
                              "evs = 0, 200, 0, 0, 0, 0\n"
                              "colour = blue\n"
                              "no equal sign\n");
  CHECK_EQ(r.problem_lines.size(), 7);
}

TEST(MovesGiveTheChanges) {
  const Recorder r = ReadText(ReadMoves,
                              "[Thunder Shock]\n"
                              "type = Fire\n"
                              "category = Physical\n"
                              "power = 60\n"
                              "accuracy = never misses\n"
                              "pp = 20\n"
                              "priority = -1\n"
                              "[Tackle]\n"
                              "priority = +1\n");
  CHECK_EQ(r.problems.size(), 0);
  CHECK_EQ(r.move_changes.size(), 2);
  if (r.move_changes.size() != 2) return;
  const MoveChange& shock = r.move_changes[0];
  CHECK(shock.move == MoveId::kThunderShock && shock.has_type &&
        shock.type == TypeId::kFire && shock.has_category &&
        shock.category == DamageCategory::kPhysical && shock.power == 60 &&
        shock.accuracy == 101 && shock.pp == 20 && shock.priority == -1);
  const MoveChange& tackle = r.move_changes[1];
  CHECK(tackle.has_priority && tackle.priority == 1 && !tackle.has_power);
}

TEST(MovesReportTheMistakes) {
  const Recorder r = ReadText(ReadMoves,
                              "[Nothing]\n"
                              "[Tackle]\n"
                              "type = Wood\n"
                              "category = magic\n"
                              "power = 300\n"
                              "accuracy = 101\n"
                              "pp = 0\n"
                              "priority = 6\n"
                              "priority = --1\n"
                              "colour = blue\n");
  CHECK_EQ(r.problem_lines.size(), 9);
}

TEST(NewSpeciesHaveANameForTheOtherFiles) {
  const Recorder r = ReadText(ReadSpecies,
                              "[new Sangoat, Skiddo, Gogoat]\n"
                              "stats = 80, 95, 70, 60, 75, 85\n"
                              "[new Mewtwo, Mew]\n"
                              "[new Bigcat, Skitty]\n");
  CHECK_EQ(r.problems.size(), 1);  // Mewtwo is a name of the game.
  CHECK_EQ(r.new_species.size(), 2);
  if (r.new_species.size() != 2) return;
  CHECK(r.new_species[0].name == u"Sangoat");
  CHECK_EQ(r.new_species[0].data_from, SpeciesId::kSkiddo);
  CHECK_EQ(r.new_species[0].model, SpeciesId::kGogoat);
  // Without a model, the model is the species of the data.
  CHECK_EQ(r.new_species[1].model, SpeciesId::kSkitty);
  CHECK(r.species_changes.size() == 1);
  // The other files find the new names, until Clear().
  u32 value = 0;
  CHECK(core::DataNames::Find(core::NameKind::kSpecies, "sangoat", 7, &value));
  CHECK_EQ(value, 1021);
  CHECK(core::DataNames::Find(core::NameKind::kSpecies, "BIGCAT", 6, &value));
  CHECK_EQ(value, 1022);
  Clear();
  CHECK(!core::DataNames::Find(core::NameKind::kSpecies, "Sangoat", 7, &value));
}

TEST(NamesGiveTheNameOfAValue) {
  CHECK(std::string(core::DataNames::GetName(core::NameKind::kMove, 84)) ==
        "thundershock");
  const c8* name = core::DataNames::GetName(core::NameKind::kSpecies, 25);
  u32 value = 0;
  CHECK(name != nullptr &&
        core::DataNames::Find(core::NameKind::kSpecies, name,
                              static_cast<u32>(std::strlen(name)), &value) &&
        value == 25);
  CHECK(core::DataNames::GetName(core::NameKind::kMove, 60000) == nullptr);
}
