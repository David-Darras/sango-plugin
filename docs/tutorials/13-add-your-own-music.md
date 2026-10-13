# Tutorial 13: Add your own music

In this tutorial, you play your own music in the game:

1. You convert an mp3 file to the music format of the game.
2. You play it from the menu of the plugin.
3. You replace a music of the game by your file.
4. You add the music of Pokémon X and Y.

**Time:** 15 minutes.
**You need:** [Getting started](../getting-started/01-install-the-tools.md) and Python 3.

> **Note:** The plugin reads the music files from the SD card. You do not
> change the game files and you do not build the plugin again.

## How it works

The game plays its music from `.bcstm` files (DSP-ADPCM). A `.bcstm` file is
not an mp3 file. The tool `tools/add_music.py` converts your file in two steps:

1. `ffmpeg` reads your file (mp3, ogg, flac, wav, m4a...) and makes a stereo
   sound with a rate of 32 kHz.
2. `VGAudioCli` writes the `.bcstm` file.

The plugin reads the list `sdmc:/sango/bgm/list.txt`. Each line has the name of a
file and the title that the menu shows, with `|` between the two:

```text
my_victory_fanfare|Victory Fanfare
```

## Step 1: Install the tools (one time)

Install `ffmpeg`.

- Windows: `winget install Gyan.FFmpeg`
- macOS: `brew install ffmpeg`
- Linux: `sudo apt install ffmpeg`

Open a new terminal, then install `VGAudioCli`. The command downloads a file of
0.8 MB from the page of its author. On macOS and Linux, you also need
[.NET](https://dotnet.microsoft.com/download).

```bash
python tools/add_music.py --setup
```

## Step 2: Convert your music

```bash
python tools/add_music.py "My Song.mp3" --title "Victory Fanfare"
```

The tool writes the result in the folder `sdmc/sango/bgm`:

```text
sdmc/sango/bgm/list.txt
sdmc/sango/bgm/my_victory_fanfare.dspadpcm.bcstm
```

Run the command again for each file. The tool adds each music to `list.txt`.
You can also give a folder: the tool converts all the audio files of the folder
and uses the file names as titles.

```bash
python tools/add_music.py my_music_folder
```

### The loop

By default, the music loops from its first second to its last second. You can
change this:

| You want | Option |
|---|---|
| A loop between two times, in seconds | `--loop 12.5 90` |
| A music that plays one time | `--no-loop` |

## Step 3: Copy the files to the SD card

Copy the folder `sango` of `sdmc` to the root of the SD card of the console.

- On a console: `sdmc:/sango/bgm/`.
- On Azahar: the folder `sdmc` of the emulator.

## Step 4: Play the music

1. Start the game and open the menu of the plugin.
2. Open the page **Sound**.
3. Select **SD Music**. The menu shows the titles of `list.txt`.
4. Select a title, then press A.

You can press A again with another title, without a change of map. The menu
shows the Bulbapedia name of each music of the game in the entry
**Background Music** in the same way.

## Replace a music of the game

You can also give your file the name of a music of the game. The game then
plays your file each time it plays this music.

1. Find the name in the folder `sound` of the RomFS of the game, for example
   `bgm_sg_art_gallery.dspadpcm.bcstm`.
2. Convert your file.
3. Rename the result to this name, in `sdmc/sango/bgm`.

```bash
python tools/add_music.py "My Song.mp3"
```

```text
my_my_song.dspadpcm.bcstm  ->  bgm_sg_art_gallery.dspadpcm.bcstm
```

The line in `list.txt` does not matter for this use.

## Add the music of Pokémon X and Y

Get the folder `sound` of the RomFS of Pokémon X or Y. It has the `.aac` files.
Convert all the files:

```bash
python tools/aac_to_bcstm.py path/to/xy/sound sdmc/sango/bgm
```

The conversion of about 230 files takes 6 minutes. The tool uses the Bulbapedia
titles of the soundtrack for the list (for example "Lumiose City"). A music without
a known title keeps its name, written in words.

## The limits

| What | Limit |
|---|---|
| Number of music in the list | 400 |
| Length of a title | 45 characters |
| Characters of a title | Letters and numbers (no accents) |
| Music that play at the same time | 1 |

## If it does not work

| Problem | What to do |
|---|---|
| The entry **SD Music** is not in the menu | `list.txt` is not in `sdmc:/sango/bgm/`. Check the folder on the SD card. |
| `ffmpeg is not installed` | Install `ffmpeg`, then open a new terminal. |
| `VGAudioCli is not installed` | Run `python tools/add_music.py --setup`. |
| The music is silent | The file is not a `.bcstm` made by the tool. Convert it again. |
| The music loops at a wrong point | Use `--loop` with the times of the loop. |

## What you learned

- The game plays `.bcstm` files, and `tools/add_music.py` makes them from any audio file.
- The file `list.txt` gives the menu the titles of your music.
- A file with the name of a music of the game replaces this music.

**Next:** [Tutorials](README.md)
