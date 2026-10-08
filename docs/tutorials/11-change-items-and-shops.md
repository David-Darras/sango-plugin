# Tutorial 11: Change items and shops

In this tutorial, you change the data of items and the stock of a Poké Mart:

1. A Potion costs 100 instead of 200.
2. The vitamins (Protein, Iron...) give 63 EVs instead of 10.
3. The Poké Mart of Oldale Town sells Rare Candies and Master Balls.

**Time:** 20 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md).

> **Note:** `pokemon::ItemCustomizer` and `pokemon::CustomShop` work only on Alpha Sapphire.

## Part 1: Change the data of items

`pokemon::ItemCustomizer` calls your callback each time the game reads the data
of an item. Your callback changes the data.

Make the file `myhack/src/my_items.cc`:

```cpp
/**
 * @file my_items.cc
 * @brief The item changes of my ROM hack.
 */

#include "pokemon/constant/item.h"
#include "pokemon/native/item_data.h"
#include "pokemon/native/shop_item.h"
#include "pokemon/patch/custom_shop.h"
#include "pokemon/patch/item_customizer.h"

namespace myhack {
namespace {

// The game calls this function each time it reads the data of an item.
void OnItemData(pokemon::ItemData* item) {
  switch (item->id) {
    case ItemId::kPotion:
      item->price = 100;
      break;
    case ItemId::kHpUp:
      item->hp_ev_value = 63;
      break;
    case ItemId::kProtein:
      item->attack_ev_value = 63;
      break;
    case ItemId::kIron:
      item->defense_ev_value = 63;
      break;
    case ItemId::kCalcium:
      item->sp_atk_ev_value = 63;
      break;
    case ItemId::kZinc:
      item->sp_def_ev_value = 63;
      break;
    case ItemId::kCarbos:
      item->speed_ev_value = 63;
      break;
    default:
      break;
  }
}

} // namespace

void InstallItems() {
  pokemon::ItemCustomizer::GetInstance().on_item_data = OnItemData;
}

} // namespace myhack
```

The members of `pokemon::ItemData` are in `lib/include/pokemon/native/item_data.h`.
Some useful members:

| Member | Meaning |
|---|---|
| `price` | The price in a Poké Mart. The selling price is half. |
| `hold_effect`, `power` | The effect when a Pokémon holds the item. |
| `fling_power` | The power of Fling with this item. |
| `hp_restore_value` | The HP that the item restores. |
| `hp_ev_value`... `speed_ev_value` | The EVs that the item gives. |
| `evolve`, `use_on_pokemon`, `overworld_function` | Lets the player use the item on a Pokémon in the overworld (see `kaizo/src/kaizo_item.cc`). |

## Part 2: Change the stock of a Poké Mart

`pokemon::CustomShop::SetItems` replaces the items of a shop.

### Step 1: Find the id of the shop

1. Add this line to `InstallItems()` (see Part 1). It writes the id of each
   shop that opens to the log:

   ```cpp
   pokemon::CustomShop::GetInstance().log_shop_ids = true;
   ```

2. Build, install your product and start the game.
3. Talk to the clerk of the shop and open the shop.
4. Press **L + R**: the log shows `shop: type=... id=...`. Write down the id.
5. Remove the line of step 1.

### Step 2: Set the items

Add this to `myhack/src/my_items.cc`:

```cpp
namespace myhack {
namespace {

const pokemon::ShopItem kOldaleItems[] = {
    // Item, (unused), price.
    {ItemId::kRareCandy, 0, 1000},
    {ItemId::kMasterBall, 0, 50000},
    {ItemId::kPotion, 0, 100},
};

constexpr u32 kOldaleShopId = 1;  // Your shop id from the log.

} // namespace
} // namespace myhack
```

Then add one line to `InstallItems()`:

```cpp
pokemon::CustomShop::SetItems(kOldaleItems, SIZE(kOldaleItems), kOldaleShopId);
```

Without the last parameter, the list replaces the items of **all** the shops.
A shop has 60 items at most.

## Part 3: Sell Pokémon

`pokemon::CustomShop` can also sell Pokémon in the normal Poké Marts. This
mode is off by default. Enable it in your code:

```cpp
pokemon::CustomShop::EnablePokemonShop(true);
```

The player can also enable it in the menu with the cheat code
`CheatCodeId::kPokemonShop` (the overlay shows it in **Pokemon > Pokemon Shop**):

```cpp
app.Add("Pokemon Shop", CheatCodeId::kPokemonShop);
```

The shops then sell the Pokémon of `kShopPokemons`
(`lib/include/pokemon/patch/custom_shop.h`). To sell your own Pokémon, give
your own list:

```cpp
const pokemon::ShopPokemon kPokemonForSale[] = {
    // Species, form, level, price.
    {SpeciesId::kEevee, FormId::kNormal, 10, 5000},
    {SpeciesId::kLarvitar, FormId::kNormal, 15, 20000},
};

pokemon::CustomShop::SetPokemons(kPokemonForSale, SIZE(kPokemonForSale),
                                 kOldaleShopId);
pokemon::CustomShop::EnablePokemonShop(true);
```

> **Note:** A shop sells Pokémon **or** items. When a shop sells Pokémon,
> `SetItems` does not change it.

## Step 3: Install and test

In `myhack/src/entrypoint.cc`, declare `myhack::InstallItems()` and call it in
`InstallCallbacks()`. Then build:

```bash
make myhack
```

1. Open a Poké Mart. The Potion costs 100.
2. Open the Poké Mart of your shop id. It sells your items.
3. Use a Protein on a Pokémon. Its Attack EVs increase by 63.

## What you learned

- `ItemCustomizer::on_item_data` changes the data of an item.
- `CustomShop::SetItems` changes the stock of a shop.
- `CustomShop::EnablePokemonShop` and `SetPokemons` sell Pokémon in a shop.
- `log_shop_ids` writes the ids of the shops to the log.

**Next:** [Tutorial 12: Add a new Pokémon](12-add-a-new-pokemon.md)
