<div align="center">

# Sango Plugin (v4.1.0) | A new CTRPF for Pokémon ORAS

![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)
![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)
![Platform](https://img.shields.io/badge/platform-3DS%20%2B%20Citra%20%2B%20Azahar-red.svg)

<div style="display: flex; justify-content: space-between; gap: 10px;">
  <img src="assets/sango_plugin_v4.jpg">
  <img src="assets/battle.png">
  <img src="assets/camera_menu.jpg">
  <img src="assets/overworld_cheat_code.jpg">
</div>

</div>

---

## Compatibility

All reverse engineering and features are built **exclusively for Pokémon Alpha Sapphire v1.4**.

---

## Description

**Sango Plugin** is a C++ framework designed to interact directly with the game engine.

**Update v2.0.0:** Due to size constraints, the project is no longer injected as an Action Replay cheat code. It has
been fully migrated to a standard **`.3gx` plugin format**.

---

## Pokémon Sango Kaizo (ROM Hack)

Check out my flagship mod built using this framework: **[Pokémon Sango Kaizo (Demo)](kaizo.md)** - a complete overhaul featuring hardcoded Nuzlocke rules, live stat editing, competitive AI, and custom QoL features!

---

## Build & Usage

**To compile:**
You will need **devkitPro**, as well as the following resources provided by the **thepixellizeross** team:

* **[libctrpf](https://gitlab.com/thepixellizeross/ctrpluginframework/-/releases)**
* **[3gxtool](https://gitlab.com/thepixellizeross/3gxtool/-/releases/v1.2)**

**To use:**
Put the `.3gx` you built (`sango_plugin.3gx`, `sango_kaizo.3gx` or `sango_undertow.3gx`) in the `luma/plugins/000400000011C500` folder, renamed to `sango_plugin.3gx`.

---

## Repository Structure

One library, several plugins built on it — `make` builds them all,
`make overlay`, `make kaizo` or `make undertow` just one.

* `lib/`: the engine every plugin shares — memory addresses, game structures,
  features (hooks), the menu framework and its pages, C++ field scripts. It
  never decides anything on its own: a plugin enables features and installs
  callbacks (`plugin.h`).

  The library is packaged by domain, one namespace per directory: `core/`
  (engine, processes, hooks), `system/` (`sys::`: device, files, graphics,
  sound), `battle/`, `overworld/`, `pokemon/`, `savedata/`, `renderer/`,
  `script/` and `ui/`. Every domain follows the same layout:

  * `<domain>/address.h`: the game's addresses, each declared for both games
    as `GAME_ADDRESS(xy, oras)`; the Makefile picks one with `-DGAME_ORAS`
    (or `-DGAME_XY` - those addresses are still to be found, they read 0).
  * `<domain>/constant/`: the enum classes, one per file. Ids of the game's
    tables end in `Id` (`SpeciesId`, `ItemId`, `MapId`...), closed
    vocabularies don't (`Nature`, `Gender`, `Format`, `StatusCondition`...).
  * `<domain>/native/`: the game's own structures, laid out as in memory,
    one per file (`battle::Config`, `core::GameManager`,
    `savedata::PokemonTeam`...). Only the families that only make sense
    together share a file: the battle mutations, the PSS payloads, the
    GARC sections and the AMX structures. Every field that is an id uses
    the matching enum class, so plugin code never juggles raw integers.
  * `<domain>/patch/`: what the plugin adds on top - hooks, patches and
    their options (`battle::Battle`, `overworld::Camera`,
    `pokemon::CustomShop`...). Options that survive a restart live in a
    `*Settings` base struct that `ConfigManager` saves and loads; pure hooks
    without options are suffixed `Patch`.

  `ui/` is the plugin's own menu framework, `ui/native/` the game's layout
  manager and `ui/patch/` the patches of the game's screens.
* `overlay/`: `sango_plugin.3gx`, every page of the library under one menu, for
  exploring and editing the game while it runs.
* `kaizo/`: `sango_kaizo.3gx`, the ROM hack.
* `undertow/`: `sango_undertow.3gx`, Pokémon Undertow: the game played from
  Team Aqua's side, as a grunt.

---

## Author

* **David Darras** (ZettaD)