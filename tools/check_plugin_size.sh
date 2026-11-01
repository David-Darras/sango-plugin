#!/bin/sh
# Shows the size of each plugin that make built, and fails when a plugin is
# too big for its memory.
#
# Luma gives a plugin the memory of the MemorySize line of its .plgInfo. The
# code, the data and the bss of the plugin use this memory, and the plugin
# needs the rest for its heap: the check keeps HEAP_KIB kibibytes free (512 by
# default). 3gxtool also warns when the code and the data are bigger than
# 2 MiB: the script shows this warning on GitHub.
#
# Run it after `make`, in the folder of the project:
#   sh tools/check_plugin_size.sh
#   HEAP_KIB=1024 sh tools/check_plugin_size.sh
set -e

SIZE="${DEVKITARM:-/opt/devkitpro/devkitARM}/bin/arm-none-eabi-size"
HEAP_KIB="${HEAP_KIB:-512}"
MIB=1048576

FOUND=0
FAILED=0
for elf in *-release.elf *-debug.elf; do
  [ -f "$elf" ] || continue
  FOUND=1
  target="${elf%-release.elf}"
  target="${target%-debug.elf}"

  # The memory of the plugin: the MemorySize of its .plgInfo (5 MiB without).
  memory_mib=5
  for info in */"$target".plgInfo; do
    [ -f "$info" ] || continue
    value=$(sed -n 's/^MemorySize: *\([0-9][0-9]*\) *MiB.*/\1/p' "$info")
    [ -n "$value" ] && memory_mib="$value"
  done
  limit=$((memory_mib * MIB - HEAP_KIB * 1024))

  # The line of the numbers: text, data, bss, dec, hex, file.
  set -- $("$SIZE" "$elf" | tail -n 1)
  text=$1
  data=$2
  bss=$3
  total=$((text + data + bss))
  echo "$elf: code $text, data $data, bss $bss: $total bytes," \
       "$((total * 100 / limit))% of $limit ($memory_mib MiB - $HEAP_KIB KiB)"

  if [ $((text + data)) -gt $((2 * MIB)) ]; then
    echo "::warning::$elf: the code and the data are bigger than 2 MiB"
  fi
  if [ "$total" -gt "$limit" ]; then
    echo "error: $elf is too big for MemorySize: $memory_mib MiB" >&2
    FAILED=1
  fi
done

if [ "$FOUND" = 0 ]; then
  echo "error: no .elf file: run make first" >&2
  exit 1
fi
exit "$FAILED"
