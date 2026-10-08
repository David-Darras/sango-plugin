# Glossary

This page defines the terms of the documentation and of the code.
Each term has one meaning only. The documentation always uses these terms.

The Pokémon terms come from [Bulbapedia](https://bulbapedia.bulbagarden.net).
When you do not know a game term, search it on Bulbapedia.

## Project terms

| Term | Meaning |
|---|---|
| **Plugin** | A `.3gx` file. Luma3DS (on a console) or Azahar (on a computer) loads it with the game. The plugin runs inside the memory of the game. |
| **Library** | The shared code in `lib/`. It contains all the features and all the game structures. It does nothing alone. |
| **Product** | One plugin that uses the library: `overlay`, `kaizo`, `undertow`, or your own ROM hack. Each product has its own folder. |
| **ROM hack** | A modified version of a game. With this project, a ROM hack is a product. The game files on the cartridge do not change. |
| **Overlay** | The product that shows all the pages of the library in one menu. Use it to explore and test the game. |
| **Feature** | A class of the library that changes one part of the game. Examples: `battle::Battle`, `overworld::Camera`. A feature is a singleton. |
| **Singleton** | A class with one object only. You get the object with `GetInstance()`. |
| **Setting** | A variable of a feature that the player can change in the menu. The settings are in a `...Settings` structure. The plugin does not save them. |
| **Callback** | A function of your product that a feature calls. The callbacks have names that start with `on_` (for example `on_wild_pokemon`). |
| **Entry point** | The `Initialize()` function of a product, in `<product>/src/entrypoint.cc`. The plugin calls it when the game starts. |
| **Frame** | One image of the game. The game shows 30 or 60 frames each second. The plugin runs code one time for each frame. |
| **Menu** | The menu of the plugin. It is on the top screen. Press **Start** to open it or to close it. |
| **Page** | One list of entries in the menu. A page is a function `void LoadXxxPage(ui::MainApplication& app, void* args)`. |
| **Entry** | One line of a page. An entry shows a value, opens a page or runs a function. |
| **Log** | A text window of the plugin. Press **L + R** to show it or to hide it. `ui::LogApplication::Print()` writes to it. |

## Technical terms

| Term | Meaning |
|---|---|
| **Address** | The location of a function or of a variable in the memory of the game. The addresses are in the `address.h` file of each domain. |
| **`GAME_ADDRESS(xy, oras)`** | A macro that selects the address for the game of the build. The first value is for X, the second value is for Alpha Sapphire. The value `0` means "not found yet". |
| **Hook** | A redirection. When the game calls a function, the plugin function runs instead. |
| **Original function** | The game function that a hook replaces. `core::HookManager::Call()` calls it. |
| **Hook id** | The name of a hook in `core::HookId`. Each hook has one id. |
| **Patch** | A change to the code or to the data of the game in memory. A hook is one type of patch. |
| **Game structure** | A C++ structure with the same memory layout as a structure of the game. The game structures are in the `native/` folders. Also called a *native structure*. |
| **Offset** | The distance in bytes from the start of a structure to one of its members. |
| **Domain** | One folder of the library: `core`, `system`, `battle`, `overworld`, `pokemon`, `savedata`, `renderer`, `script`, `ui`, `net`. Each domain is also a C++ namespace. |
| **Process** | One state of the game: the title screen, the overworld, a battle, the summary screen... The game runs one main process at a time. A process has a *vtable* address that identifies it. |
| **CRO** | A module of game code that the game loads only when it needs it. The battle code is a CRO. A hook in a CRO is enabled when its CRO is loaded. |
| **Archive** | A GARC file of the game. It contains many files (models, texts, data). An archive has an id (`core::ArchiveId`). |
| **Script** | A program of the game that controls an event: a conversation, a cutscene, a gift. The game scripts use the Pawn language (`.amx` files). |
| **Native** | A C function of the game that a script can call. Example: `ItemAdd`. |
| **C++ script** | A script that you write in C++ with `script::Context`. The game runs it like a game script. |

## Pokémon terms

| Term | Meaning |
|---|---|
| **Overworld** | The part of the game where the player walks: routes, towns, buildings, caves. |
| **Map** | One place of the overworld: a route, a town, a building or a floor. A map has an id (`MapId`). The game data and the code also call a map a *zone*. |
| **Tile** | One square of the ground of a map. The positions of the characters are in tiles. |
| **Character** | A person or a Pokémon that stands in the overworld. The player is also a character. |
| **Model** | The 3D object of a character, of a Pokémon or of a prop. A model has an id (`ModelId`). |
| **Species** | A kind of Pokémon, for example Pikachu. A species has an id (`SpeciesId`) that is equal to its National Pokédex number. |
| **Form** | A variant of a species, for example Alolan Raichu or Mega Charizard X. A form has an id (`FormId`). |
| **Move** | An attack or a status move that a Pokémon uses in battle. A move has an id (`MoveId`). |
| **Ability** | A passive power of a Pokémon, for example Intimidate. An ability has an id (`AbilityId`). |
| **Item** | An object of the Bag. An item has an id (`ItemId`). |
| **Held item** | The item that a Pokémon holds. |
| **Nature** | A personality value that changes the stats of a Pokémon. |
| **Type** | An element of a Pokémon or of a move, for example Fire or Water. |
| **Stats** | HP, Attack, Defense, Sp. Atk, Sp. Def and Speed. |
| **Base stats** | The stats of a species before IVs, EVs, level and nature. |
| **IV** | Individual value. A value from 0 to 31 for each stat. |
| **EV** | Effort value. A value from 0 to 252 for each stat. |
| **Learnset** | The moves that a species learns when its level increases. |
| **Evolution** | The change of a Pokémon into a different species. |
| **Mega Evolution** | A change of form during a battle, with a Mega Stone. |
| **Shiny Pokémon** | A Pokémon with different colors. |
| **Party** | The Pokémon that the player carries (6 at most). |
| **PC box** | A storage box for the Pokémon that are not in the party. |
| **Bag** | The item storage of the player. A Bag has pockets. |
| **Trainer** | A character that battles the player. A trainer has an id (`TrainerId`). |
| **Trainer class** | The title of a trainer, for example "Youngster". |
| **Wild Pokémon** | A Pokémon that does not belong to a trainer. |
| **Encounter table** | The list of wild Pokémon that can appear on a map. |
| **Static encounter** | A wild Pokémon that stands in the overworld, for example Kyogre. |
| **Gift Pokémon** | A Pokémon that a character gives to the player. |
| **In-game trade** | A trade with a character of the game. |
| **Field move** | A move that a Pokémon uses in the overworld, for example Surf or Cut. |
| **Hidden item** | An item that is invisible on the ground. |
| **Tall grass** | Grass where wild Pokémon appear. |
| **Weather** | Rain, sunlight, sandstorm, hail... in the overworld or in battle. |
| **Terrain** | A battle condition on the ground, for example Electric Terrain. |
| **Status condition** | A condition of a Pokémon, for example burn, freeze or poison. |
| **Battlefield** | The place of a battle. In the code, *field* has this meaning only in the battle domain. |
| **Side** | One half of the battlefield: the side of the player or the side of the opponent. |
| **Day Care** | The place where Pokémon gain levels and lay Eggs. |
| **Poké Mart** | A shop that sells items. |
| **Secret Base** | A place that the player decorates in Omega Ruby and Alpha Sapphire. |
| **Pokémon-Amie** | The feature where the player plays with a Pokémon on the bottom screen. |
| **Super Training** | The feature where the player trains the EVs of a Pokémon. |
| **PSS** | The Player Search System: the online menu on the bottom screen. |
| **O-Power** | A temporary boost that the player activates from the PSS. |
| **PokéNav Plus** | The bottom screen of Omega Ruby and Alpha Sapphire. Its apps are PlayNav, BuzzNav, AreaNav and DexNav. |
| **DexNav** | The app of the PokéNav Plus that finds hidden wild Pokémon. |
| **Soaring** | Flight over Hoenn on Latios or Latias. |

## Names in the code

| You see | It means |
|---|---|
| `kName` | A constant. |
| `PascalCase` | A type or a function. |
| `snake_case` | A variable or a member. |
| `..._` | A private member of a class. |
| `_0`, `_1`... | Bytes of a game structure that nobody understands yet. |
| `...Hook` | A function that replaces a game function. |
| `...Settings` | The settings of a feature. |
| `on_...` | A callback. |
