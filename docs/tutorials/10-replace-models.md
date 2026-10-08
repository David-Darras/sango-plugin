# Tutorial 10: Replace models

In this tutorial, you change two models:

1. The player looks like a Team Aqua Grunt in the overworld.
2. On the title screen, Groudon becomes a shiny Mega Camerupt.

**Time:** 20 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md).

## How it works

The game reads its models from **archives** (GARC files). Each model is one
file of an archive, with a file id.

- `core::Archive` calls your callbacks before the game reads a file.
  Your callback can give a different file id. The game then loads a different model.
- `pokemon::ModelReplacement` calls your callback before the game makes a
  Pokémon model. Your callback can change the species, the form and the color.

## Step 1: Replace the model of the player

The overworld models are in the archive `ArchiveId::kOverworldModel`.
The file id of a model is its `ModelId`.

The game reads files in two ways: with a stream and with a queued read.
Use the two callbacks.

Make the file `myhack/src/my_models.cc`:

```cpp
/**
 * @file my_models.cc
 * @brief The model changes of my ROM hack.
 */

#include "core/address.h"
#include "core/native/process_manager.h"
#include "core/patch/archive.h"
#include "overworld/constant/model.h"
#include "pokemon/patch/model_replacement.h"

namespace myhack {
namespace {

// Returns the model to use instead of `model`.
ModelId ReplaceModel(ModelId model) {
  if (model == ModelId::kBrendan) return ModelId::kTeamAquaGruntMale;
  return model;
}

// The game streams a file of an archive.
u32 OnStreamFile(const u32* archive, u32 file_id) {
  if (core::Archive::IsArchive(archive, ArchiveId::kOverworldModel)) {
    return static_cast<u32>(ReplaceModel(static_cast<ModelId>(file_id)));
  }
  return file_id;
}

// The game reads a file of an archive with a queued read.
void OnReadFile(core::ArchiveInput* input) {
  if (core::Archive::IsArchive(input, ArchiveId::kOverworldModel)) {
    input->file_id =
        static_cast<u32>(ReplaceModel(static_cast<ModelId>(input->file_id)));
  }
}

} // namespace

void InstallModels() {
  auto& archive = core::Archive::GetInstance();
  archive.on_stream_file = OnStreamFile;
  archive.on_read_file = OnReadFile;
}

} // namespace myhack
```

`ModelId::kBrendan` is the male player. For the female player, use `ModelId::kMay`.

## Step 2: Replace a Pokémon model

Add this to `myhack/src/my_models.cc`, in the unnamed namespace:

```cpp
// The game makes a Pokemon model. `info` describes the model.
void OnPokemonModel(PokeInfo* info) {
  // Change the model only on the title screen.
  if (!core::ProcessManager::GetInstance().IsCurrentProcess(
          core::address::kTitleScreenVtable)) {
    return;
  }
  if (info->species == SpeciesId::kGroudon && info->form == FormId::kNormal) {
    info->species = SpeciesId::kCamerupt;
    info->form = FormId::kCameruptMega;
    info->is_shiny = true;
  }
}
```

Then add one line to `InstallModels()`:

```cpp
pokemon::ModelReplacement::GetInstance().on_create = OnPokemonModel;
```

This changes only the model. The data of the Pokémon does not change.

## Step 3: Install the callbacks

In `myhack/src/entrypoint.cc`, declare `myhack::InstallModels()` and call it in
`InstallCallbacks()`.

## Step 4: Build and test

```bash
make myhack
```

1. On the title screen, a shiny Mega Camerupt replaces Groudon.
2. Start the game with the male player. In the overworld, the player looks like a Team Aqua Grunt.

## Find the ids

| What | Where |
|---|---|
| Overworld models | `lib/include/overworld/constant/model.h` (`ModelId`) |
| Archives | `lib/include/core/constant/archive_id.h` (`ArchiveId`) |
| Species | `lib/include/pokemon/constant/species.h` (`SpeciesId`) |
| Forms | `lib/include/pokemon/constant/form.h` (`FormId`) |
| Processes | `lib/include/core/address.h` (`k...Vtable`) |

The overlay has a page to preview the overworld models: **Player > Model**.

## Examples in the project

| File | Change |
|---|---|
| `undertow/src/entrypoint.cc` | The player as a Team Aqua Grunt, the title screen Pokémon. |
| `kaizo/src/kaizo_model.cc` | The overworld models of the Pokémon and of the characters. |
| `kaizo/src/entrypoint.cc` | The icon of the player (`ArchiveId::kPlayerIcon`). |

## What you learned

- The models are files of archives. `core::Archive` changes the file id.
- `pokemon::ModelReplacement` changes the species, the form and the color of a Pokémon model.
- `IsCurrentProcess` limits a change to one part of the game.

**Next:** [Tutorial 11: Change items and shops](11-change-items-and-shops.md)
