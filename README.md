<div align="center">

# Sango Plugin (v2.0.0) | A new CTRPF for Pokémon ORAS

![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)
![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)
![Platform](https://img.shields.io/badge/platform-3DS%20%2B%20Citra%20%2B%20Azahar-red.svg)

<div style="display: flex; justify-content: space-between; gap: 10px;">
  <img src="assets/battle.png">
  <img src="assets/camera_menu.jpg">
  <img src="assets/overworld_cheat_code.jpg">
  <img src="assets/pokemon_editor.jpg">
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

## Build & Usage

**To compile:**
You will need **devkitPro**, as well as the following resources provided by the **thepixellizeross** team:

* **[libctrpf](https://gitlab.com/thepixellizeross/ctrpluginframework/-/releases)**
* **[3gxtool](https://gitlab.com/thepixellizeross/3gxtool/-/releases/v1.2)**

**To use:**
Put the `sango_plugin.3gx` file in the `luma/plugins/000400000011C500` folder.

---

## Repository Structure

* `include/`: Header files (Memory addresses, data structures, and game classes).
* `src/`: Menu implementation and hooks.

---

## Author

* **David Darras** (ZettaD)