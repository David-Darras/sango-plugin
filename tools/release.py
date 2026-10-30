#!/usr/bin/env python3
"""Builds the release zip of the plugin and publishes it on GitHub.

Usage (the Makefile calls it):
  release.py pack    --version 7.2.0 --plugin sango_plugin-release.3gx
  release.py publish --version 7.2.0 [--draft] [--yes]

The zip has the layout of the SD card:
  luma/plugins/000400000011C500/sango_plugin.3gx
  README.txt
"""

import argparse
import os
import shutil
import subprocess
import sys
import zipfile

TITLE_ID = "000400000011C500"
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASSETS = os.path.join(ROOT, "assets", "release")
DIST = os.path.join(ROOT, "dist")


def fill(text, version, pokemon_url, overworld_url, music_url):
    """Replaces the placeholders of a template."""
    default = "(the link is on the release page)"
    return (text.replace("{{VERSION}}", version)
                .replace("{{POKEMON_MODELS_URL}}", pokemon_url or default)
                .replace("{{OVERWORLD_MODELS_URL}}", overworld_url or default)
                .replace("{{MUSIC_URL}}", music_url or default))


def read_asset(name):
    with open(os.path.join(ASSETS, name), encoding="utf-8") as f:
        return f.read()


def zip_path(version):
    return os.path.join(DIST, "Sango-Plugin-%s.zip" % version)


def notes_path():
    return os.path.join(DIST, "notes.md")


def pack(args):
    if not os.path.isfile(args.plugin):
        sys.exit("The plugin file does not exist: %s (build it first)" % args.plugin)
    os.makedirs(DIST, exist_ok=True)

    readme = fill(read_asset("README.txt"), args.version, args.pokemon_models_url,
                  args.overworld_models_url, args.music_url)
    notes = fill(read_asset("notes.md"), args.version, args.pokemon_models_url,
                 args.overworld_models_url, args.music_url)
    with open(notes_path(), "w", encoding="utf-8", newline="\n") as f:
        f.write(notes)

    target = zip_path(args.version)
    with zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as z:
        z.write(args.plugin, "luma/plugins/%s/sango_plugin.3gx" % TITLE_ID)
        z.writestr("README.txt", readme.replace("\r\n", "\n").replace("\n", "\r\n"))
        # The icons of the new types (sdmc:/sango/types): the PNG files of
        # assets/types become BCLIM files here (the files are not in the repo).
        types_dir = os.path.join(ROOT, "assets", "types")
        sys.path.insert(0, os.path.join(ROOT, "tools"))
        import png_to_bclim
        from PIL import Image
        for name in sorted(os.listdir(types_dir)):
            if name.endswith(".png"):
                with Image.open(os.path.join(types_dir, name)) as image:
                    data = png_to_bclim.convert(image)
                z.writestr("sango/types/%s.bclim" % name[:-4], data)
    shutil.copyfile(args.plugin, os.path.join(DIST, "sango_plugin.3gx"))

    print("Release pack: %s" % target)
    with zipfile.ZipFile(target) as z:
        for info in z.infolist():
            print("  %-52s %8d bytes" % (info.filename, info.file_size))
    if not (args.pokemon_models_url and args.overworld_models_url and args.music_url):
        print("Note: POKEMON_MODELS_URL, OVERWORLD_MODELS_URL or MUSIC_URL is empty. Set them in config.mk.")


def git(*args):
    return subprocess.run(["git"] + list(args), cwd=ROOT, capture_output=True,
                          text=True).stdout.strip()


def publish(args):
    tag = "v" + args.version
    target = zip_path(args.version)
    if not os.path.isfile(target) or not os.path.isfile(notes_path()):
        sys.exit("Run `make release-pack` first.")
    if shutil.which("gh") is None:
        sys.exit("The GitHub CLI (gh) is not installed. See https://cli.github.com")

    if git("status", "--porcelain"):
        print("Warning: the working tree has uncommitted changes.")
    head = git("rev-parse", "HEAD")
    if not git("branch", "-r", "--contains", head):
        sys.exit("The commit %s is not on GitHub. Push it first: git push" % head[:8])

    exists = subprocess.run(["gh", "release", "view", tag], cwd=ROOT,
                            capture_output=True).returncode == 0
    if exists:
        sys.exit("The release %s already exists. Delete it or change the version." % tag)

    kind = "a DRAFT of the release" if args.draft else "the release"
    if not args.yes:
        answer = input("Publish %s %s (commit %s) on GitHub? [y/N] "
                       % (kind, tag, head[:8])).strip().lower()
        if answer not in ("y", "yes"):
            sys.exit("Cancelled.")

    cmd = ["gh", "release", "create", tag, target,
           os.path.join(DIST, "sango_plugin.3gx"),
           "--title", "Sango Plugin %s" % tag,
           "--notes-file", notes_path(), "--target", head]
    if args.draft:
        cmd.append("--draft")
    subprocess.run(cmd, cwd=ROOT, check=True)
    print("Done.")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("pack")
    p.add_argument("--version", required=True)
    p.add_argument("--plugin", required=True)
    p.add_argument("--pokemon-models-url", default="")
    p.add_argument("--overworld-models-url", default="")
    p.add_argument("--music-url", default="")
    p.set_defaults(func=pack)

    p = sub.add_parser("publish")
    p.add_argument("--version", required=True)
    p.add_argument("--draft", action="store_true")
    p.add_argument("--yes", action="store_true")
    p.set_defaults(func=publish)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
