# Tutorial 15: Add an item

In this tutorial, you add a new item: **Quick Charm**. When a Pokémon that holds
it enters a battle, its Speed increases by one stage. The overlay already has
two new items (Plasma Core and Dream Pillow) in `overlay/src/custom_items.cc`:
you can use them as examples.

**Time:** 20 minutes.
**You need:** [Tutorial 4](04-add-a-move.md) and [Tutorial 5](05-add-an-ability.md).
**Game:** Alpha Sapphire only. Pokémon X has no support for new items.

## How a new item works

Alpha Sapphire has 775 items (ids 1 to 775). A new item has an id from 800 to
1023 (`ItemExtension::kFirstId` to `ItemExtension::kLastId`).

`battle::ItemExtension` makes the game accept the id:

- It gives the name and the description of the item to the game, in the
  summary and in the menus.
- It gives an icon to the new item: your own icon, or the icon of an item of
  the game.
- It changes the data of the item (price, Fling...) with a function that you
  write.
- It registers the reactions of the item when a Pokémon holds it in a battle.

The data of a new item starts as the data of a Heart Scale, an item with no
effect. Your function then changes the data.

## Step 1: Choose an id

Add the id to a header of your product:

```cpp
constexpr ItemId kItemQuickCharm = static_cast<ItemId>(810);
```

The overlay uses the ids 800 and 801. Use a different id.

## Step 2: Write the item

Make the file `myhack/src/my_items.cc`:

```cpp
#include "battle/constant/moment_kind.h"
#include "battle/constant/mutation_kind.h"
#include "battle/constant/situation_key.h"
#include "battle/constant/stat_stage_effect_kind.h"
#include "battle/native/controller.h"
#include "battle/native/mutation.h"
#include "battle/native/situation.h"
#include "battle/patch/item_extension.h"
#include "pokemon/native/item_data.h"

namespace myhack {
namespace {

// The reaction: the battle engine calls it when a Pokémon enters the battle.
void QuickCharmReaction(battle::Listener* self, battle::Controller* controller,
                        battle::UID owner, s32* local_state) {
  // The moment is for all the Pokémon. Continue only for the holder.
  if (battle::Situation::Get(battle::SituationKey::kPokemonId) !=
      owner.value) {
    return;
  }

  auto* mutation = static_cast<battle::AdjustStatStageMutation*>(
      controller->Create(battle::MutationKind::kAdjustStatStage, owner));
  mutation->target_count = 1;
  mutation->target_ids[0] = owner;
  mutation->stage_kind = battle::StatStageEffectKind::kSpeed;
  mutation->stage_delta = 1;
  controller->Apply(mutation);
}

// The data of the item: the game shows the price in a Poké Mart.
void PatchQuickCharmData(pokemon::ItemData& item) {
  item.price = 2000;
  item.fling_power = 30;
}

const battle::ReactionTable kQuickCharmReactions[] = {
    {battle::MomentKind::kPokemonEntered, QuickCharmReaction},
};

} // namespace

void RegisterItems() {
  battle::ItemExtension::AddItem(
      {kItemQuickCharm, u"Quick Charm",
       u"A charm that makes the holder faster\nwhen it enters a battle.",
       ItemId::kLightBall, PatchQuickCharmData, kQuickCharmReactions,
       SIZE(kQuickCharmReactions)});
}

} // namespace myhack
```

The fields of the description are:

| Field | Meaning |
|---|---|
| `id` | The id of the item. |
| `name` | The name that the game shows. |
| `description` | The description that the game shows. Use `\n` for a new line. |
| `icon_item` | The item of the game whose icon the new item shows. |
| `patch_data` | The function that changes the data of the item. It can be `nullptr`. |
| `reactions`, `reaction_count` | The reactions of the item in a battle. They can be `nullptr` and 0. |

The reactions are the same as the reactions of a move or of an ability (see
[Tutorial 5](05-add-an-ability.md)). The moment that you choose tells when the
engine calls them. The engine calls the reactions only when a Pokémon holds the
item.

## Step 3: Make the icon

The icon of an item is an image of 30 x 30 pixels. The game reads it as a file
on the SD card: `sdmc:/sango/items/<name>.bclim`, with the name of the item
(for example `Quick Charm.bclim`). You do not give the path.

Draw a PNG with any program (the size is 30 x 30 pixels, the background is
transparent). Then change it to the format that the game reads. The tool keeps
15 colors for the opaque pixels:

```bash
python tools/png_to_item_icon.py "assets/items/Quick Charm.png" "Quick Charm.bclim"
```

Copy the file to `sdmc:/sango/items` (for the emulator, the folder
`sdmc/sango/items`). The release script makes the BCLIM files of all the PNG
files of `assets/items` by itself, and it puts them in the zip: keep the PNG
file in your project.

Without the file, the item shows the icon of the item `icon_item`.

## Step 4: Register the item

Call your function in your `Initialize()` function, after the start of the
plugin:

```cpp
void Initialize() {
  plugin::InitializeEngine();
  myhack::RegisterItems();
  // ...
}
```

## Step 5: Give the item to a Pokémon

Write the id in the data of a Pokémon:

```cpp
core->item = kItemQuickCharm;
```

The overlay does it for the test team: the button **Receive The New Type Team**
(page **Pokémon**) gives Plasma Core to the first Pokémon and Dream Pillow to the
second.

## Use the item in the Bag

An item can also act on a Pokémon when the player uses it in the Bag, like a
Rare Candy. Give two functions to the description: `can_use` tells if the item
can act on a Pokémon, and `use` does the change.

```cpp
// The player can use the item on any Pokémon.
bool CanUseShinySwitch(const pokemon::CoreData& pokemon) {
  return true;
}

// The change, and the message that the Bag shows.
const c16* UseShinySwitch(pokemon::CoreData& pokemon) {
  // A shiny Pokémon becomes normal, a normal Pokémon becomes shiny.
  const bool is_shiny = pokemon::Utils::IsShiny(pokemon.id, pokemon.shiny_id);
  pokemon.SetShiny(!is_shiny);
  return is_shiny ? u"{pokemon} lost its shine."
                  : u"{pokemon} shines with all the colors!";
}

battle::ItemExtension::AddItem(
    {kItemShinySwitch, u"Shiny Switch",
     u"A gem that shines with all the colors.\nUse it on a Pokemon to make it "
     u"shiny.",
     ItemId::kHeartScale, nullptr, nullptr, 0, CanUseShinySwitch, UseShinySwitch});
```

The Bag then shows "Use" for the item. After the player chooses a Pokémon:

1. The plugin calls `can_use`. When it returns false, the Bag shows "It won't
   have any effect."
2. The plugin calls `use`. The function changes the data of the Pokémon (the
   shiny state, the ability, the species, the moves...) and returns the message.
   In the text, `{pokemon}` becomes the name of the Pokémon. With null, the Bag
   shows "{pokemon} used the item."
3. The game removes one item, updates the screen of the Pokémon and shows the
   message.

The overlay has two examples: Shiny Switch (a shiny Pokémon) and Ability Switch
(the next ability). The button **Put The New Items In The Bag** (page
**Pokémon**) puts them in the Bag.

## Make an evolution stone

An item can make a Pokémon evolve. Give the function `evolve_into` and no `use`
function. The game plays its own animation of evolution, checks the Pokémon, and
removes the item.

```cpp
// Gives the species that the item makes: the first evolution, with any method.
SpeciesId EvolveIntoFirstEvolution(SpeciesId species,
                                   const pokemon::EvolutionData& evolutions) {
  const auto& first = evolutions.data[0];
  return first.method != pokemon::EvolutionMethod::kNone ? first.species
                                                         : SpeciesId::kNone;
}

battle::ItemExtension::AddItem(
    {kItemEvolutionPrism, u"Evolution Prism",
     u"A prism that holds the power of all\nthe evolution stones. Use it on a "
     u"Pokemon\nto make it evolve.",
     ItemId::kHeartScale, nullptr, nullptr, 0, nullptr, nullptr,
     EvolveIntoFirstEvolution});
```

Each time that the game loads the evolutions of a species, the plugin calls
`evolve_into`. When the function returns a species, the plugin adds one line to
the evolutions: "use the item". The game then treats the item like an
evolution stone. A species for which the function returns `SpeciesId::kNone`
does not evolve with the item, and the Bag shows "It won't have any effect."

A species has room for 8 evolutions. A species with 8 evolutions (Eevee) has no
room for the new line.

## Make a medicine

The Bag has its own effects for the items that the player uses on a Pokémon. The
game does the work and shows its own messages. The data function of the item
chooses the effects with `battle::ItemMedicine` (`battle/patch/item_medicine.h`).
Call `MakeUsable()` first, then add the effects:

```cpp
void PatchMoonTonicData(pokemon::ItemData& item) {
  ItemMedicine::MakeUsable(item);
  ItemMedicine::RestoreHp(item, ItemMedicine::kHalf);
  ItemMedicine::CureAllStatus(item);
}

ItemExtension::AddItem({kItemMoonTonic, u"Moon Tonic", u"...",
                        ItemId::kHeartScale, PatchMoonTonicData});
```

| Function | Effect |
|----------|--------|
| `RestoreHp(item, amount)` | Restores HP: a number, `kQuarter`, `kHalf` or `kAll` |
| `Revive(item, amount)` | Revives a fainted Pokémon with that many HP |
| `ReviveParty(item)` | Revives all the fainted Pokémon of the party |
| `CureStatus(...)`, `CureAllStatus(item)` | Cures sleep, poison, burn, freeze, paralysis |
| `LevelUp(item)` | Raises the level by one |
| `RestorePp(item, amount, all_moves)` | Restores the PP of one move (the player chooses) or of all |
| `RaisePp(item, to_maximum)` | A PP Up or a PP Max (the player chooses the move) |
| `ChangeEv(item, stat, amount)` | Adds or removes effort values (-127 to 127) |
| `IgnoreEvLimit(item, true)` | Lets a vitamin go above 100 effort values (up to 252) |
| `ChangeFriendship(item, low, medium, high)` | Changes the friendship; one number for each range (under 100, 100 to 199, 200 and more) |

The game has no medicine that removes HP. For this effect, and for all the other
effects, give the item a `use` function (see "Use the item in the Bag") and call
`ItemExtension::ChangeHp(-20)` in it: a positive number adds HP, a negative number
removes HP, and the HP stay between 1 and the maximum HP. Set the last field of
the `ItemSpec` (`changes_hp`) to `true`: the Bag then plays the animation of the HP
bar like for a medicine, and the bar counts down when the HP go down.

## Change an item of the game

You can also change an item that the game has (ids 1 to 775) with the same
function. Give the id of the item. A field that you leave empty keeps the value
of the game:

```cpp
// Leftovers restore 1/8 of the HP, not 1/16. The name, the data and the icon do
// not change.
battle::ItemExtension::AddItem(
    {ItemId::kLeftovers, nullptr,
     u"A held item that restores 1/8 of the\nmax HP of the holder at the end of\n"
     u"each turn.",
     ItemId::kNone, nullptr, kLeftoversReactions, SIZE(kLeftoversReactions)});
```

| Field | With the id of an item of the game |
|---|---|
| `name`, `description` | Null keeps the text of the game. |
| `icon_item` | `ItemId::kNone` keeps the icon of the game. A file `sdmc:/sango/items/<name>.bclim` also changes the icon (use the name of the item). |
| `patch_data` | Changes the data of the game (price, Fling...). Null keeps it. |
| `reactions` | Null keeps the reactions of the game. A table replaces all the reactions of the game. |

Some items have effects in the code of the game and not in their reactions (for
example the damage of Life Orb). To change them, replace the reactions and write
the new effect yourself.

## Check

1. Build with `make myhack` and give the item to a Pokémon.
2. The summary and the party menu show the name and the icon of the item.
3. In a battle, the reaction occurs at the moment that you chose.

## What the plugin does not change

- A new item is in the Bag only when your product puts it there: the overlay
  does it with the button **Put The New Items In The Bag**. The player cannot
  buy it yet. The items that the player uses from the Bag (a candy, a capsule, a
  mineral) are the next step.
- The data that your function `patch_data` writes (price, Fling...) is for the
  code that reads the item through the data of the item. The Bag reads the data
  of a Heart Scale for a new item.
