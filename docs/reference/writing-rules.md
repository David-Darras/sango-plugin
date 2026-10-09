# Writing rules

This page gives the rules for the code, the comments and the documentation.
Follow them when you change the project.

## The language: Simplified Technical English

The comments and the documentation use the rules of
[ASD-STE100 Simplified Technical English](https://www.asd-ste100.org).
Many readers of this project do not speak English as a first language.
Simple sentences help them, and translation tools work better.

| Rule | Example |
|---|---|
| Write short sentences: 20 words at most in a procedure, 25 words at most in a description. | |
| Write one idea in each sentence. | |
| Use the active voice. | "The hook replaces the function." Not: "The function is replaced by the hook." |
| Use the present tense. | "The game calls this function." Not: "The game will call this function." |
| Use the imperative in a procedure. | "Add the address to `address.h`." |
| Keep the articles (*a*, *an*, *the*). | "Install the hook." Not: "Install hook." |
| Use one word for one meaning. Use the terms of the [glossary](glossary.md). | Always "overworld", never "field" for the walking part of the game. |
| Do not use idioms or slang. | Not: "under the hood", "out of the box". |
| Do not use the names of the internal game files or of the developers. | Use the names of [Bulbapedia](https://bulbapedia.bulbagarden.net) and of the community tools. |

> **The names of the game:** use the Pokémon names of Bulbapedia:
> "Pokémon-Amie", "PokéNav Plus", "Super Training", "Secret Base",
> "Soaring". The names of the natives of the scripts (`TalkMdlMsg_Seq`...)
> are names of the game data: keep them.

## The code

The project uses a style near the
[Google C++ style](https://google.github.io/styleguide/cppguide.html).

| Item | Style | Example |
|---|---|---|
| Types and functions | `PascalCase` | `MoveAnimation`, `GetInstance()` |
| Variables and members | `snake_case` | `trainer_id`, `is_enabled` |
| Private members | `snake_case_` | `count_` |
| Constants and enum values | `kPascalCase` | `kMaxEntries`, `SpeciesId::kPikachu` |
| Indentation | 2 spaces | |
| Line length | 80 columns | |
| Files | English, CRLF line ends | |

### Language limits

The plugin runs inside the memory of the game. It must stay small.

- C++11 (`-std=gnu++11`). Do not use C++14 or later.
- No exceptions (`throw`, `try`).
- No RTTI (`dynamic_cast`, `typeid`).
- No `std::string`, no `std::vector`, no `new` in a loop. Use fixed arrays.

### Structure

- **Game structures** are in `<domain>/native/`, one structure in each file.
  Unknown bytes are `_0`, `_1`... Ids use the enum classes. See [Game structures](../concepts/game-structures.md).
- **Enum classes** are in `<domain>/constant/`, one enum in each file.
- **Features** are in `<domain>/patch/`. A feature is a singleton.
  Its settings are in a `...Settings` structure. Its callbacks start with `on_`.
- **Addresses** are in `<domain>/address.h`, with `GAME_ADDRESS(xy, oras)`.
- **Hook ids** are in `core/constant/hook_id.h`.
- **Pages** are in `lib/src/ui/page/`, declared in `ui/page/pages.h`.
- **Products** never change `lib/`. They use the settings, the callbacks and the `Add`/`Register` functions.

## The Doxygen comments

Each header has Doxygen comments. Doxygen makes the API reference from them
(see [Set up your editor](../getting-started/04-set-up-your-editor.md#the-api-reference-doxygen)).

### A file

```cpp
/**
 * @file battle.h
 * @brief Changes the battles: animations, capture rules, health bars.
 *
 * Add more sentences when the file needs an explanation.
 */
```

### A class or a structure

```cpp
/**
 * @brief Adds new moves and new abilities to the game.
 *
 * Your product registers its moves with AddMove().
 */
class GameExtension {
```

### A function

```cpp
/**
 * @brief Adds a new move.
 * @param spec The description of the move.
 * @return false when the list is full.
 */
static bool AddMove(const MoveSpec& spec);
```

For a short function, one line is enough:

```cpp
/// Returns the number of Pokémon in the party.
u8 GetCount() const;
```

### A member or an enum value

Put the comment at the end of the line with `///<`:

```cpp
u8 level; ///< The level, from 1 to 100.
SpeciesId species; ///< The species.
```

When the line becomes longer than 80 columns, put the comment on the line
before, with `///`:

```cpp
/// Checks if a delayed move (like Future Sight) is ready.
kCheckDelayedMoveReady = 6,
```

Never put a `///<` comment alone on the next line.

### A comment inside a function

Inside a function (in a `.cc` file), use `//` and full sentences:

```cpp
// The data of a Pokemon is encrypted: decrypt it, read it, encrypt it.
pokemon->accessor->Decrypt();
```

A short title of a group of lines is also a `//` comment: `// Hooks.`

### The same format everywhere

| Where | Format |
|---|---|
| The top of a file | `/** @file ... @brief ... */` after the license. |
| A type, a function, a constant | `///` lines before it. `/** @brief ... */` when it has `@param`, `@return`, `@code` or several paragraphs. |
| A member, an enum value | `///<` at the end of the line, or `///` before it when the line is too long. |
| The code of a function | `//` full sentences. |
| A list of values (`// 0x2D`, `// 12`) | A number or a name only. These labels stay as they are. |
| The end of a namespace | `} // namespace battle` |

Do not keep code in comments, separator lines (`// ------`), `TODO` notes or
addresses that nobody uses.

### What to write

- Write what the code does and why. Do not repeat the name.
  - Good: `/// Returns true when the player can catch the Pokémon.`
  - Bad: `/// Is capture allowed.`
- Give the unit of a value: frames, percent, bytes, tiles.
- Give the special values: "0 = none", "101 = never misses".
- Write the limits: "16 at most".
- Do not write the history of the code. Git keeps it.
