#!/usr/bin/env python3
"""Makes a new ROM hack: a product folder that the Makefile builds.

    python tools/new_hack.py myhack
    python tools/new_hack.py myhack --author "Your name" --summary "My ROM hack"

The script makes these files in the project folder:

    myhack/
      myhack.plgInfo     the information of the plugin (3gxtool reads it)
      include/           your headers
      src/entrypoint.cc  your settings and your menu (see lib/include/hack.h)

Then type `make myhack` in the devkitPro shell. The plugin file is
myhack-release.3gx. See docs/tutorials/01-create-your-rom-hack.md.
"""

import argparse
import os
import re
import sys

PROJECT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# The folders of the project that are not products, and the products of the
# project.
RESERVED = {
    "assets", "build", "debug", "dist", "docs", "kaizo", "lib", "overlay",
    "private", "release", "release-overlay", "tools", "undertow",
}

PLGINFO = """\
Author: {author}

Version:
    Major: 0
    Minor: 1
    Revision: 0

Targets:
    #- 0x0011C500

Title: {name}

Summary: {summary}

Description: |
  {summary}. A ROM hack for Pokemon Alpha Sapphire v1.4, made with Sango Plugin.

Compatibility: Any

MemorySize: 5MiB

UsePrivateMemory: false
"""

ENTRYPOINT = """\
/**
 * @file entrypoint.cc
 * @brief The settings and the menu of {summary}.
 *
 * The library calls the functions of this file (see lib/include/hack.h):
 * - SetUpHack() one time, when the game starts,
 * - LoadHackMenu() when the player opens the menu (Start),
 * - UpdateHack() one time for each frame.
 * Remove a function that you do not need.
 */

#include "sango.h"

// Sets the features of the ROM hack: new moves, wild Pokemon, scripts...
// The tutorials of docs/tutorials/ add their code here.
void SetUpHack() {{
  // Example: 1 wild Pokemon out of 8 is shiny.
  pokemon::Shiny::GetInstance().rate = pokemon::ShinyRate::k1_8;
}}

// The first page of the menu. Without this function, the menu shows all the
// pages of the library.
void LoadHackMenu(ui::MainApplication& app, void* args) {{
  app.Add("Shiny rate", pokemon::Shiny::GetInstance().rate)
     .Add("Battle", ui::LoadBattlePage)
     .Add("Overworld", ui::LoadOverworldPage)
     .Add("Pokemon", ui::LoadPokemonPage);
}}

// Called one time for each frame. Keep it fast.
void UpdateHack() {{
}}
"""


def fail(message):
    print("error: " + message, file=sys.stderr)
    sys.exit(1)


def write(path, text):
    with open(path, "w", encoding="utf-8") as file:
        file.write(text)
    print("made " + os.path.relpath(path, PROJECT))


def main():
    parser = argparse.ArgumentParser(
        description="Makes a new ROM hack: a product folder that the Makefile "
                    "builds.")
    parser.add_argument("name",
                        help="the name of the product: lowercase letters, "
                             "digits and _ (for example myhack)")
    parser.add_argument("--author", default="Your name",
                        help="the author that the plugin shows")
    parser.add_argument("--summary", default=None,
                        help="one line about the ROM hack")
    args = parser.parse_args()

    name = args.name
    if not re.match(r"^[a-z][a-z0-9_]*$", name):
        fail("the name must start with a lowercase letter and contain only "
             "lowercase letters, digits and _: " + name)
    # The name is also the data folder sdmc:/sango/<name>: the paths of the
    # library must stay short.
    if len(name) > 20:
        fail("the name must have 20 characters at most: " + name)
    if name in RESERVED:
        fail("the project already uses the name " + name + ". Use an other name.")

    folder = os.path.join(PROJECT, name)
    if os.path.exists(folder):
        fail("the folder " + name + " already exists. Use an other name.")

    summary = args.summary or name
    for text in (args.author, summary):
        if "\n" in text or "\"" in text:
            fail("the author and the summary must be one line without \".")

    os.makedirs(os.path.join(folder, "include"))
    os.makedirs(os.path.join(folder, "src"))
    write(os.path.join(folder, name + ".plgInfo"),
          PLGINFO.format(author=args.author, name=name, summary=summary))
    write(os.path.join(folder, "src", "entrypoint.cc"),
          ENTRYPOINT.format(summary=summary))

    print()
    print("Your ROM hack is ready. In the devkitPro shell, type:")
    print()
    print("    make " + name)
    print()
    print("The plugin file is " + name + "-release.3gx.")
    print("The files of the ROM hack on the SD card are in sdmc:/sango/" + name + ".")


if __name__ == "__main__":
    main()
