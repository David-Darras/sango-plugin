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
 * @file archive.cc
 * @brief Lets a product change the files that the game reads from its archives.
 *
 * The declarations are in core/patch/archive.h.
 */

#include "core/patch/archive.h"
#include <cstring>
#include "battle/patch/move_choreography.h"
#include "battle/patch/trainer_class.h"
#include "battle/patch/trainer_model_extension.h"
#include "battle/patch/effect_style.h"
#include "battle/patch/item_extension.h"
#include "renderer/patch/type_icon.h"
#include "core/hook.h"
#include "core/problems.h"
#include "core/utils.h"
#include "system/native/file.h"
#include "system/address.h"

namespace core {

namespace {
core::Hook<bool(void*, ArchiveInput*)> read_file_async_hook;
core::Hook<bool(u32*, void*, u32, void*, u32, u32, u32)> read_file_async2_hook;
core::Hook<void*(u32*, u32, void*, void*, s32, u32*)> load_compressed_hook;
core::Hook<u32(u32*, u32)> get_file_size_hook;
core::Hook<u32(u32*, u32, void*)> load_data2_hook;
core::Hook<u32(u32*, u32)> get_file_size2_hook;
core::Hook<void(u32*, u32, u32*)> get_info_hook;
core::Hook<void(u32*, u32, u32, void*, u32*)> read_hook;
core::Hook<bool(void*, void*, const c16*, u32, u32, u32, void*, u32, void*, void*)>
    file_async_read_hook;
} // namespace

static bool FileStartAsyncReadHook(void* file, void* heap, const c16* filename,
                                   u32 offset, u32 size, u32 size_again,
                                   void* buffer, u32 priority, void* callback,
                                   void* work);

namespace {
} // namespace

void Archive::Initialize() {
  read_file_async_hook.Install(sys::address::kArchiveReadFileAsync,
                               ReadFileAsync);
  read_file_async2_hook.Install(sys::address::kArchiveReadFileAsync2,
                                ReadFileAsync2);
  load_compressed_hook.Install(sys::address::kArchiveLoadCompressedFile,
                               LoadCompressedHook);
  get_file_size_hook.Install(sys::address::kArchiveGetFileSize,
                             GetFileSizeHook);
  load_data2_hook.Install(sys::address::kArchiveLoadData2, LoadDataHook2);
  if (sys::address::kArchiveGetFileSize2) {
    get_file_size2_hook.Install(sys::address::kArchiveGetFileSize2,
                                GetFileSizeHook2);
  }
  file_async_read_hook.Install(sys::address::kFileStartAsyncRead,
                               FileStartAsyncReadHook);
  if (sys::address::kArchiveGetInfo) {
    get_info_hook.Install(sys::address::kArchiveGetInfo, GetInfoHook);
    read_hook.Install(sys::address::kArchiveRead, ReadHook);
  }
}

static bool OpenSd(sys::File* file, const c16* path) {
  static bool mounted = false;
  file->Open(path, sys::File::kRead);
  if (file->IsOpen()) return true;
  if (!mounted) {
    mounted = true;
    sys::File::MountSdmc();
    file->Open(path, sys::File::kRead);
  }
  return file->IsOpen();
}

struct OverrideEntry {
  u32 file_id;
  u32 size;
};

// The files of an archive that the SD card replaces. The index file has the
// number of files, then (file id, size) for each file, sorted by file id.
//
// With `whole_file` false, a file of the SD card has its size (4 bytes), then
// the data of the file without compression. With `whole_file` true, a file of
// the SD card has the same bytes as the file of the archive (compressed), and
// the game reads it as it reads the archive.
struct OverrideSet {
  ArchiveId archive;
  const c16* index_path;
  const c16* file_format; ///< The path of a file, with one `%d`.
  u32 token; ///< The bit that marks a read of this set in an offset.
  bool whole_file;
  OverrideEntry* entries;
  u32 capacity;
  u32 count;
  bool loaded;
};

// The offsets of the real archives are less than 0x40000000. A fake offset has
// the token in its four high bits and the file id in the other bits.
static constexpr u32 kPokemonToken = 0x80000000;
static constexpr u32 kOverworldToken = 0x40000000;
static constexpr u32 kLandToken = 0xC0000000;
static constexpr u32 kAreaToken = 0xD0000000;
static constexpr u32 kMatrixToken = 0xE0000000;
static constexpr u32 kZoneToken = 0x50000000;
static constexpr u32 kItemIconToken = 0xF0000000;
static constexpr u32 kItemDataToken = 0xB0000000;
static constexpr u32 kTrainerModelToken = 0xA0000000;
static constexpr u32 kTrainerSequenceToken = 0x90000000;
static constexpr u32 kTrainerLayoutToken = 0x70000000;
static constexpr u32 kBackgroundToken = 0x60000000;
// The fake offset of the Pokédex data file of Sango Studio. It is not a token
// of the four high bits: the offsets of the model archive (1 GB) take all the
// values below 0x40000000. A read is the one of the studio only when its
// archive is the Pokédex data (see ReadHook).
static constexpr u32 kStudioToken = 0x3F000000;
static constexpr u32 kStudioMask = 0xFF000000;
// The fake offset of the icons of the species that Sango Studio writes in
// sdmc:/sango-studio/species/icon. Like the token of the studio, it is not a
// token of the four high bits: it is valid only in the archive of the icons
// (a/0/9/1), which is small.
static constexpr u32 kIconToken = 0x3E000000;
// The same for the other small archives that Sango Studio replaces file by
// file: the level-up moves, the egg moves, the evolutions, the experience
// tables (leveling_rate) and the texts of the eight languages (game_text). The
// files of the SD card have the bytes of the files of the archive (these
// archives are not compressed).
static constexpr u32 kLevelUpToken = 0x3D000000;
static constexpr u32 kEggMoveToken = 0x3C000000;
static constexpr u32 kEvolutionToken = 0x3B000000;
static constexpr u32 kGrowthToken = 0x3A000000;
// The data, the teams and the classes of the trainers that Sango Studio
// writes (the archives 36, 38 and 37).
static constexpr u32 kTrainerDataToken = 0x30000000;
static constexpr u32 kTrainerClassToken = 0x2F000000;
static constexpr u32 kTrainerTeamToken = 0x2E000000;
// The pictures of the trainers that Sango Studio writes (the archive 160).
static constexpr u32 kTrainerPictureToken = 0x2D000000;
// The battle models of the trainers that Sango Studio writes (the archive
// 133): the files replace the models of the game by their number.
static constexpr u32 kStudioTrainerModelToken = 0x2C000000;
// The choreographies of the moves that Sango Studio writes (the archive 34): the
// files replace the choreographies of the game by their number.
static constexpr u32 kStudioChoreographyToken = 0x2B000000;
// The particle effects that Sango Studio writes (the archive 31): the files
// replace the effects of the game by their number.
static constexpr u32 kStudioEffectToken = 0x2A000000;
// The scripts that Sango Studio writes: the field scripts (the archive 29), the
// AI of the trainers (7) and the scripts of Pokémon-Amie (89). The files
// replace the files of the game by their number.
static constexpr u32 kStudioFieldScriptToken = 0x29000000;
static constexpr u32 kStudioBattleAiToken = 0x28000000;
static constexpr u32 kStudioAmieScriptToken = 0x27000000;
// The screen of the DexNav that Sango Studio writes (the archive 248, one file):
// the pictures and the layout of the bottom screen.
static constexpr u32 kStudioDexNavToken = 0x26000000;
static constexpr u32 kTextTokenFirst = 0x32000000;  // + 0x01000000 * language
// The cries: the sequences and the wave archives of the Pokémon (the archive
// 100), not compressed.
static constexpr u32 kVoiceToken = 0x31000000;
// The fonts that Sango Studio replaces (the archive 167), not compressed.
static constexpr u32 kFontToken = 0x25000000;
// The item icons that Sango Studio replaces (the archive 92), compressed as in
// the archive.
static constexpr u32 kStudioItemIconToken = 0x24000000;
// The layouts (the menus) that Sango Studio replaces: every archive that has
// layouts uses this token. The set of the archive is found by the archive, so
// the token can be the same for all of them. The files have the bytes of the
// files of the archive (compressed). A real offset of these archives is far
// smaller than this token (the biggest one is 42 MB).
static constexpr u32 kLayoutToken = 0x23000000;
// The files of the battle backgrounds that Sango Studio replaces (the archive
// 136), compressed as in the archive.
static constexpr u32 kStudioBackgroundToken = 0x22000000;
// The fake offset of the mega evolution tables that Sango Studio writes
// (the archive 193), not compressed.
static constexpr u32 kMegaEvolutionToken = 0x21000000;
// The PSS icons that Sango Studio replaces (the archive 108), the models of
// the player in the Soar sequence (the archive 254) and the pack of the
// introduction (the archive 269, with the portraits of the choice of the
// character). The files have the bytes of the files of the archives.
static constexpr u32 kPlayerIconToken = 0x1F000000;
static constexpr u32 kSoaringModelToken = 0x1E000000;
static constexpr u32 kIntroToken = 0x1D000000;
// The title screen that Sango Studio changes: the models of the scene (the
// archive 152, compressed as in the archive) and the two sequences that play
// it (the archive 162, not compressed).
static constexpr u32 kTitleDataToken = 0x1C000000;
static constexpr u32 kTitleSequenceToken = 0x1B000000;
// True for the fake offset of a small archive: the token has no bit in the
// four high bits alone (a real offset of these archives is far smaller).
static constexpr bool IsSmallToken(u32 token) {
  return token != 0 && token < 0x40000000;
}
static constexpr u32 kTokenMask = 0xF0000000;
static constexpr u32 kFileIdMask = ~kTokenMask;
static OverrideEntry pokemon_entries[6144];
static OverrideEntry overworld_entries[1024];
static OverrideEntry land_entries[1024];
static OverrideEntry area_entries[512];
static OverrideEntry matrix_entries[512];
static OverrideEntry zone_entries[1024];
static OverrideEntry trainer_model_entries[256];
static OverrideEntry trainer_sequence_entries[64];
static OverrideEntry trainer_layout_entries[8];
static OverrideEntry background_entries[128];
// The icons of the new items: RegisterItemIcon() fills this list.
static constexpr u32 kMaxItemIcons = 128;
static OverrideEntry item_icon_entries[kMaxItemIcons];
static c16 item_icon_paths[kMaxItemIcons][96];
// The data of the new items: RegisterItemData() fills this list.
static constexpr u32 kMaxItemData = 128;
static OverrideEntry item_data_entries[kMaxItemData];
static u8 item_data_raw[kMaxItemData][Archive::kItemDataFileSize];
// The Pokédex data file of Sango Studio: SetPokedexDataFile() fills this list.
static constexpr u32 kPokedexDataFile = 1;
static const c16 kStudioPokedexPath[] =
    u"sdmc:/sango-studio/species/pokedex_data.bin";
static OverrideEntry studio_pokedex_entries[1];
// The icons of the species that Sango Studio changed: the index file lists them.
static constexpr u32 kMaxSpeciesIcons = 1280;
static OverrideEntry species_icon_entries[kMaxSpeciesIcons];
// The changed files of the other small archives of Sango Studio.
static constexpr u32 kMaxStudioFiles = 512;
static OverrideEntry level_up_entries[kMaxStudioFiles];
static OverrideEntry egg_move_entries[kMaxStudioFiles];
static OverrideEntry evolution_entries[kMaxStudioFiles];
static constexpr u32 kMaxMegaEvolutionFiles = 128;
static OverrideEntry mega_evolution_entries[kMaxMegaEvolutionFiles];
static constexpr u32 kMaxGrowthFiles = 16;
static OverrideEntry growth_entries[kMaxGrowthFiles];
static constexpr u32 kMaxTrainerFiles = 1024;
static OverrideEntry trainer_data_entries[kMaxTrainerFiles];
static OverrideEntry trainer_class_entries[kMaxTrainerFiles];
static OverrideEntry trainer_team_entries[kMaxTrainerFiles];
static OverrideEntry trainer_picture_entries[kMaxTrainerFiles];
static OverrideEntry studio_trainer_model_entries[256];
static OverrideEntry studio_choreography_entries[1024];
static OverrideEntry studio_effect_entries[2048];
static OverrideEntry studio_field_script_entries[128];
static OverrideEntry studio_battle_ai_entries[32];
static OverrideEntry studio_amie_script_entries[8];
static OverrideEntry studio_dex_nav_entries[4];
static constexpr u32 kMaxVoiceFiles = 256;
static OverrideEntry voice_entries[kMaxVoiceFiles];
static OverrideEntry font_entries[16];
static constexpr u32 kMaxStudioItemIcons = 1024;
static OverrideEntry studio_item_icon_entries[kMaxStudioItemIcons];
static constexpr u32 kMaxPlayerIcons = 320;
static OverrideEntry player_icon_entries[kMaxPlayerIcons];
static OverrideEntry soaring_model_entries[4];
static OverrideEntry intro_entries[2];
static OverrideEntry title_data_entries[16];
static OverrideEntry title_sequence_entries[4];
static constexpr u32 kMaxTextFiles = 128;
static OverrideEntry text_entries[8][kMaxTextFiles];
// The changed files of the archives with layouts (see tools/gen_layout_sets.py):
// the sets of these archives share this list of entries.
static OverrideEntry studio_background_entries[160];
#include "core/data/layout_pool.inc"
static OverrideEntry layout_entries[kLayoutPoolSize];
static OverrideSet override_sets[] = {
    {ArchiveId::kPokemonModel, u"sdmc:/sango-studio/species/model/index.bin",
     u"sdmc:/sango-studio/species/model/%d.pc", kPokemonToken, false, pokemon_entries,
     6144, 0, false},
    {ArchiveId::kOverworldModel, u"sdmc:/sango-studio/overworld/model/index.bin",
     u"sdmc:/sango-studio/overworld/model/%d.bin", kOverworldToken, true, overworld_entries,
     1024, 0, false},
    {ArchiveId::kMapLandData, u"sdmc:/sango-studio/overworld/land/index.bin",
     u"sdmc:/sango-studio/overworld/land/%d.bin", kLandToken, true, land_entries, 1024, 0,
     false},
    {ArchiveId::kOverworldArea, u"sdmc:/sango-studio/overworld/area/index.bin",
     u"sdmc:/sango-studio/overworld/area/%d.bin", kAreaToken, true, area_entries, 512, 0,
     false},
    {ArchiveId::kMapMatrix, u"sdmc:/sango-studio/overworld/matrix/index.bin",
     u"sdmc:/sango-studio/overworld/matrix/%d.bin", kMatrixToken, true, matrix_entries, 512, 0,
     false},
    // The packs of the zones (the events: characters, warps, signs, triggers)
    // that the Overworld page of Sango Studio writes.
    {ArchiveId::kZoneData, u"sdmc:/sango-studio/overworld/zone/index.bin",
     u"sdmc:/sango-studio/overworld/zone/%d.bin", kZoneToken, true, zone_entries, 1024, 0,
     false},
    // The models of the trainers of X and Y: the files after the last file of
    // the archive (see battle::TrainerModelExtension). The pack of X and Y of
    // Sango Studio writes them.
    {ArchiveId::kTrainerModel, u"sdmc:/sango-studio/trainer/xy_model/index.bin",
     u"sdmc:/sango-studio/trainer/xy_model/%d.bin", kTrainerModelToken, true,
     trainer_model_entries, 256, 0, false},
    // The introductions and the background of the trainers of X and Y: new
    // files of the archive of the battle animations and of the layouts of the
    // intros. The game gets them with the replacements of the extension.
    {ArchiveId::kBattleMoveChoreography,
     u"sdmc:/sango-studio/trainer/xy_sequence/index.bin",
     u"sdmc:/sango-studio/trainer/xy_sequence/%d.bin", kTrainerSequenceToken, true,
     trainer_sequence_entries, 64, 0, false},
    {ArchiveId::kMoveEffectLayout, u"sdmc:/sango-studio/trainer/xy_layout/index.bin",
     u"sdmc:/sango-studio/trainer/xy_layout/%d.bin", kTrainerLayoutToken, true,
     trainer_layout_entries, 8, 0, false},
    // The pieces of the battle backgrounds of X and Y: new files after the
    // last file of the archive (see battle::XyBackground).
    {ArchiveId::kBattleBackground, u"sdmc:/sango-studio/battle/xy_background/index.bin",
     u"sdmc:/sango-studio/battle/xy_background/%d.bin", kBackgroundToken, true,
     background_entries, 128, 0, false},
    // The files of the icons have the paths that RegisterItemIcon() got. The
    // list is in memory: no index file, so the set is already loaded.
    {ArchiveId::kItemIcon, u"", u"", kItemIconToken, true, item_icon_entries,
     kMaxItemIcons, 0, true},
    {ArchiveId::kItemData, u"", u"", kItemDataToken, true, item_data_entries,
     kMaxItemData, 0, true},
    // The Pokédex data file of Sango Studio. The list is in memory: no index
    // file, so the set is already loaded.
    {ArchiveId::kPokedexData, u"", u"", kStudioToken, true,
     studio_pokedex_entries, 1, 0, true},
    // The icons of the species: the files of Sango Studio have the same bytes
    // as the files of the archive (compressed), and the index file has the
    // number of files, then (file id, size) for each one, sorted by file id.
    {ArchiveId::kPokemonIcon, u"sdmc:/sango-studio/species/icon/index.bin",
     u"sdmc:/sango-studio/species/icon/%d.bin", kIconToken, true,
     species_icon_entries, kMaxSpeciesIcons, 0, false},
    // The moves, the evolutions and the experience tables.
    {ArchiveId::kLevelUpMove, u"sdmc:/sango-studio/species/level_up_moves/index.bin",
     u"sdmc:/sango-studio/species/level_up_moves/%d.bin", kLevelUpToken, true,
     level_up_entries, kMaxStudioFiles, 0, false},
    {ArchiveId::kEggMove, u"sdmc:/sango-studio/species/egg_moves/index.bin",
     u"sdmc:/sango-studio/species/egg_moves/%d.bin", kEggMoveToken, true,
     egg_move_entries, kMaxStudioFiles, 0, false},
    {ArchiveId::kEvolutionTable, u"sdmc:/sango-studio/species/evolution/index.bin",
     u"sdmc:/sango-studio/species/evolution/%d.bin", kEvolutionToken, true,
     evolution_entries, kMaxStudioFiles, 0, false},
    {ArchiveId::kMegaEvolutionTable,
     u"sdmc:/sango-studio/species/mega_evolution/index.bin",
     u"sdmc:/sango-studio/species/mega_evolution/%d.bin", kMegaEvolutionToken,
     true, mega_evolution_entries, kMaxMegaEvolutionFiles, 0, false},
    {ArchiveId::kGrowthTable, u"sdmc:/sango-studio/leveling_rate/index.bin",
     u"sdmc:/sango-studio/leveling_rate/%d.bin", kGrowthToken, true,
     growth_entries, kMaxGrowthFiles, 0, false},
    // The trainers: their data, their classes and their teams.
    {ArchiveId::kTrainerData, u"sdmc:/sango-studio/trainer/data/index.bin",
     u"sdmc:/sango-studio/trainer/data/%d.bin", kTrainerDataToken, true,
     trainer_data_entries, kMaxTrainerFiles, 0, false},
    {ArchiveId::kTrainerTypeData, u"sdmc:/sango-studio/trainer/class/index.bin",
     u"sdmc:/sango-studio/trainer/class/%d.bin", kTrainerClassToken, true,
     trainer_class_entries, kMaxTrainerFiles, 0, false},
    {ArchiveId::kTrainerPokemonData, u"sdmc:/sango-studio/trainer/pokemon/index.bin",
     u"sdmc:/sango-studio/trainer/pokemon/%d.bin", kTrainerTeamToken, true,
     trainer_team_entries, kMaxTrainerFiles, 0, false},
    // The pictures of the trainers: the files have the bytes of the files of
    // the archive (compressed).
    {ArchiveId::kTrainerTexture, u"sdmc:/sango-studio/trainer/picture/index.bin",
     u"sdmc:/sango-studio/trainer/picture/%d.bin", kTrainerPictureToken, true,
     trainer_picture_entries, kMaxTrainerFiles, 0, false},
    // The battle models of the trainers: the files have the bytes of the files
    // of the archive (compressed). This set comes after the one of the models of
    // X and Y: a file that Sango Studio replaces is found first.
    {ArchiveId::kTrainerModel, u"sdmc:/sango-studio/trainer/model/index.bin",
     u"sdmc:/sango-studio/trainer/model/%d.bin", kStudioTrainerModelToken, true,
     studio_trainer_model_entries, 256, 0, false},
    // The choreographies of the moves: the files have the bytes of the files of the
    // archive (compressed).
    {ArchiveId::kBattleMoveChoreography,
     u"sdmc:/sango-studio/choreography/index.bin",
     u"sdmc:/sango-studio/choreography/%d.bin", kStudioChoreographyToken,
     true, studio_choreography_entries, 1024, 0, false},
    // The particle effects: the files have the bytes of the files of the
    // archive (compressed).
    {ArchiveId::kMoveEffectParticle, u"sdmc:/sango-studio/effect/index.bin",
     u"sdmc:/sango-studio/effect/%d.bin", kStudioEffectToken, true,
     studio_effect_entries, 2048, 0, false},
    // The scripts: the files have the bytes of the files of the archive
    // (compressed).
    {ArchiveId::kOverworldScript, u"sdmc:/sango-studio/script/field/index.bin",
     u"sdmc:/sango-studio/script/field/%d.bin", kStudioFieldScriptToken, true,
     studio_field_script_entries, 128, 0, false},
    {ArchiveId::kBattleAi, u"sdmc:/sango-studio/script/ai/index.bin",
     u"sdmc:/sango-studio/script/ai/%d.bin", kStudioBattleAiToken, true,
     studio_battle_ai_entries, 32, 0, false},
    {ArchiveId::kPokemonAmieScript, u"sdmc:/sango-studio/script/amie/index.bin",
     u"sdmc:/sango-studio/script/amie/%d.bin", kStudioAmieScriptToken, true,
     studio_amie_script_entries, 8, 0, false},
    // The DexNav: one file with the bytes of the file of the archive
    // (compressed).
    {ArchiveId::kDexNav, u"sdmc:/sango-studio/dexnav/index.bin",
     u"sdmc:/sango-studio/dexnav/%d.bin", kStudioDexNavToken, true,
     studio_dex_nav_entries, 4, 0, false},
    // The cries: the files of the wave archives that the user replaced.
    {ArchiveId::kPokemonVoice, u"sdmc:/sango-studio/species/cry/index.bin",
     u"sdmc:/sango-studio/species/cry/%d.bin", kVoiceToken, true,
     voice_entries, kMaxVoiceFiles, 0, false},
    // The fonts: the files have the bytes of the font files of the archive.
    {ArchiveId::kFont, u"sdmc:/sango-studio/font/index.bin",
     u"sdmc:/sango-studio/font/%d.bin", kFontToken, true,
     font_entries, 16, 0, false},
    // The item icons that Sango Studio replaces (the Item page and the icon
    // packs): the files have the bytes of the files of the archive (CLIM images
    // compressed with LZ11). This set comes after the set of the new items, so
    // the icons of the new items keep their own files.
    {ArchiveId::kItemIcon, u"sdmc:/sango-studio/item/icon/index.bin",
     u"sdmc:/sango-studio/item/icon/%d.bin", kStudioItemIconToken, true,
     studio_item_icon_entries, kMaxStudioItemIcons, 0, false},
    // The PSS icons: the files have the bytes of the files of the archive (CLIM
    // images with a palette, compressed with LZ11).
    {ArchiveId::kPlayerIcon, u"sdmc:/sango-studio/player/icon/index.bin",
     u"sdmc:/sango-studio/player/icon/%d.bin", kPlayerIconToken, true,
     player_icon_entries, kMaxPlayerIcons, 0, false},
    // The packs of the player in the Soar sequence (compressed).
    {ArchiveId::kSoaringCharacter,
     u"sdmc:/sango-studio/player/soaring_model/index.bin",
     u"sdmc:/sango-studio/player/soaring_model/%d.bin", kSoaringModelToken, true,
     soaring_model_entries, 4, 0, false},
    // The pack of the introduction: one file.
    {ArchiveId::kIntroMovie, u"sdmc:/sango-studio/player/intro/index.bin",
     u"sdmc:/sango-studio/player/intro/%d.bin", kIntroToken, true,
     intro_entries, 2, 0, false},
    // The title screen. The pictures of the logo are layouts: the set of the
    // layouts of the same archive has them.
    {ArchiveId::kCutsceneData, u"sdmc:/sango-studio/title/data/index.bin",
     u"sdmc:/sango-studio/title/data/%d.bin", kTitleDataToken, true,
     title_data_entries, 16, 0, false},
    {ArchiveId::kCutsceneSequence, u"sdmc:/sango-studio/title/sequence/index.bin",
     u"sdmc:/sango-studio/title/sequence/%d.bin", kTitleSequenceToken, true,
     title_sequence_entries, 4, 0, false},
    // The texts of the game, one archive for each language.
    {ArchiveId::kGameTextJapanese, u"sdmc:/sango-studio/game_text/ja/index.bin",
     u"sdmc:/sango-studio/game_text/ja/%d.bin", kTextTokenFirst, true,
     text_entries[0], kMaxTextFiles, 0, false},
    {ArchiveId::kGameTextKanji, u"sdmc:/sango-studio/game_text/ja_kanji/index.bin",
     u"sdmc:/sango-studio/game_text/ja_kanji/%d.bin", kTextTokenFirst + 0x01000000,
     true, text_entries[1], kMaxTextFiles, 0, false},
    {ArchiveId::kGameTextEnglish, u"sdmc:/sango-studio/game_text/en/index.bin",
     u"sdmc:/sango-studio/game_text/en/%d.bin", kTextTokenFirst + 0x02000000,
     true, text_entries[2], kMaxTextFiles, 0, false},
    {ArchiveId::kGameTextFrench, u"sdmc:/sango-studio/game_text/fr/index.bin",
     u"sdmc:/sango-studio/game_text/fr/%d.bin", kTextTokenFirst + 0x03000000,
     true, text_entries[3], kMaxTextFiles, 0, false},
    {ArchiveId::kGameTextItalian, u"sdmc:/sango-studio/game_text/it/index.bin",
     u"sdmc:/sango-studio/game_text/it/%d.bin", kTextTokenFirst + 0x04000000,
     true, text_entries[4], kMaxTextFiles, 0, false},
    {ArchiveId::kGameTextGerman, u"sdmc:/sango-studio/game_text/de/index.bin",
     u"sdmc:/sango-studio/game_text/de/%d.bin", kTextTokenFirst + 0x05000000,
     true, text_entries[5], kMaxTextFiles, 0, false},
    {ArchiveId::kGameTextSpanish, u"sdmc:/sango-studio/game_text/es/index.bin",
     u"sdmc:/sango-studio/game_text/es/%d.bin", kTextTokenFirst + 0x06000000,
     true, text_entries[6], kMaxTextFiles, 0, false},
    {ArchiveId::kGameTextKorean, u"sdmc:/sango-studio/game_text/ko/index.bin",
     u"sdmc:/sango-studio/game_text/ko/%d.bin", kTextTokenFirst + 0x07000000,
     true, text_entries[7], kMaxTextFiles, 0, false},
    // The pieces of the battle backgrounds (the ground, the plate...): the files
    // have the bytes of the files of the archive. This set comes after the one of
    // the pieces of X and Y: a piece that Sango Studio replaces is found first.
    {ArchiveId::kBattleBackground, u"sdmc:/sango-studio/battle/background/index.bin",
     u"sdmc:/sango-studio/battle/background/%d.bin", kStudioBackgroundToken, true,
     studio_background_entries, 160, 0, false},
    // The layouts of the menus: one set for each archive that has some.
#include "core/data/layout_sets.inc"
};

static u32 OverrideSize(OverrideSet& set, u32 file_id);

// The set of an archive. When several sets are for the archive, the first one
// that replaces the file wins; else the first set of the archive.
static OverrideSet* FindSet(const u32* archive, u32 file_id) {
  OverrideSet* first = nullptr;
  for (OverrideSet& set : override_sets) {
    if (!Archive::IsArchive(archive, set.archive)) continue;
    if (first == nullptr) first = &set;
    if (OverrideSize(set, file_id) != 0) return &set;
  }
  return first;
}

// The set of a fake offset that has its token in the four high bits. A set with
// a small token (below 0x40000000) is never found here: a real offset of a large
// archive can have the same bits. The models of the species go up to 1 GB, so
// 0x30000000, the token of the trainer data, is also a real offset of their
// archive. A small token is valid only with its own archive (see ReadHook and
// FileStartAsyncReadHook).
static OverrideSet* FindSetByToken(u32 offset) {
  for (OverrideSet& set : override_sets) {
    if (IsSmallToken(set.token)) continue;
    if ((offset & kTokenMask) == set.token) return &set;
  }
  return nullptr;
}

// The threads of the game ask for files at the same time on a console. The
// index of a set must be read by one thread, and the other threads must wait
// for the end of the read: a thread that sees an index that is not read yet
// thinks that the file is not on the SD card, and the game reads its own
// archive at an offset that does not exist.
static LightLock override_index_lock = 1;

static void LoadOverrideIndex(OverrideSet& set) {
  if (__atomic_load_n(&set.loaded, __ATOMIC_ACQUIRE)) return;
  LightLock_Lock(&override_index_lock);
  if (!set.loaded) {
    // Try once: with no index file, the game calls this for each file it
    // reads, and a new try on the SD card each time makes the game freeze.
    sys::File file;
    if (OpenSd(&file, set.index_path)) {
      u32 count = 0;
      file.Read(&count, 4);
      if (count > set.capacity) count = set.capacity;
      file.Read(set.entries, count * sizeof(OverrideEntry));
      file.Close();
      set.count = count;
    }
    __atomic_store_n(&set.loaded, true, __ATOMIC_RELEASE);
  }
  LightLock_Unlock(&override_index_lock);
}

// Returns the size of the file of the SD card, or 0 when it is not there.
static u32 OverrideSize(OverrideSet& set, u32 file_id) {
  LoadOverrideIndex(set);
  u32 low = 0;
  u32 high = set.count;
  while (low < high) {
    const u32 mid = low + (high - low) / 2;
    if (set.entries[mid].file_id == file_id) return set.entries[mid].size;
    if (set.entries[mid].file_id < file_id) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }
  return 0;
}

static u32 LzStoreSize(u32 raw) { return 4 + raw + (raw + 7) / 8; }

static bool ItemDataIsRegistered(u32 file_id) {
  for (OverrideSet& set : override_sets) {
    if (set.archive != ArchiveId::kItemData) continue;
    for (u32 i = 0; i < set.count; i++) {
      if (set.entries[i].file_id == file_id) return true;
    }
  }
  return false;
}

// Writes a line "code file size" in sdmc:/sango-studio/overworld/model/trace.log. It shows
// how the game reads the files that the SD card replaces. The codes: 1 size
// of a file, 2 information of a file, 3 read in a thread, 4 read with the
// offset.
static LightLock trace_lock = 1;

static void WriteTrace(u32 what, u32 file_id, u32 size) {
  static sys::File file;
  static bool opened = false;
  static u32 lines = 0;
  if (lines >= 600) return;
  // Threads of the game write here at the same time on a console.
  LightLock_Lock(&trace_lock);
  if (lines < 600) {
    if (!opened) {
      opened = true;
      // The folders may not be there (a user who has no overworld models).
      sys::File::CreateDirectory(u"sdmc:/sango-studio");
      sys::File::CreateDirectory(u"sdmc:/sango-studio/overworld");
      sys::File::CreateDirectory(u"sdmc:/sango-studio/overworld/model");
      file.Open(u"sdmc:/sango-studio/overworld/model/trace.log");
    }
    if (file.IsOpen()) {
      lines++;
      file.WriteText(u"%u %u %u\n", what, file_id, size);
    }
  }
  LightLock_Unlock(&trace_lock);
}

static void Trace(OverrideSet* set, u32 what, u32 file_id, u32 size) {
  if (set == nullptr) return;
  // The sets of the item icons, of the item data and of the studio are in
  // memory: no log.
  if (set->token == kItemIconToken || set->archive == ArchiveId::kItemData ||
      set->token == kStudioToken) {
    return;
  }
  WriteTrace(what, file_id, size);
}

// A line for a file of the models that the species add (the numbers of these
// files are after the last file of the archive), whatever the set.
static void TraceId(u32 what, u32 file_id, u32 size) {
  if (file_id >= 8000 && file_id < 20000) WriteTrace(what, file_id, size);
}

static constexpr u32 kSpeciesTableFile = 0;

static void ServeLz(OverrideSet& set, u32 file_id, void* buffer, u32 size) {
  c16 path[64];
  Utils::Format(path, set.file_format, file_id);
  sys::File file;
  if (!OpenSd(&file, path)) {
    Trace(&set, 11, file_id, size);
    return;
  }
  u32 raw = 0;
  file.Read(&raw, 4);
  u8* out = (u8*)buffer;
  u8* end = out + size;
  const u32 header = 0x11 | (raw << 8);
  std::memcpy(out, &header, 4);
  out += 4;
  u8 chunk[512];
  u32 left = raw;
  while (left > 0 && out < end) {
    const u32 n = left < sizeof(chunk) ? left : sizeof(chunk);
    file.Read(chunk, n);
    left -= n;
    for (u32 k = 0; k < n && out < end; k += 8) {
      *out++ = 0;
      const u32 m = (n - k) < 8 ? (n - k) : 8;
      std::memcpy(out, chunk + k, m);
      out += m;
    }
  }
  file.Close();
  // The size of the data in the file and the bytes written to the buffer.
  Trace(&set, 12, raw, static_cast<u32>(out - static_cast<u8*>(buffer)));
}

static OverrideSet& GetItemIconSet() {
  for (OverrideSet& set : override_sets) {
    if (set.token == kItemIconToken) return set;
  }
  return override_sets[0];
}

// Writes the file of the icon of a new item as the game reads it: a stream
// of LZ11 without compression (a flag byte of 0, then 8 bytes).
static void ServeItemIcon(u32 file_id, void* buffer, u32 size) {
  OverrideSet& set = GetItemIconSet();
  const c16* path = nullptr;
  for (u32 i = 0; i < set.count; i++) {
    if (set.entries[i].file_id == file_id) path = item_icon_paths[i];
  }
  if (path == nullptr) return;

  static u8 raw[4096];
  const u32 length = renderer::TypeIcon::ReadFile(path, raw, sizeof(raw));
  if (length == 0) return;

  u8* out = static_cast<u8*>(buffer);
  u8* end = out + size;
  const u32 header = 0x11 | (length << 8);
  std::memcpy(out, &header, 4);
  out += 4;
  for (u32 k = 0; k < length && out < end; k += 8) {
    *out++ = 0;
    const u32 m = (length - k) < 8 ? (length - k) : 8;
    for (u32 j = 0; j < m && out < end; j++) *out++ = raw[k + j];
  }
}

static OverrideSet& GetItemDataSet() {
  for (OverrideSet& set : override_sets) {
    if (set.archive == ArchiveId::kItemData) return set;
  }
  return override_sets[0];
}

// Writes the data of a new item as the game reads it. The files of the item
// data have no compression: the game reads the 36 bytes as they are.
static void ServeItemData(u32 file_id, void* buffer, u32 size) {
  OverrideSet& set = GetItemDataSet();
  for (u32 i = 0; i < set.count; i++) {
    if (set.entries[i].file_id != file_id) continue;
    const u32 length = size < Archive::kItemDataFileSize
                           ? size
                           : Archive::kItemDataFileSize;
    std::memcpy(buffer, item_data_raw[i], length);
    return;
  }
}

// Copies a file of the SD card (a set with `whole_file`) to the buffer.
static void ServeFile(OverrideSet& set, u32 file_id, void* buffer, u32 size) {
  if (set.archive == ArchiveId::kItemData) {
    ServeItemData(file_id, buffer, size);
    return;
  }
  if (set.token == kItemIconToken) {
    ServeItemIcon(file_id, buffer, size);
    return;
  }
  if (set.token == kStudioToken) {
    sys::File file;
    if (OpenSd(&file, kStudioPokedexPath)) {
      file.Read(buffer, size);
      file.Close();
    }
    return;
  }
  c16 path[64];
  Utils::Format(path, set.file_format, file_id);
  sys::File file;
  if (!OpenSd(&file, path)) return;
  file.Read(buffer, size);
  file.Close();
  if (set.archive == ArchiveId::kBattleMoveChoreography) {
    battle::TrainerModelExtension::PatchPlayerPicture(file_id, buffer, size);
  }
}

static constexpr u32 kMaxRawSize = 0x4000;
static u8 raw_table[kMaxRawSize];
static u32 raw_size = 0;
static bool raw_loaded = false;

static void LoadRawTable() {
  if (raw_loaded) return;
  sys::File file;
  if (!OpenSd(&file, u"sdmc:/sango-studio/species/model/0.raw")) return;
  u32 size = 0;
  file.Read(&size, 4);
  if (size != 0 && size <= kMaxRawSize) {
    file.Read(raw_table, size);
    raw_size = size;
    raw_loaded = true;
  }
  file.Close();
}

static u32 RawSize() {
  LoadRawTable();
  return raw_size;
}

static bool ServeRaw(void* buffer, u32 size) {
  LoadRawTable();
  if (raw_size == 0) return false;
  if (size > raw_size) size = raw_size;
  std::memcpy(buffer, raw_table, size);
  return true;
}

static bool ItemDataIsRegistered(u32 file_id);

// The data of an item is the file of the archive of the item data that has the
// number of the item. A new item has no file: the game reads the file of the
// template item (see battle::ItemExtension). The data that the plugin changes
// (patch_data) is for the code that reads the item through ItemData.
static u32 RemapItemFile(const u32* archive, u32 file_id,
                         bool keep_registered = true) {
  if (file_id < battle::ItemExtension::kFirstId ||
      file_id > battle::ItemExtension::kLastId) {
    return file_id;
  }
  if (!Archive::IsArchive(archive, ArchiveId::kItemData)) return file_id;
  if (battle::ItemExtension::FindItem(static_cast<ItemId>(file_id)) ==
      nullptr) {
    return file_id;
  }
  // The plugin serves the data of an item that it has (the set of the archive).
  if (keep_registered && ItemDataIsRegistered(file_id)) return file_id;
  return battle::ItemExtension::kTemplateFile;
}

// The size of a file that the SD card replaces, as the game reads it. Returns
// 0 when the game must read its own file.
static u32 ReplacedSize(OverrideSet* set, u32 file_id) {
  if (set == nullptr) return 0;
  if (set->archive == ArchiveId::kPokemonModel &&
      file_id == kSpeciesTableFile && RawSize()) {
    return RawSize();
  }
  const u32 size = OverrideSize(*set, file_id);
  if (size == 0) return 0;
  return set->whole_file ? size : LzStoreSize(size);
}

// Writes a line "archive file" in sdmc:/sango/text_trace.log for the first
// reads of the files of the texts of the game (the archives 71 to 78): it shows
// which language the game reads, and which files.
static void TraceText(u32* archive, u32 file_id) {
  static sys::File file;
  static bool opened = false;
  static u32 lines = 0;
  static u32 seen[64][2];
  if (lines >= 64) return;
  for (u32 number = 71; number <= 78; number++) {
    if (!Archive::IsArchive(archive, static_cast<ArchiveId>(number))) continue;
    for (u32 i = 0; i < lines; i++) {
      if (seen[i][0] == number && seen[i][1] == file_id) return;
    }
    seen[lines][0] = number;
    seen[lines][1] = file_id;
    if (!opened) {
      opened = true;
      sys::File::CreateDirectory(u"sdmc:/sango");
      file.Open(u"sdmc:/sango/text_trace.log");
    }
    lines++;
    if (file.IsOpen()) file.WriteText(u"%u %u\n", number, file_id);
    return;
  }
}

u32 Archive::GetFileSizeHook(u32* archive, u32 file_id) {
  TraceId(30, file_id, 0);
  TraceText(archive, file_id);
  file_id = RemapItemFile(archive, file_id);
  file_id = ReplacedStreamFile(archive, file_id);
  if (IsArchive(archive, ArchiveId::kBattleMoveChoreography)) {
    const u32 patched = battle::MoveChoreographies::PatchedSize(archive, file_id);
    if (patched) return patched;
  }
  OverrideSet* set = FindSet(archive, file_id);
  const u32 replaced = ReplacedSize(set, file_id);
  if (replaced) {
    Trace(set, 1, file_id, replaced);
    return replaced;
  }
  return get_file_size_hook(archive, file_id);
}

u32 Archive::GetFileSizeHook2(u32* archive, u32 file_id) {
  TraceId(31, file_id, 0);
  TraceText(archive, file_id);
  file_id = RemapItemFile(archive, file_id);
  file_id = ReplacedStreamFile(archive, file_id);
  if (IsArchive(archive, ArchiveId::kBattleMoveChoreography)) {
    const u32 patched = battle::MoveChoreographies::PatchedSize(archive, file_id);
    if (patched) return patched;
  }
  OverrideSet* set = FindSet(archive, file_id);
  const u32 replaced = ReplacedSize(set, file_id);
  if (replaced) {
    Trace(set, 1, file_id, replaced);
    return replaced;
  }
  return get_file_size2_hook(archive, file_id);
}

void Archive::GetInfoHook(u32* archive, u32 file_id, u32* info) {
  TraceId(32, file_id, 0);
  TraceText(archive, file_id);
  file_id = RemapItemFile(archive, file_id);
  file_id = ReplacedStreamFile(archive, file_id);
  OverrideSet* set = FindSet(archive, file_id);
  const u32 replaced = info != nullptr ? ReplacedSize(set, file_id) : 0;
  if (replaced) {
    Trace(set, 2, file_id, replaced);
    info[0] = info[1] = replaced;
    info[2] = set->token | file_id;
    return;
  }
  get_info_hook(archive, file_id, info);
}

void Archive::ReadHook(u32* archive, u32 offset, u32 size,
                                void* buffer, u32* read) {
  if (offset >= 0x40000000) WriteTrace(37, offset, size);
  // The Pokédex data file of Sango Studio: a fake offset in the archive of the
  // Pokédex data (a real offset of this small archive never has this value).
  if (IsArchive(archive, ArchiveId::kPokedexData) &&
      (offset & kStudioMask) == kStudioToken) {
    for (OverrideSet& studio : override_sets) {
      if (studio.token != kStudioToken) continue;
      ServeFile(studio, offset & ~kStudioMask, buffer, size);
      if (read != nullptr) *read = size;
      return;
    }
  }
  // The small archives that Sango Studio replaces file by file (the icons of
  // the species, the moves, the evolutions, the experience tables, the texts).
  // The fake offset is valid only in the archive of the set.
  if (IsSmallToken(offset & kStudioMask)) {
    for (OverrideSet& small : override_sets) {
      if (small.token == kStudioToken || small.token != (offset & kStudioMask)) {
        continue;
      }
      if (!IsArchive(archive, small.archive)) continue;
      Trace(&small, 4, offset & ~kStudioMask, size);
      ServeFile(small, offset & ~kStudioMask, buffer, size);
      if (read != nullptr) *read = size;
      return;
    }
  }
  OverrideSet* set = FindSetByToken(offset);
  if (set != nullptr) {
    const u32 file_id = offset & kFileIdMask;
    if (set->archive == ArchiveId::kPokemonModel &&
        file_id == kSpeciesTableFile) {
      ServeRaw(buffer, size);
    } else if (set->whole_file) {
      Trace(set, 4, file_id, size);
      ServeFile(*set, file_id, buffer, size);
    } else {
      Trace(set, 6, file_id, size);
      ServeLz(*set, file_id, buffer, size);
    }
    if (read != nullptr) *read = size;
    return;
  }
  read_hook(archive, offset, size, buffer, read);
}

u32 Archive::LoadDataHook2(u32* archive, u32 file_id, void* buffer) {
  TraceId(33, file_id, 0);
  TraceText(archive, file_id);
  file_id = RemapItemFile(archive, file_id);
  file_id = ReplacedStreamFile(archive, file_id);
  if (IsArchive(archive, ArchiveId::kBattleMoveChoreography)) {
    u32 patched = 0;
    if (battle::MoveChoreographies::Serve(archive, file_id, buffer, &patched)) {
      return patched;
    }
  }
  OverrideSet* set = FindSet(archive, file_id);
  if (set != nullptr && set->whole_file) {
    // The game reads the whole file at once (a small file, for example).
    const u32 size = OverrideSize(*set, file_id);
    if (size) {
      Trace(set, 5, file_id, size);
      ServeFile(*set, file_id, buffer, size);
      return size;
    }
  }
  if (set != nullptr && !set->whole_file) {
    if (set->archive == ArchiveId::kPokemonModel &&
        file_id == kSpeciesTableFile && RawSize() &&
        ServeRaw(buffer, RawSize())) {
      return RawSize();
    }
    const u32 size = OverrideSize(*set, file_id);
    if (size) {
      Trace(set, 7, file_id, LzStoreSize(size));
      ServeLz(*set, file_id, buffer, LzStoreSize(size));
      return LzStoreSize(size);
    }
  }
  const u32 loaded = load_data2_hook(archive, file_id, buffer);
  if (IsArchive(archive, ArchiveId::kTrainerData)) {
    battle::TrainerClasses::Patch(file_id, buffer, loaded);
  } else if (IsArchive(archive, ArchiveId::kTrainerTypeData)) {
    battle::TrainerClasses::PatchRecord(file_id, buffer, loaded);
  }
  return loaded;
}

void* Archive::LoadCompressedHook(u32* archive, u32 file_id, void* heap_work,
                                  void* heap_data, s32 align, u32* out_size) {
  TraceId(34, file_id, 0);
  TraceText(archive, file_id);
  file_id = RemapItemFile(archive, file_id, false);
  file_id = ReplacedStreamFile(archive, file_id);
  OverrideSet* set = FindSet(archive, file_id);
  if (set != nullptr && !set->whole_file) {
    const u32 size = OverrideSize(*set, file_id);
    if (size) {
      Trace(set, 10, file_id, size);
      void* buffer = core::GameFunction<void*(void*, u32, u32)>::Call(
          sys::address::kHeapAlloc, heap_data, size, align ? align : 4);
      if (buffer != nullptr) {
        c16 path[64];
        Utils::Format(path, set->file_format, file_id);
        sys::File file;
        if (OpenSd(&file, path)) {
          u32 raw = 0;
          file.Read(&raw, 4);
          // The data of the file is the decompressed data: `raw` bytes.
          const u32 length = raw < size ? raw : size;
          file.Read(buffer, length);
          file.Close();
          if (length < size) {
            std::memset(static_cast<u8*>(buffer) + length, 0, size - length);
          }
          // The graphics chip and the other cores read this memory without
          // the cache of this core: the game does the same flush after it
          // decompresses a file. A console needs it, an emulator does not.
          svcFlushProcessDataCache(CUR_PROCESS_HANDLE,
                                   reinterpret_cast<u32>(buffer), size);
          if (out_size != nullptr) *out_size = length;
          return buffer;
        }
        core::GameFunction<void(void*)>::Call(sys::address::kHeapFree, buffer);
      }
    }
  }
  return load_compressed_hook(archive, file_id, heap_work, heap_data, align,
                              out_size);
}

// True when the text ends with `ending`.
static bool EndsWith(const c16* text, const c16* ending) {
  if (text == nullptr) return false;
  u32 length = 0;
  while (text[length] != 0) length++;
  u32 ending_length = 0;
  while (ending[ending_length] != 0) ending_length++;
  if (ending_length > length) return false;
  for (u32 i = 0; i < ending_length; i++) {
    if (text[length - ending_length + i] != ending[i]) return false;
  }
  return true;
}

// The game reads a part of an archive in a thread, with the offset that the
// archive gave. For a file of the SD card, the offset has the token of the set.
// The plugin copies the file to the buffer at once. The thread then reads four
// bytes of the real archive into a small buffer, so that the game sees the
// end of the read as usual.
static bool FileStartAsyncReadHook(void* file, void* heap, const c16* filename,
                                   u32 offset, u32 size, u32 size_again,
                                   void* buffer, u32 priority, void* callback,
                                   void* work) {
  OverrideSet* set = FindSetByToken(offset);
  if (offset >= 0x40000000) WriteTrace(38, offset, size);
  // The Pokédex data file of the studio: its fake offset is only valid in the
  // file of the Pokédex data (the id 132 is the file a/1/3/2).
  if (set == nullptr && (offset & kStudioMask) == kStudioToken &&
      EndsWith(filename, u"a/1/3/2")) {
    for (OverrideSet& studio : override_sets) {
      if (studio.token == kStudioToken) set = &studio;
    }
  }
  // The small archives of Sango Studio: the file of the archive is the last
  // part of the name that the game gives (for example "a/0/9/1").
  if (set == nullptr && IsSmallToken(offset & kStudioMask)) {
    for (OverrideSet& small : override_sets) {
      if (small.token == kStudioToken || small.token != (offset & kStudioMask)) {
        continue;
      }
      const u32 number = static_cast<u32>(small.archive);
      c16 name[16];
      name[0] = u'a';
      name[1] = u'/';
      name[2] = u'0' + (number / 100);
      name[3] = u'/';
      name[4] = u'0' + ((number / 10) % 10);
      name[5] = u'/';
      name[6] = u'0' + (number % 10);
      name[7] = 0;
      if (EndsWith(filename, name)) set = &small;
    }
  }
  if (set != nullptr && set->whole_file) {
    static u32 scratch[4];
    const u32 file_id = offset & (IsSmallToken(set->token) ? ~kStudioMask : kFileIdMask);
    Trace(set, 3, file_id, size);
    ServeFile(*set, file_id, buffer, size);
    return file_async_read_hook(file, heap, filename, 0, 4, 4, scratch,
                                priority, callback, work);
  }
  // A set that is not a whole file (the models of the species): the game must
  // not read its own archive at the fake offset. The plugin gives the data as
  // the synchronous read does, and the thread reads four real bytes.
  if (set != nullptr && !set->whole_file) {
    static u32 scratch_models[4];
    const u32 file_id = offset & kFileIdMask;
    if (set->archive == ArchiveId::kPokemonModel &&
        file_id == kSpeciesTableFile) {
      ServeRaw(buffer, size);
    } else {
      Trace(set, 8, file_id, size);
      ServeLz(*set, file_id, buffer, size);
    }
    return file_async_read_hook(file, heap, filename, 0, 4, 4, scratch_models,
                                priority, callback, work);
  }
  return file_async_read_hook(file, heap, filename, offset, size, size_again,
                              buffer, priority, callback, work);
}

u32 Archive::GetOriginalFileSize(u32* archive, u32 file_id) {
  return get_file_size_hook(archive, file_id);
}

u32 Archive::LoadOriginalData(u32* archive, u32 file_id, void* buffer) {
  return load_data2_hook(archive, file_id, buffer);
}

bool Archive::RegisterItemIcon(u32 file_id, const c16* path) {
  OverrideSet& set = GetItemIconSet();
  if (set.count >= kMaxItemIcons) return false;
  // The file must be a BCLIM image of the SD card.
  static u8 raw[4096];
  const u32 length = renderer::TypeIcon::ReadFile(path, raw, sizeof(raw));
  if (length == 0) return false;

  u32 i = 0;
  while (path[i] != 0 && i < SIZE(item_icon_paths[0]) - 1) {
    item_icon_paths[set.count][i] = path[i];
    i++;
  }
  item_icon_paths[set.count][i] = 0;
  set.entries[set.count].file_id = file_id;
  set.entries[set.count].size = LzStoreSize(length);
  set.count++;
  return true;
}

void Archive::SetPokedexDataFile(u32 size) {
  for (OverrideSet& set : override_sets) {
    if (set.token != kStudioToken) continue;
    set.entries[0].file_id = kPokedexDataFile;
    set.entries[0].size = size;
    set.count = size != 0 ? 1 : 0;
  }
}

bool Archive::RegisterItemData(u32 item_id, const u8* data) {
  OverrideSet& set = GetItemDataSet();
  // The data of an item that the plugin already gave: replace it.
  for (u32 i = 0; i < set.count; i++) {
    if (set.entries[i].file_id == item_id) {
      std::memcpy(item_data_raw[i], data, kItemDataFileSize);
      return true;
    }
  }
  if (set.count >= kMaxItemData) return false;
  // The list stays sorted by the id of the item.
  u32 place = set.count;
  while (place > 0 && set.entries[place - 1].file_id > item_id) {
    set.entries[place] = set.entries[place - 1];
    std::memcpy(item_data_raw[place], item_data_raw[place - 1],
                kItemDataFileSize);
    place--;
  }
  set.entries[place].file_id = item_id;
  set.entries[place].size = kItemDataFileSize;
  std::memcpy(item_data_raw[place], data, kItemDataFileSize);
  set.count++;
  return true;
}

bool Archive::HasSdFile(ArchiveId archive_id, u32 file_id) {
  for (OverrideSet& set : override_sets) {
    if (set.archive == archive_id) return OverrideSize(set, file_id) != 0;
  }
  return false;
}

bool Archive::IsArchive(const u32* archive_data,
                             const ArchiveId archive_id) {
  u32* archive_table = (u32*)sys::address::kArchiveFilenameTable;
  return archive_data[12] == archive_table[static_cast<u32>(archive_id)];
}

bool Archive::IsArchive(const ArchiveInput* input, const ArchiveId archive_id) {
  return input->archive_id == archive_id;
}

bool Archive::ReadFileAsync2(u32* archive, void* heap, u32 file_id,
                                  void* buffer,
                                  u32 p4, u32 p5, u32 p6) {
  TraceId(35, file_id, 0);
  auto& feat = GetInstance();
  file_id = RemapItemFile(archive, file_id);
  file_id = ReplacedStreamFile(archive, file_id);
  if (feat.on_stream_file != nullptr) {
    file_id = feat.on_stream_file(archive, file_id);
  }
  return read_file_async2_hook(archive, heap, file_id, buffer, p4, p5, p6);
}

bool Archive::ReadFileAsync(void* file_manager, ArchiveInput* input) {
  TraceId(36, input->file_id, 0);
  auto& feat = GetInstance();
  battle::EffectStyles::TrackRead(input);
  input->file_id = ReplacedQueuedFile(input, input->file_id);
  if (feat.on_read_file != nullptr) feat.on_read_file(input);
  return read_file_async_hook(file_manager, input);
}

bool Archive::ReplaceFile(ArchiveId archive_id, u32 from, u32 to) {
  auto& feat = GetInstance();
  Replacement* same = feat.replacements_.Find([&](const Replacement& entry) {
    return entry.archive_id == archive_id && entry.from == from;
  });
  if (same != nullptr) {
    same->to = to;
    return true;
  }
  if (feat.replacements_.IsFull()) {
    Problems::Add(u"archive %u: more than %u replaced files",
                  static_cast<u32>(archive_id), kMaxReplacements);
    return false;
  }
  feat.replacements_.Add({archive_id, from, to});
  return true;
}

void Archive::ClearReplacements() { GetInstance().replacements_.Clear(); }

void Archive::RemoveReplacement(ArchiveId archive_id, u32 from) {
  GetInstance().replacements_.RemoveIf([&](const Replacement& entry) {
    return entry.archive_id == archive_id && entry.from == from;
  });
}

u32 Archive::ReplacedStreamFile(const u32* archive, u32 file_id) {
  const auto& feat = GetInstance();
  for (const Replacement& entry : feat.replacements_) {
    if (entry.from == file_id && IsArchive(archive, entry.archive_id)) {
      return entry.to;
    }
  }
  return file_id;
}

u32 Archive::ReplacedQueuedFile(const ArchiveInput* input, u32 file_id) {
  const auto& feat = GetInstance();
  for (const Replacement& entry : feat.replacements_) {
    if (entry.from == file_id && IsArchive(input, entry.archive_id)) {
      return entry.to;
    }
  }
  return file_id;
}

} // namespace core
