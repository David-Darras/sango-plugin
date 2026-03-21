<div align="center">

# Sango Plugin

![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)
![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)
![Platform](https://img.shields.io/badge/platform-Old%2FNew%202D--3DS%20%2B%20Citra-red.svg)
![Status](https://img.shields.io/badge/status-Work%20In%20Progress-yellow.svg)

![Pokémon Alpha Sapphire](https://upload.wikimedia.org/wikipedia/en/f/f1/AlphaSapphire.jpg)

![Sango Plugin](assets/sango-plugin-v1.0.0.jpg)

</div>

---

## ⚠️ Important Compatibility Note

All **reverse engineering** for this project was performed exclusively on **Pokémon Alpha Sapphire v1.4**.

---

## Description

**Sango Plugin** is a C++ framework (currently under development) designed to interact directly with the game engine.

---

## 🔧 Dependencies & Build

This plugin is designed to be **fully standalone**:

* ❌ Does **not rely on libraries** such as `libctrpf` or `libctru`

Instead, the plugin **relies entirely on the game's own code and internal libraries**.  
All core operations (file handling, rendering, input, etc.) are performed by directly calling **game functions**, not
external SDK libraries.

However, to compile the project, you will still need:

* ✅ **devkitPro** (required toolchain for Nintendo 3DS development)

### Build Pipeline

Once compiled, the resulting `.elf` file must be converted into an **Action Replay cheat code**.

For this purpose, a dedicated tool is required:

➡️ **elf2arcc**  
https://github.com/David-Darras/elf2arcc

Use the generated code with Rosalina or Citra.

---

## 🚧 Project Status: Work In Progress

This repository currently contains the **technical foundations** and the plugin infrastructure.

Major features are planned for future updates, including:

* **Graphics Modification:** Real-time texture and model adjustments.
* **Battle Manipulation:** Real-time management of combat data.
* **Savedata Edition:** Modify in-game data (bag inventory, Pokémon, etc.)
* **Field Management:** Environment and map data handling.

---

## Core Features (Currently Implemented)

Despite being in early development, the following foundations are fully functional:

* **Comprehensive Menu System:** Touch-based UI including buttons, a Unicode (UTF-16) keyboard, and a numeric keypad.
* **System Wrappers:**
    * `Device` & `Controller`: Hardware input management.
    * `Graphics`: Basic 2D rendering, primitives, and text display.
    * `Sound`: Playback control for music, SFX, and Pokémon cries.
    * `File`: I/O operations on the SD card.

---

## Repository Structure

* `include/`: Header files (Class definitions, memory addresses, and data structures).
* `src/`: Core implementation (Menu logic, hook applications, and the `crt0.s` entry point).

---

## Author

* **David Darras** (ZettaD)