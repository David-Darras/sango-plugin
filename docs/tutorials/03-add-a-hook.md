# Tutorial 3: Add a hook

In this tutorial, you replace a function of the game with your own function.
The game calls `AddPokemonToTeam` when a Pokémon joins a party.
Your hook writes the species to the log. It also stops Wurmple: Wurmple cannot
join the party of the player.

**Time:** 15 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md).
**Read first:** [Hooks and addresses](../concepts/hooks-and-addresses.md).

## Step 1: Find the address and the signature

The address is in `lib/include/pokemon/address.h`:

```cpp
constexpr uptr kAddPokemonToTeam = GAME_ADDRESS(0x0039FA78, 0x003B6754);
```

The signature of the game function is:

```cpp
bool AddPokemonToTeam(savedata::PokemonTeam* team, savedata::PokemonParam* pokemon);
```

- `team` is the party that receives the Pokémon.
- `pokemon` is the new Pokémon.
- The function returns `true` when the Pokémon joins the party.

> **How do you know the signature?** Somebody studied the code of the game
> (reverse engineering). For the addresses of the library, look at the code
> that already uses them. Here, `kaizo/src/kaizo_gift.cc` uses the same hook.

## Step 2: Write the hook

Make the file `myhack/src/my_hooks.cc`:

```cpp
/**
 * @file my_hooks.cc
 * @brief The hooks of my ROM hack.
 */

#include "common.h"
#include "core/hook.h"
#include "pokemon/address.h"
#include "savedata/native/pokemon_team.h"
#include "ui/log_application.h"

namespace myhack {

// The return type, the name of the hook, the parameters of the game
// function, then its address.
HOOK(bool, AddPokemonToTeam,
     (savedata::PokemonTeam* team, savedata::PokemonParam* pokemon),
     pokemon::address::kAddPokemonToTeam) {
  // The data of a Pokemon is encrypted: decrypt it, read it, encrypt it.
  pokemon->accessor->Decrypt();
  const SpeciesId species = pokemon->core->species;
  pokemon->accessor->Encrypt();

  ui::LogApplication::Print(u"New Pokemon: species %u",
                            static_cast<u32>(species));

  // Stop Wurmple, but only for the party of the player.
  const bool is_player_party = team == &savedata::PokemonTeam::GetInstance();
  if (is_player_party && species == SpeciesId::kWurmple) {
    return false;
  }

  // Run the original function of the game.
  return original(team, pokemon);
}

} // namespace myhack
```

You do not change other files, not even `entrypoint.cc`:
`plugin::InitializeEngine()` installs all the hooks of `HOOK()`.

The compiler checks `original(team, pokemon)`: the arguments must have the
types of the parameters. The name of the hook (`AddPokemonToTeam`) must be
unique in the namespace.

## Step 3: Build and test

```bash
make myhack
```

In the game:

1. Catch a Pokémon or receive a gift Pokémon.
2. Press **L + R**: the log shows `New Pokemon: species ...`.
3. Catch a Wurmple on Route 101 or Route 102. It does not join your party.

## Change the result or the parameters

A hook can do three things:

| What | How |
|---|---|
| Change the parameters | Change them before you call `original(...)`. |
| Change the result | Change the value that `original(...)` returns, then return it. |
| Replace the function | Do not call `original(...)`. Return your own value. |

## Hooks in the battle code

The battle code is a CRO: it is in memory only during a battle.
For a hook in the battle code, add the vtable of the battle process after
the address:

```cpp
HOOK(void, UpdateGauge, (uptr gauge, u16 max_hp, u32 new_hp),
     battle::address::kUpdateGauge, battle::address::kVtable) {
  original(gauge, max_hp, new_hp);
}
```

The plugin writes the hook again at the start of each battle.

## Disable a hook

The hook is an object: `AddPokemonToTeam::original`.

```cpp
AddPokemonToTeam::original.Disable();  // The game function is normal again.
AddPokemonToTeam::original.Enable();   // The hook works again.
```

## Add a hook to the library

When a hook is useful for all the products, add a feature to the library:

1. Add the address to `lib/include/<domain>/address.h`.
2. Make the feature in `lib/include/<domain>/patch/` and `lib/src/<domain>/`.
3. Write the hook in the `.cc` file of the feature. Use `HOOK()`, or a
   `core::Hook` object and `Install()` when the hook function is a private
   member of the class (see [Hooks and addresses](../concepts/hooks-and-addresses.md#a-hook-in-a-class-of-the-library)).
4. Call the `Initialize()` of the feature in `plugin::InitializeEngine()`
   (`lib/src/plugin.cc`) if the feature has one.
5. Give the feature callbacks or settings. The products use them.

## What you learned

- A hook has the same signature as the game function.
- `HOOK()` declares a hook. The plugin installs it at the start.
- `original(...)` runs the original function.
- A hook in a CRO needs the vtable of its process.
- The data of a Pokémon is encrypted.

**Next:** [Tutorial 4: Add a move](04-add-a-move.md)
