"""Adds your own music to the overlay of Sango Plugin.

It converts mp3, ogg, flac, wav, m4a (any format of ffmpeg) to the .bcstm
format of the game. It then adds the music to the list "SD Music" of the page
Sound.

First time only:
  python tools/add_music.py --setup

Add music:
  python tools/add_music.py "My Song.mp3"
  python tools/add_music.py "My Song.mp3" --title "Victory Fanfare" --no-loop
  python tools/add_music.py "My Song.mp3" --loop 12.5 90
  python tools/add_music.py my_music_folder

The result is in the folder "sdmc/sango/bgm". Copy the folder "sango" of
"sdmc" to the SD card of the console (or to the SD folder of the emulator).
Without --loop and --no-loop, the music loops from the first second to the
last second.
"""

import argparse
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bcstm_tools

AUDIO = {".mp3", ".ogg", ".oga", ".opus", ".flac", ".wav", ".m4a", ".aac",
         ".wma", ".aiff", ".aif"}
PREFIX = "my_"
MAX_NAME = 39


def make_name(title):
  """Returns a file name without spaces: "My Song!" gives "my_my_song"."""
  slug = re.sub(r"[^a-z0-9]+", "_", title.lower()).strip("_")
  return (PREFIX + slug)[:MAX_NAME]


def read_list(path):
  entries = {}
  if os.path.isfile(path):
    for line in open(path, encoding="utf-8"):
      line = line.strip()
      if line:
        name, _, title = line.partition("|")
        entries[name] = title or name
  return entries


def write_list(path, entries):
  # The menu shows the titles in the order of the file: sort by title.
  ordered = sorted(entries.items(), key=lambda item: item[1].lower())
  with open(path, "w", encoding="utf-8", newline="\n") as f:
    f.write("".join("%s|%s\n" % item for item in ordered))


def main():
  parser = argparse.ArgumentParser(description=__doc__,
      formatter_class=argparse.RawDescriptionHelpFormatter)
  parser.add_argument("input", nargs="?", help="An audio file or a folder.")
  parser.add_argument("--setup", action="store_true",
                      help="Download VGAudioCli (one time).")
  parser.add_argument("--title", help="The title in the menu (one file). "
                      "Default: the name of the file.")
  parser.add_argument("--loop", nargs=2, type=float, metavar=("START", "END"),
                      help="The loop points, in seconds.")
  parser.add_argument("--no-loop", action="store_true",
                      help="The music plays one time.")
  parser.add_argument("--out", default=os.path.join("sdmc", "sango", "bgm"),
                      help="The output folder. Default: sdmc/sango/bgm.")
  parser.add_argument("--ffmpeg")
  parser.add_argument("--vgaudio")
  args = parser.parse_args()

  if args.setup:
    bcstm_tools.setup_vgaudio()
    return
  if not args.input:
    parser.error("give an audio file or a folder")
  if args.loop and args.no_loop:
    parser.error("use --loop or --no-loop, not both")

  if os.path.isdir(args.input):
    files = [os.path.join(args.input, f) for f in sorted(os.listdir(args.input))
             if os.path.splitext(f)[1].lower() in AUDIO]
    if args.title:
      parser.error("--title works with one file only")
  else:
    files = [args.input]
  if not files:
    sys.exit("No audio file found.")

  os.makedirs(args.out, exist_ok=True)
  list_path = os.path.join(args.out, "list.txt")
  entries = read_list(list_path)
  loop = None if args.no_loop else (tuple(args.loop) if args.loop else "full")
  for source in files:
    title = args.title or os.path.splitext(os.path.basename(source))[0]
    title = title.replace("|", " ")[:45]
    name = make_name(title)
    out_path = os.path.join(args.out, name + bcstm_tools.EXTENSION)
    print("Converting %s -> %s" % (source, name + bcstm_tools.EXTENSION),
          flush=True)
    bcstm_tools.convert(source, out_path, loop, args.ffmpeg, args.vgaudio)
    entries[name] = title
  write_list(list_path, entries)
  print("Done. %d music in %s" % (len(entries), list_path))


if __name__ == "__main__":
  main()
