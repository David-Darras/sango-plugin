# Game structures

A **game structure** is a C++ structure with the same memory layout as a
structure of the game. With it, the plugin reads and changes the data of the
game with normal C++ code.

The game structures are in the `native/` folder of each domain.

## An example

```cpp
// lib/include/savedata/native/pokemon_team.h
struct PokemonTeam {
  SINGLETON(PokemonTeam)
  STATIC_INLINE PokemonTeam& GetInstance() {
    return core::DataManager::GetInstance().GetPokemonTeam();
  }

  static constexpr u32 kMaxSlots = 6;

  PokemonParam* pokemons[kMaxSlots];  // offset 0x00: 6 pointers
  u8 count;                           // offset 0x18: number of Pokémon
  u8 _0[3];                           // offset 0x19: unknown bytes
};
```

- The members are in the same order as in memory.
- Each member has the same size as in memory.
- `GetInstance()` returns the object of the game. The plugin does not make a copy:
  when you change a member, you change the game.

Use it like this:

```cpp
auto& team = savedata::PokemonTeam::GetInstance();
for (u32 i = 0; i < team.count; i++) {
  // Use team.pokemons[i].
}
```

## The rules

### The size of each member is exact

A wrong size moves all the next members. Then the plugin reads wrong values.

- Use the types of `core/types.h`: `u8`, `u16`, `u32`, `s8`, `s16`, `s32`, `f32`, `c16`.
- Do not use `int`, `long` or `bool` for a member if you do not know their size in the game.

### Unknown bytes have the names `_0`, `_1`...

When nobody knows the meaning of some bytes, the structure keeps them as an
array: `u8 _0[3];`. When you find their meaning, give them a name.

### The sizes are checked

A `static_assert` checks the size of a structure or the offset of a member:

```cpp
static_assert(sizeof(PokemonTeam) == 0x1C,
              "PokemonTeam must have the size of the game structure");
static_assert(offsetof(PokemonTeam, count) == 0x18,
              "count must be at offset 0x18");
```

If you change a structure and the size is wrong, the build stops.
This protects you from a crash in the game.
Add a `static_assert` when you know the size of a structure.

### Ids use enum classes

A member that contains an id uses the enum class of this id, with the exact width:

```cpp
SpeciesId species;   // enum class SpeciesId : u16
FormId form : 5;     // a bitfield of 5 bits
```

You do not need a `static_cast` when you use the member:

```cpp
if (pokemon.species == SpeciesId::kPikachu) { ... }
```

The enum classes are in the `constant/` folder of each domain.
The ids of the game tables end with `Id` (`SpeciesId`, `ItemId`, `MapId`...).
The closed lists do not (`Nature`, `Gender`, `Format`...).

## Call a function of the game

A game structure can call a function of the game with
`core::GameFunction<signature>::Call(address, arguments...)`
(`core/game_function.h`). The signature has the same form as for
`core::Hook`:

```cpp
INLINE void HealAllPokemons() {
  core::GameFunction<void(PokemonTeam*)>::Call(pokemon::address::kHealTeam,
                                              this);
}
```

Read it like this: "`kHealTeam` is a function that receives a `PokemonTeam*`
and returns nothing. Call it with `this`."

## Encrypted Pokémon data

The game encrypts the data of each Pokémon (`pokemon::CoreData`), and the
calculated data of the party (`pokemon::RuntimeData`: level, HP, stats).
Use `pokemon/edit_pokemon.h`:

```cpp
savedata::PokemonParam* first = team.Get(0);  // Null when the party is empty.
if (first == nullptr) return;

// Change: decrypted while `edit` lives, encrypted at the end of the block.
{
  pokemon::EditPokemon edit(*first);
  edit->moves[0] = MoveId::kThunderbolt;
} // Encrypts the data, then calculates the stats again.

// Read: a decrypted copy. The Pokémon does not change.
const pokemon::CoreData copy = pokemon::ReadPokemon(*first->core);
```

`EditPokemon` also changes a Pokémon of a box: give its `CoreData`.
Give `false` as the second parameter when the game must not calculate the stats
again (a Pokémon of a battle, or data that you restore after).

Do not call `accessor->Decrypt()` two times without `Encrypt()`: the data is
then broken, and the Pokémon becomes a Bad Egg. `EditPokemon` does the two
calls for you.

## Add a game structure

1. Make the file `lib/include/<domain>/native/<name>.h`.
2. Put the structure in the namespace of the domain.
3. Use the enum classes of `constant/` for the ids.
4. Add a `static_assert` for the size.
5. Add Doxygen comments: what the structure is, and what each known member is.

See [the writing rules](../reference/writing-rules.md) for the style.
