"""Finds ffmpeg and VGAudioCli, and converts an audio file to a .bcstm.

The game plays its music from DSP-ADPCM ".bcstm" files. ffmpeg reads any
audio format (mp3, ogg, flac, wav, aac...). VGAudioCli writes the .bcstm.
The scripts add_music.py and aac_to_bcstm.py use this module.
"""

import os
import shutil
import subprocess
import sys
import tempfile
import urllib.request
import wave
import zipfile

EXTENSION = ".dspadpcm.bcstm"
HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "bin")
VGAUDIO_URL = ("https://github.com/Thealexbarney/VGAudio/releases/download/"
               "v2.2.1/VGAudioCli.zip")
# The sample rate of the music of the game is about 32 kHz.
SAMPLE_RATE = 32000


def find_ffmpeg(explicit=None):
  """Returns the path of ffmpeg."""
  if explicit:
    return explicit
  found = shutil.which("ffmpeg")
  if found:
    return found
  sys.exit("ffmpeg is not installed. Windows: winget install Gyan.FFmpeg. "
           "Then open a new terminal. See docs/tutorials/13-add-your-own-music.md.")


def vgaudio_command(explicit=None):
  """Returns the command that runs VGAudioCli, as a list."""
  if explicit:
    return [explicit]
  found = shutil.which("VGAudioCli")
  if found:
    return [found]
  if os.name == "nt":
    exe = os.path.join(BIN, "VGAudio", "net451_standalone", "VGAudioCli.exe")
    if os.path.isfile(exe):
      return [exe]
  else:
    dll = os.path.join(BIN, "VGAudio", "netcoreapp2.0", "VGAudioCli.dll")
    if os.path.isfile(dll):
      return ["dotnet", dll]
  sys.exit("VGAudioCli is not installed. Run: python tools/add_music.py --setup")


def setup_vgaudio():
  """Downloads VGAudioCli (about 0.8 MB) from the page of its author."""
  os.makedirs(BIN, exist_ok=True)
  archive = os.path.join(BIN, "VGAudioCli.zip")
  print("Downloading", VGAUDIO_URL)
  urllib.request.urlretrieve(VGAUDIO_URL, archive)
  with zipfile.ZipFile(archive) as zipped:
    zipped.extractall(os.path.join(BIN, "VGAudio"))
  os.remove(archive)
  print("VGAudioCli is ready in", os.path.join(BIN, "VGAudio"))


def convert(source, out_path, loop=None, ffmpeg=None, vgaudio=None,
            sample_rate=SAMPLE_RATE):
  """Converts an audio file to a stereo .bcstm.

  `loop` is (start, end) in seconds, "full" for the whole music, or None for
  a music that plays one time.
  """
  ffmpeg = find_ffmpeg(ffmpeg)
  command = vgaudio_command(vgaudio)
  with tempfile.TemporaryDirectory() as temp:
    wav_path = os.path.join(temp, "music.wav")
    subprocess.run([ffmpeg, "-y", "-loglevel", "error", "-i", source, "-vn",
                    "-ac", "2", "-ar", str(sample_rate), "-acodec",
                    "pcm_s16le", wav_path], check=True)
    with wave.open(wav_path, "rb") as wav:
      frames = wav.getnframes()
    arguments = []
    if loop == "full":
      arguments = ["-l", "0-%d" % frames]
    elif loop:
      start = int(loop[0] * sample_rate)
      end = min(int(loop[1] * sample_rate), frames)
      if not 0 <= start < end:
        sys.exit("The loop points are not inside the music (%.2f s long)."
                 % (frames / sample_rate))
      arguments = ["-l", "%d-%d" % (start, end)]
    else:
      arguments = ["--no-loop"]
    subprocess.run(command + ["-i", wav_path, "-o", out_path] + arguments,
                   check=True, stdout=subprocess.DEVNULL)
