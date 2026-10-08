# Tutorial 3: Add a hook

In this tutorial, you replace a function of the game with your own function.
The game calls `AddPokemonToTeam` when a Pokémon joins a party.
Your hook writes the species to the log. It also stops Wurmple: Wurmple cannot
join the party of the player.

**Time:** 20 minutes.
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
#include "core/hook_manager.h"
#include "pokemon/address.h"
#include "savedata/native/pokemon_team.h"
#include "ui/log_application.h"

namespace myhack {
namespace {

// The hook. It has the same parameters and the same return type as the
// game function.
bool AddPokemonToTeamHook(savedata::PokemonTeam* team,
                          savedata::PokemonParam* pokemon) {
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
  return core::HookManager::Call<bool>(HookId::kProduct0, team, pokemon);
}

} // namespace

void InstallHooks() {
  core::HookManager::Initialize(HookId::kProduct0,
                                pokemon::address::kAddPokemonToTeam,
                                (uptr)AddPokemonToTeamHook);
}

} // namespace myhack
```

Each hook needs its own hook id. A product uses `HookId::kProduct0` to
`HookId::kProduct15`. Do not use the same id two times.

## Step 3: Install the hook

In `myhack/src/entrypoint.cc`:

1. Declare the function before `InstallCallbacks`:

   ```cpp
   namespace myhack {
   void InstallHooks();
   }
   ```

2. Call it in `Initialize()`, after `plugin::InitializeEngine()`:

   ```cpp
   void Initialize() {
     plugin::InitializeEngine();
     InstallCallbacks();
     myhack::InstallHooks();
     plugin::OpenMenu(ui::MainAppPainter::GetInstance(), LoadRootPage);
     plugin::Start(EveryFrame);
   }
   ```

## Step 4: Build and test

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
| Change the parameters | Change them before `HookManager::Call`. |
| Change the result | Change the value that `HookManager::Call` returns, then return it. |
| Replace the function | Do not call `HookManager::Call`. Return your own value. |

## Hooks in the battle code

The battle code is a CRO: it is in memory only during a battle.
For a hook in the battle code:

1. Install it disabled: `HookManager::Initialize(id, address, function, false)`.
2. Enable it when a battle starts: `HookManager::ForceEnable(id)`.
   Use the callback `core::ProcessPatch::GetInstance().on_process_load`
   and compare the vtable with `battle::address::kVtable`.

## Add a hook to the library

When a hook is useful for all the products, add a feature to the library:

1. Add the address to `lib/include/<domain>/address.h`.
2. Add a hook id to `lib/include/core/constant/hook_id.h` (before `kProduct0`).
3. Make the feature in `lib/include/<domain>/patch/` and `lib/src/<domain>/`.
4. Call its `Initialize()` in `plugin::InitializeEngine()` (`lib/src/plugin.cc`).
5. Give the feature callbacks or settings. The products use them.

## What you learned

- A hook has the same signature as the game function.
- `HookManager::Initialize` installs a hook, `HookManager::Call` runs the original function.
- A product uses the hook ids `kProduct0` to `kProduct15`.
- The data of a Pokémon is encrypted.

**Next:** [Tutorial 4: Add a move](04-add-a-move.md)
