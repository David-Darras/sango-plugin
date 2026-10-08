# Tutorial 12: Add a new Pokémon

> **Coming soon.** This tutorial is not ready yet.

The library already adds Pokémon of generations VII, VIII and IX to the game
(see `pokemon::SpeciesTable` and `pokemon::AlolanForms`).
A new Pokémon needs two parts:

1. **The data:** base stats, types, abilities, learnset, evolutions, name.
   The data is in `lib/src/pokemon/data/gen7_species.inc` and
   `lib/src/pokemon/data/gen8_species.inc`.
2. **The 3D model:** the game cannot read the models of later games directly.
   A conversion tool changes them into the format of Alpha Sapphire.
   The plugin reads the converted models from the SD card
   (`sdmc:/sango/pokemodel/`).

The conversion tools are not public yet. This tutorial will explain the two
parts when the tools are available.

Until then, you can:

- change the data of an existing species with the page
  **Pokemon > Species Data** of the overlay,
- change the look of a Pokémon with `pokemon::ModelReplacement`
  ([Tutorial 10](10-replace-models.md)).
