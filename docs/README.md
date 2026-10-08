# Sango Plugin documentation

This documentation helps you to make your own ROM hack of Pokémon Alpha Sapphire
with the Sango Plugin library. You do not need to know reverse engineering.
You need to know some C++.

## Getting started

Do these steps in order.

1. [Install the tools](getting-started/01-install-the-tools.md): devkitPro, libctrpf, 3gxtool, Git.
2. [Build the plugin](getting-started/02-build-the-plugin.md): download the code and type `make`.
3. [Run the plugin](getting-started/03-run-the-plugin.md): on the Azahar emulator or on a 3DS console.
4. [Set up your editor](getting-started/04-set-up-your-editor.md): code completion and the API reference.
5. [Troubleshooting](getting-started/05-troubleshooting.md): the common problems and their solutions.

## Concepts

Read these pages to understand the code.

- [Architecture](concepts/architecture.md): the library, the products, the domains, the start of the plugin.
- [Hooks and addresses](concepts/hooks-and-addresses.md): how the plugin changes the game.
- [Game structures](concepts/game-structures.md): how the plugin reads the data of the game.
- [The battle engine](concepts/battle-engine.md): moments, reactions and mutations.
- [Scripts](concepts/scripts.md): the scripts of the game and the C++ scripts.
- [The menu](concepts/menu.md): pages and entries.
- [Settings](concepts/settings.md): the values that change how a feature works.

## Tutorials

Short procedures with code that you can copy.
See [the list of the tutorials](tutorials/README.md).

| Tutorial | |
|---|---|
| [1. Create your ROM hack](tutorials/01-create-your-rom-hack.md) | [7. Change the wild Pokémon](tutorials/07-change-wild-pokemon.md) |
| [2. Add a menu page](tutorials/02-add-a-menu-page.md) | [8. Change the trainer teams](tutorials/08-change-trainer-teams.md) |
| [3. Add a hook](tutorials/03-add-a-hook.md) | [9. Write an overworld script](tutorials/09-write-an-overworld-script.md) |
| [4. Add a move](tutorials/04-add-a-move.md) | [10. Replace models](tutorials/10-replace-models.md) |
| [5. Add an ability](tutorials/05-add-an-ability.md) | [11. Change items and shops](tutorials/11-change-items-and-shops.md) |
| [6. Make a move animation](tutorials/06-make-a-move-animation.md) | [12. Add a new Pokémon](tutorials/12-add-a-new-pokemon.md) (coming soon) |

## Reference

- [Glossary](reference/glossary.md): the meaning of each term.
- [Writing rules](reference/writing-rules.md): the style of the code, of the comments and of the documentation.
- **API reference:** type `doxygen` in the project folder, then open `doxygen/html/index.html`.
