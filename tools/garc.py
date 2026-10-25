"""Reads the archives (GARC files) of the games and the LZ11 compression.

An archive has numbered files. This module reads them without the game.
"""

import struct


def read_garc(path):
  """Returns the files of an archive, as a list of lists of bytes.

  Most files have one part, so `files[3][0]` is the data of file 3.
  """
  data = open(path, "rb").read()
  if data[:4] != b"CRAG":
    raise ValueError("%s is not an archive" % path)
  header_size, _, _, _, data_offset, _, _ = struct.unpack_from(
      "<IHHIIII", data, 4)
  position = header_size
  if data[position:position + 4] != b"OTAF":
    raise ValueError("%s: the table of files is missing" % path)
  size, count = struct.unpack_from("<IH", data, position + 4)
  offsets = struct.unpack_from("<%dI" % count, data, position + 12)
  position += size
  if data[position:position + 4] != b"BTAF":
    raise ValueError("%s: the table of sizes is missing" % path)
  base = position + 12
  files = []
  for offset in offsets:
    cursor = base + offset
    mask = struct.unpack_from("<I", data, cursor)[0]
    cursor += 4
    parts = []
    for bit in range(32):
      if mask & (1 << bit):
        start, _, length = struct.unpack_from("<III", data, cursor)
        cursor += 12
        parts.append(data[data_offset + start:data_offset + start + length])
    files.append(parts)
  return files


def decompress_lz11(data):
  """Decompresses data that starts with the byte 0x11."""
  if data[0] != 0x11:
    raise ValueError("the data is not LZ11")
  size = data[1] | data[2] << 8 | data[3] << 16
  position = 4
  out = bytearray()
  while len(out) < size:
    flags = data[position]
    position += 1
    for bit in range(8):
      if len(out) >= size:
        break
      if flags & (0x80 >> bit):
        first = data[position]
        kind = first >> 4
        if kind == 0:
          length = ((first & 15) << 4 | data[position + 1] >> 4) + 0x11
          distance = ((data[position + 1] & 15) << 8 | data[position + 2]) + 1
          position += 3
        elif kind == 1:
          length = ((first & 15) << 12 | data[position + 1] << 4
                    | data[position + 2] >> 4) + 0x111
          distance = ((data[position + 2] & 15) << 8
                      | data[position + 3]) + 1
          position += 4
        else:
          length = kind + 1
          distance = ((first & 15) << 8 | data[position + 1]) + 1
          position += 2
        for _ in range(length):
          out.append(out[-distance])
      else:
        out.append(data[position])
        position += 1
  return bytes(out)


def compress_lz11(data, chain=24):
  """Compresses data with LZ11. The result is much smaller than with
  compress_lz11_store: use it for large files, because the game keeps a buffer
  with the size of the compressed file."""
  if len(data) >= 1 << 24:
    raise ValueError("the data is too large")
  out = bytearray([0x11, len(data) & 255, len(data) >> 8 & 255,
                   len(data) >> 16 & 255])
  size = len(data)
  table = {}
  position = 0
  while position < size:
    flags = 0
    block = bytearray()
    for bit in range(8):
      if position >= size:
        break
      best_length = 0
      best_distance = 0
      if position + 3 <= size:
        key = data[position:position + 3]
        candidates = table.get(key)
        if candidates:
          limit = min(size - position, 0x10110)
          for candidate in reversed(candidates[-chain:]):
            distance = position - candidate
            if distance > 4096:
              break
            length = 3
            while length < limit and data[candidate + length] == \
                data[position + length]:
              length += 1
            if length > best_length:
              best_length = length
              best_distance = distance
              if length == limit:
                break
      if best_length >= 3:
        flags |= 0x80 >> bit
        d = best_distance - 1
        if best_length <= 16:
          block.append((best_length - 1) << 4 | d >> 8)
          block.append(d & 255)
        elif best_length <= 0x110:
          n = best_length - 0x11
          block.append(n >> 4)
          block.append((n & 15) << 4 | d >> 8)
          block.append(d & 255)
        else:
          n = best_length - 0x111
          block.append(0x10 | n >> 12)
          block.append(n >> 4 & 255)
          block.append((n & 15) << 4 | d >> 8)
          block.append(d & 255)
        advance = best_length
      else:
        block.append(data[position])
        advance = 1
      for k in range(position, position + advance):
        if k + 3 <= size:
          table.setdefault(data[k:k + 3], []).append(k)
      position += advance
    out.append(flags)
    out += block
  return bytes(out)


def compress_lz11_store(data):
  """Writes LZ11 data without compression (the game reads it as usual)."""
  if len(data) >= 1 << 24:
    raise ValueError("the data is too large")
  out = bytearray([0x11, len(data) & 255, len(data) >> 8 & 255,
                   len(data) >> 16 & 255])
  for start in range(0, len(data), 8):
    out.append(0)
    out += data[start:start + 8]
  return bytes(out)


def unpack_mm(data):
  """Splits a pack of the games ("MM") into its parts."""
  if data[:2] != b"MM":
    raise ValueError("the data is not a pack")
  count = struct.unpack_from("<H", data, 2)[0]
  offsets = struct.unpack_from("<%dI" % (count + 1), data, 4)
  return [data[offsets[i]:offsets[i + 1]] for i in range(count)]


def pack_mm(parts, align=0x80, magic=b"MM"):
  """Writes a pack ("MM") of parts. Each part starts at a multiple of align.

  The packs of the map blocks ("GR") have the same layout: give their magic.
  """
  header_size = (4 + 4 * (len(parts) + 1) + align - 1) // align * align
  offsets = []
  body = bytearray()
  for part in parts:
    offsets.append(header_size + len(body))
    body += part
    body += bytes(-len(body) % align)
  offsets.append(header_size + len(body))
  header = magic + struct.pack("<H", len(parts))
  header += struct.pack("<%dI" % len(offsets), *offsets)
  return header + bytes(header_size - len(header)) + bytes(body)


def extend_motion_pack(pack, count):
  """Gives a pack of motions more slots, without motion in the new slots.

  The pack has the number of motions, one offset for each motion (0 is no
  motion), the size, then the motions. The game asks for the motions by their
  number: a pack made for X and Y has fewer slots than a pack of ORAS.
  """
  old = struct.unpack_from("<I", pack, 0)[0]
  if old >= count:
    return pack
  delta = 4 * (count - old)
  offsets = struct.unpack_from("<%dI" % old, pack, 4)
  size = struct.unpack_from("<I", pack, 4 + 4 * old)[0]
  data = pack[8 + 4 * old:]
  moved = [o + delta if o else 0 for o in offsets] + [0] * (count - old)
  return (struct.pack("<I", count) + struct.pack("<%dI" % count, *moved)
          + struct.pack("<I", size + delta) + data)
