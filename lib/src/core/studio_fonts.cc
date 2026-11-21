#include "core/patch/studio_fonts.h"
#include <cstring>
#include "battle/patch/studio_rules.h"
#include "core/problems.h"
#include "system/native/file.h"

namespace core {
namespace {

constexpr u32 kHeaderSize = 0x34;
constexpr u32 kChunk = 4096;
constexpr u32 kMaxMatches = 4;
constexpr u32 kMaxWidthBlocks = 64;
constexpr u32 kFontCount = 5;
constexpr u32 kAlign = 0x80;

/// A file that is read at any place. The file keeps the position of the last
/// read, so a read gives the jump from there.
struct Source {
  sys::File file;
  s64 pos = 0;

  bool ReadAt(u32 at, void* out, u32 size) {
    const s32 read = file.Read(out, size, static_cast<s64>(at) - pos);
    pos = static_cast<s64>(at) + size;
    return read == static_cast<s32>(size);
  }
};

u32 Word(const u8* bytes) {
  u32 out;
  std::memcpy(&out, bytes, 4);
  return out;
}

u16 Half(const u8* bytes) {
  u16 out;
  std::memcpy(&out, bytes, 2);
  return out;
}

/// Looks for the fonts in memory: the places that have the header of the file.
/// The first 4 bytes are not compared (the game changes the signature), nor
/// the pointers of the block of the information.
u32 FindFonts(const u8* header, u32** places) {
  static u32 found[kMaxMatches];
  u32 count = 0;
  static const u32 kRanges[2][2] = {{0x08000000, 0x14000000},
                                    {0x14000000, 0x1C000000}};
  for (const auto& range : kRanges) {
    u32 address = range[0];
    while (address < range[1] && count < kMaxMatches) {
      MemInfo info;
      PageInfo page;
      if (R_FAILED(svcQueryMemory(&info, &page, address))) break;
      const u32 start = info.base_addr > address ? info.base_addr : address;
      const u32 end = info.base_addr + info.size;
      if (end <= address) break;
      if ((info.perm & MEMPERM_READ) != 0 && (info.perm & MEMPERM_WRITE) != 0 &&
          info.state != MEMSTATE_FREE) {
        u32 at = (start + kAlign - 1) & ~(kAlign - 1);
        for (; at + kHeaderSize <= end && count < kMaxMatches; at += kAlign) {
          const u8* live = reinterpret_cast<const u8*>(at);
          if (live == header) continue;
          if (std::memcmp(live + 4, header + 4, 0x20) == 0 &&
              std::memcmp(live + 0x30, header + 0x30, 4) == 0) {
            found[count++] = at;
          }
        }
      }
      address = end;
    }
  }
  *places = found;
  return count;
}

/// Copies a part of the file in the font in memory.
bool CopyPart(Source* source, u32 at, u32 size, u32 live) {
  static u8 chunk[kChunk];
  for (u32 done = 0; done < size;) {
    const u32 count = size - done < kChunk ? size - done : kChunk;
    if (!source->ReadAt(at + done, chunk, count)) return false;
    std::memcpy(reinterpret_cast<void*>(live + at + done), chunk, count);
    done += count;
  }
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE, live + at, size);
  return true;
}

/// Writes the file of the font `id` in the font in memory. Returns the number
/// of fonts in memory that changed.
u32 Patch(u32 id) {
  static const c16 kFolder[] = u"sdmc:/sango-studio/font/";
  static const c16 kSuffix[] = u".bin";
  c16 path[96];
  battle::studio::MakePath(kFolder, id, kSuffix, path);
  Source source;
  source.file.Open(path, sys::File::kRead);
  if (!source.file.IsOpen()) return 0;

  static u8 header[kHeaderSize];
  if (!source.ReadAt(0, header, kHeaderSize) ||
      std::memcmp(header, "CFNT", 4) != 0 ||
      std::memcmp(header + 0x14, "FINF", 4) != 0) {
    return 0;
  }
  const u32 size = Word(header + 0x0C);
  const u32 glyphs = Word(header + 0x24);
  const u32 widths = Word(header + 0x28);
  u8 sheets[0x18];
  if (glyphs + sizeof(sheets) > size || !source.ReadAt(glyphs, sheets, sizeof(sheets))) {
    return 0;
  }
  const u32 sheet_bytes = Word(sheets + 4) * Half(sheets + 8);
  const u32 sheet_start = Word(sheets + 0x14);
  if (sheet_start > size || sheet_bytes > size - sheet_start) return 0;

  u32* places = nullptr;
  const u32 count = FindFonts(header, &places);
  u32 changed = 0;
  for (u32 i = 0; i < count; i++) {
    const u32 live = places[i];
    bool ok = CopyPart(&source, sheet_start, sheet_bytes, live);
    // The blocks of widths: the 3 bytes of each glyph.
    u32 at = widths;
    for (u32 block = 0; ok && at != 0 && block < kMaxWidthBlocks; block++) {
      u8 head[8];
      if (at + sizeof(head) > size || !source.ReadAt(at, head, sizeof(head))) {
        ok = false;
        break;
      }
      const u32 first = Half(head);
      const u32 last = Half(head + 2);
      if (last < first || at + 8 + 3 * (last - first + 1) > size) {
        ok = false;
        break;
      }
      ok = CopyPart(&source, at + 8, 3 * (last - first + 1), live);
      at = Word(head + 4);
    }
    if (ok) changed++;
  }
  return changed;
}

} // namespace

void StudioFonts::LoadAll() {
  for (u32 id = 0; id < kFontCount; id++) Patch(id);
}

bool StudioFonts::Owns(const c8* path) {
  return std::memcmp(path, "font/", 5) == 0;
}

void StudioFonts::Apply(const c8* path) {
  const c8* at = path + 5;
  if (*at < '0' || *at > '9') return;
  u32 id = 0;
  while (*at >= '0' && *at <= '9' && id < 1000) id = id * 10 + (*at++ - '0');
  if (id < kFontCount) Patch(id);
}

} // namespace core
