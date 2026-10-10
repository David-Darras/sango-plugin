/*
 * Copyright (C) 2026  David Darras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file icon.cc
 * @brief The icons of the game (items, Pokémon, types...), drawn by the
 *        menu.
 *
 * The declarations are in ui/widget/icon.h.
 */

#include "ui/widget/icon.h"

#include <3ds.h>
#include <cstdio>
#include <cstring>

#include "core/constant/archive_id.h"
#include "pokemon/patch/model_loader.h"
#include "system/native/graphics.h"

namespace ui {
namespace {
// The footer of a BCLIM image is at the end of the file.
constexpr u32 kFooterSize = 0x28;
constexpr u32 kFooterSignature = 0x4D494C43; // 'CLIM'
constexpr u32 kFooterFileSizeOffset = 0x0C;
constexpr u32 kFooterWidthOffset = 0x1C;
constexpr u32 kFooterHeightOffset = 0x1E;
constexpr u32 kFooterFormatOffset = 0x20;
// The formats of a BCLIM image that the icons use.
constexpr u32 kFormatRgba5551 = 7;
constexpr u32 kFormatRgba4 = 8;
constexpr u32 kFormatEtc1a4 = 11;

// A layout archive: the number of files, a list of names (64 bytes each),
// then the files. The BCLIM images are in the order of the names.
constexpr u32 kLayoutNameSize = 64;

// The texture of the GPU keeps the physical address of its pixels at this
// offset (the address that the GPU reads).
constexpr uptr kTexturePixelsOffset = 0x8;

// The pixels of one texture: 64 x 32, 4 bytes each (A, B, G, R).
constexpr u32 kTextureBytes = IconPool::kWidth * IconPool::kHeight * 4;
u8 g_pixels[kTextureBytes];

// The modifiers of the colors of ETC1 (the two smaller values; the other
// two are the same values with a minus sign).
const s16 kEtc1Modifiers[8][2] = {{2, 8},   {5, 17},  {9, 29},  {13, 42},
                                  {18, 60}, {24, 80}, {33, 106}, {47, 183}};

// The names of the language icons: the index is the language (Language).
const c8* const kLanguageIcons[] = {nullptr, "jpn", "eng", "fra", "ita",
                                    "ger",   nullptr, "spa", "kor"};

// The item of each Poké Ball (pokemon::Ball): 0 for kNone.
const u16 kBallItems[] = {0,   1,   2,   3,   4,   5,   6,   7,   8,
                          9,   10,  11,  12,  13,  14,  15,  16,  492,
                          493, 494, 495, 496, 497, 498, 499, 576};

// The file of the item icon of each item (ItemId), in the archive of the
// item icons.
const u16 kItemIconFiles[776] = {
    629, 0, 1, 2, 3, 4, 5, 6, 7, 8,
    9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
    59, 60, 61, 62, 63, 64, 65, 66, 67, 68,
    69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
    79, 80, 81, 82, 83, 84, 85, 86, 87, 88,
    89, 90, 91, 92, 93, 94, 95, 96, 97, 98,
    99, 100, 101, 102, 103, 104, 105, 106, 107, 108,
    109, 110, 111, 629, 629, 629, 112, 113, 114, 115,
    629, 629, 629, 629, 629, 629, 629, 629, 629, 629,
    629, 629, 629, 629, 116, 117, 118, 119, 120, 121,
    122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
    132, 133, 134, 135, 136, 137, 138, 139, 140, 141,
    142, 143, 144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 154, 155, 156, 157, 158, 159, 160, 161,
    162, 163, 164, 165, 166, 167, 168, 169, 170, 171,
    172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
    182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201,
    202, 203, 204, 205, 206, 207, 208, 209, 210, 211,
    212, 213, 214, 215, 216, 217, 218, 219, 220, 221,
    222, 223, 224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239, 105, 240,
    241, 242, 243, 244, 245, 246, 247, 248, 249, 250,
    251, 252, 253, 254, 255, 256, 257, 258, 259, 260,
    261, 262, 263, 264, 265, 266, 267, 268, 269, 270,
    271, 272, 273, 274, 275, 276, 277, 278, 279, 280,
    281, 282, 283, 284, 285, 286, 287, 288, 289, 290,
    291, 292, 293, 294, 295, 296, 297, 298, 299, 300,
    301, 302, 303, 304, 305, 306, 307, 308, 309, 310,
    311, 311, 312, 313, 314, 315, 313, 312, 316, 309,
    314, 314, 312, 311, 312, 317, 323, 312, 312, 318,
    319, 320, 320, 321, 312, 321, 311, 322, 315, 312,
    311, 313, 316, 313, 319, 316, 319, 323, 309, 312,
    316, 311, 312, 309, 315, 312, 312, 316, 324, 315,
    318, 312, 317, 309, 320, 323, 316, 309, 316, 323,
    309, 312, 322, 309, 312, 312, 319, 312, 319, 320,
    320, 324, 312, 325, 312, 321, 314, 319, 325, 310,
    325, 313, 311, 318, 312, 312, 325, 312, 324, 311,
    326, 327, 328, 326, 328, 588, 629, 629, 329, 330,
    331, 332, 333, 334, 335, 336, 337, 338, 339, 340,
    341, 342, 343, 344, 345, 346, 347, 348, 349, 350,
    351, 352, 353, 354, 355, 356, 357, 358, 359, 360,
    361, 362, 363, 364, 365, 366, 367, 368, 369, 370,
    371, 372, 373, 374, 375, 376, 377, 378, 379, 380,
    381, 382, 383, 384, 385, 386, 387, 388, 389, 390,
    391, 392, 393, 394, 395, 396, 397, 398, 399, 400,
    401, 0, 402, 403, 404, 629, 629, 629, 629, 629,
    629, 629, 629, 629, 629, 629, 629, 629, 629, 629,
    629, 629, 629, 629, 629, 629, 629, 629, 629, 629,
    629, 629, 405, 406, 407, 408, 409, 410, 411, 412,
    413, 414, 415, 416, 417, 418, 419, 420, 421, 422,
    423, 424, 425, 426, 427, 428, 429, 430, 431, 432,
    433, 434, 435, 436, 437, 438, 439, 440, 441, 442,
    443, 444, 445, 446, 447, 448, 449, 450, 451, 452,
    453, 454, 455, 456, 457, 458, 459, 460, 461, 462,
    463, 464, 465, 466, 467, 468, 469, 470, 471, 472,
    473, 474, 475, 476, 477, 478, 479, 480, 481, 482,
    483, 484, 485, 486, 487, 488, 489, 490, 320, 312,
    309, 491, 492, 493, 0, 0, 494, 495, 496, 496,
    497, 498, 499, 500, 501, 502, 491, 494, 503, 504,
    505, 506, 620, 507, 621, 508, 509, 510, 622, 623,
    511, 624, 625, 626, 627, 628, 512, 513, 514, 515,
    516, 517, 518, 519, 520, 521, 522, 523, 524, 525,
    526, 527, 528, 529, 530, 531, 532, 533, 534, 535,
    536, 537, 538, 539, 540, 541, 542, 543, 544, 545,
    312, 309, 315, 546, 312, 547, 548, 549, 550, 551,
    552, 553, 554, 555, 556, 557, 558, 559, 560, 561,
    562, 563, 564, 565, 566, 567, 568, 629, 569, 570,
    571, 572, 573, 574, 575, 576, 577, 578, 579, 580,
    581, 582, 583, 584, 585, 586, 587, 328, 589, 590,
    591, 592, 593, 594, 595, 629, 629, 629, 629, 629,
    629, 596, 597, 598, 599, 600, 601, 602, 603, 604,
    605, 606, 607, 608, 609, 610, 611, 612, 613, 614,
    615, 616, 617, 629, 618, 619,
};

// The file of the Pokémon icon of each species (the male, form 0), in the
// archive of the Pokémon icons.
const u16 kPokemonIconFiles[722] = {
    0, 1, 2, 4, 5, 6, 9, 10, 11, 13,
    14, 15, 16, 17, 18, 20, 21, 22, 24, 25,
    26, 27, 28, 29, 30, 37, 40, 41, 42, 43,
    44, 45, 46, 47, 48, 49, 51, 53, 54, 55,
    57, 59, 60, 61, 62, 63, 64, 65, 66, 67,
    68, 69, 70, 71, 72, 73, 74, 75, 76, 77,
    78, 79, 81, 83, 84, 86, 87, 88, 89, 90,
    91, 92, 93, 94, 95, 96, 97, 98, 99, 100,
    102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 117, 118, 119, 120, 121, 122,
    124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
    134, 135, 136, 137, 138, 140, 141, 142, 143, 144,
    145, 146, 147, 148, 149, 150, 151, 153, 154, 155,
    157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
    167, 168, 170, 171, 172, 173, 174, 175, 176, 177,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189,
    191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
    201, 202, 203, 204, 206, 208, 209, 210, 211, 212,
    213, 215, 216, 217, 218, 219, 220, 222, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234,
    235, 239, 284, 285, 286, 287, 288, 289, 291, 292,
    293, 294, 296, 297, 299, 300, 302, 304, 305, 306,
    307, 308, 309, 310, 311, 312, 313, 314, 315, 317,
    318, 319, 320, 321, 322, 323, 324, 325, 326, 327,
    328, 329, 330, 331, 332, 334, 335, 336, 338, 339,
    340, 341, 342, 343, 345, 346, 347, 349, 350, 351,
    353, 354, 355, 356, 357, 358, 359, 360, 361, 362,
    363, 364, 365, 366, 367, 368, 369, 370, 371, 372,
    373, 374, 376, 377, 378, 379, 380, 381, 382, 383,
    384, 385, 386, 387, 388, 389, 390, 391, 392, 393,
    394, 395, 397, 399, 400, 401, 403, 404, 406, 407,
    409, 410, 411, 412, 413, 414, 416, 417, 418, 420,
    421, 422, 423, 425, 426, 427, 428, 429, 430, 431,
    432, 433, 434, 435, 437, 438, 440, 442, 443, 444,
    445, 446, 447, 448, 449, 450, 451, 452, 453, 454,
    455, 456, 461, 462, 464, 465, 466, 467, 468, 471,
    473, 474, 476, 477, 478, 479, 480, 481, 482, 483,
    484, 485, 486, 488, 489, 490, 492, 493, 495, 496,
    498, 500, 502, 503, 506, 507, 510, 512, 513, 514,
    516, 517, 518, 519, 520, 521, 522, 523, 524, 525,
    526, 527, 528, 529, 530, 531, 532, 534, 536, 537,
    538, 539, 541, 545, 546, 547, 548, 549, 550, 551,
    552, 553, 556, 558, 559, 560, 561, 562, 564, 565,
    566, 567, 568, 569, 570, 571, 572, 573, 574, 575,
    576, 577, 578, 579, 580, 582, 583, 584, 586, 587,
    588, 589, 590, 591, 592, 593, 594, 595, 596, 597,
    599, 600, 601, 602, 603, 604, 605, 606, 608, 610,
    611, 612, 613, 614, 615, 617, 618, 619, 620, 625,
    628, 629, 630, 631, 632, 633, 634, 635, 637, 638,
    639, 640, 641, 644, 645, 646, 647, 648, 649, 650,
    651, 653, 654, 655, 656, 657, 658, 659, 660, 661,
    662, 663, 664, 665, 667, 669, 670, 671, 672, 673,
    674, 676, 677, 678, 679, 680, 681, 682, 683, 684,
    686, 689, 690, 691, 692, 693, 694, 695, 696, 697,
    699, 700, 701, 702, 703, 704, 705, 706, 707, 708,
    710, 712, 713, 714, 715, 716, 718, 719, 720, 721,
    722, 723, 724, 725, 726, 727, 728, 729, 730, 731,
    733, 734, 735, 736, 737, 738, 739, 740, 742, 743,
    744, 745, 746, 747, 748, 751, 755, 758, 759, 760,
    761, 762, 765, 767, 768, 769, 770, 771, 772, 773,
    775, 777, 779, 780, 781, 782, 783, 784, 786, 787,
    788, 789, 790, 791, 792, 793, 794, 795, 796, 797,
    798, 799, 800, 802, 804, 805, 806, 807, 808, 809,
    810, 811, 812, 813, 814, 815, 816, 817, 818, 819,
    820, 822, 824, 825, 826, 828, 829, 835, 839, 845,
    908, 909, 910, 905, 906, 907, 911, 912, 913, 886,
    887, 944, 945, 946, 862, 863, 874, 859, 861, 890,
    896, 901, 917, 918, 943, 919, 851, 922, 924, 934,
    935, 937, 951, 952, 932, 933, 915, 916, 938, 939,
    884, 885, 947, 949, 920, 921, 927, 928, 929, 930,
    961, 955, 931, 960, 957, 958, 959, 953, 857, 858,
    925, 926, 941, 942, 904, 956, 963, 965, 966, 969,
    972, 970,
};

// The place of a pixel in the 8 x 8 tiles of the GPU: the bits of x and y
// alternate (Morton order).
u32 GetTileOffset(u32 x, u32 y) {
  return (x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
         ((x & 4) << 2) | ((y & 4) << 3);
}

// The opposite of GetTileOffset(): the x and the y of a place in a tile.
void GetTilePosition(u32 offset, u32& x, u32& y) {
  x = (offset & 1) | ((offset >> 1) & 2) | ((offset >> 2) & 4);
  y = ((offset >> 1) & 1) | ((offset >> 2) & 2) | ((offset >> 3) & 4);
}

// The smallest size of a texture (a power of 2) that contains `value`.
u32 GetTextureSize(u32 value) {
  u32 size = 8;
  while (size < value) size *= 2;
  return size;
}

// Returns the address that the CPU uses for a physical address of the
// pixels of a texture, or null. The pixels are in FCRAM (the linear memory)
// or in VRAM.
u8* GetPixelAddress(u32 physical) {
  if (physical >= 0x20000000 && physical < 0x28000000) {
    return (u8*)(physical - 0x0C000000); // FCRAM: 0x14000000 for the CPU.
  }
  if (physical >= 0x18000000 && physical < 0x18600000) {
    return (u8*)(physical + 0x07000000); // VRAM: 0x1F000000 for the CPU.
  }
  return nullptr;
}

u16 ReadU16(const u8* data) { return data[0] | (data[1] << 8); }

u32 ReadU32(const u8* data) {
  return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

u64 ReadU64(const u8* data) {
  return (u64)ReadU32(data) | ((u64)ReadU32(data + 4) << 32);
}

u8 Clamp(s32 value) {
  return value < 0 ? 0 : (value > 255 ? 255 : (u8)value);
}

// Writes one pixel into g_pixels.
void WritePixel(u32 x, u32 y, u8 r, u8 g, u8 b, u8 a) {
  const u32 tile = (y / 8) * (IconPool::kWidth / 8) + x / 8;
  u8* target = g_pixels + (tile * 64 + GetTileOffset(x % 8, y % 8)) * 4;
  target[0] = a;
  target[1] = b;
  target[2] = g;
  target[3] = r;
}

// Writes one pixel into g_pixels, from a 16-bit pixel of a BCLIM image.
void WritePixel16(u32 x, u32 y, u16 raw, u32 format) {
  if (format == kFormatRgba4) {
    WritePixel(x, y, ((raw >> 12) & 0xF) * 17, ((raw >> 8) & 0xF) * 17,
               ((raw >> 4) & 0xF) * 17, (raw & 0xF) * 17);
    return;
  }
  const u8 r = (raw >> 11) & 0x1F;
  const u8 g = (raw >> 6) & 0x1F;
  const u8 b = (raw >> 1) & 0x1F;
  WritePixel(x, y, (r << 3) | (r >> 2), (g << 3) | (g >> 2),
             (b << 3) | (b >> 2), (raw & 1) ? 0xFF : 0);
}

/**
 * Decodes one 4 x 4 block of ETC1A4 into g_pixels.
 * @param alpha The 4 bits of alpha of each pixel. The pixel (x, y) is at
 *        the bits (x * 4 + y) * 4.
 * @param color The ETC1 block: two base colors, two modifier tables, and
 *        two bits for each pixel (the bit x * 4 + y and the bit 16 + that).
 * @param (left, top) The place of the block in the texture of the pool.
 * @param (width, height) The size of the visible part of the image, from
 *        (image_x, image_y): the block starts at that place of the image.
 */
void WriteEtc1a4Block(u64 alpha, u64 color, s32 left, s32 top, u32 image_x,
                      u32 image_y, u32 width, u32 height) {
  const bool is_flipped = (color >> 32) & 1;
  const bool is_differential = (color >> 33) & 1;
  s32 base[2][3];
  if (is_differential) {
    for (u32 c = 0; c < 3; c++) {
      const u32 shift = 59 - c * 8;
      const s32 first = (color >> shift) & 0x1F;
      s32 delta = (color >> (shift - 3)) & 0x7;
      if (delta >= 4) delta -= 8;
      const s32 second = (first + delta) & 0x1F;
      base[0][c] = (first << 3) | (first >> 2);
      base[1][c] = (second << 3) | (second >> 2);
    }
  } else {
    for (u32 c = 0; c < 3; c++) {
      const u32 shift = 60 - c * 8;
      base[0][c] = ((color >> shift) & 0xF) * 17;
      base[1][c] = ((color >> (shift - 4)) & 0xF) * 17;
    }
  }
  const u32 tables[2] = {(u32)(color >> 37) & 7, (u32)(color >> 34) & 7};

  for (u32 x = 0; x < 4; x++) {
    for (u32 y = 0; y < 4; y++) {
      if (image_x + x >= width || image_y + y >= height) continue;
      const s32 target_x = left + x;
      const s32 target_y = top + y;
      if (target_x < 0 || target_y < 0 || target_x >= IconPool::kWidth ||
          target_y >= IconPool::kHeight) {
        continue;
      }
      const u32 bit = x * 4 + y;
      const u32 half = is_flipped ? (y >= 2) : (x >= 2);
      const s16* modifiers = kEtc1Modifiers[tables[half]];
      s32 modifier = ((color >> bit) & 1) ? modifiers[1] : modifiers[0];
      if ((color >> (16 + bit)) & 1) modifier = -modifier;
      const u8 a = ((alpha >> (bit * 4)) & 0xF) * 17;
      WritePixel(target_x, target_y, Clamp(base[half][0] + modifier),
                 Clamp(base[half][1] + modifier),
                 Clamp(base[half][2] + modifier), a);
    }
  }
}

// Writes the name (without ".bclim") of the layout icon of a value into
// `name`. Returns the layout archive, or false when the value has no icon.
bool GetLayoutIcon(IconKind kind, u32 id, c8* name, u32 capacity,
                   ArchiveId& archive) {
  switch (kind) {
    case IconKind::kType:
      if (id >= 18) return false;
      archive = ArchiveId::kGraphicFontCommon;
      snprintf(name, capacity, "type_icon_%02lu_", id);
      return true;
    case IconKind::kStatus:
      if (id < 1 || id > 5) return false;
      archive = ArchiveId::kGraphicFontCommon;
      snprintf(name, capacity, "sick_icon_%02lu_", id);
      return true;
    case IconKind::kLanguage:
      if (id >= SIZE(kLanguageIcons) || kLanguageIcons[id] == nullptr) {
        return false;
      }
      archive = ArchiveId::kGraphicFontCommon;
      snprintf(name, capacity, "%s.", kLanguageIcons[id]);
      return true;
    case IconKind::kMoveCategory:
      if (id >= 3) return false;
      archive = ArchiveId::kMoveCategoryIcon;
      snprintf(name, capacity, "damage_type_icon_%02lu_", id);
      return true;
    default:
      return false;
  }
}

bool IsLayoutKind(IconKind kind) {
  return kind == IconKind::kType || kind == IconKind::kStatus ||
         kind == IconKind::kLanguage || kind == IconKind::kMoveCategory;
}
} // namespace

IconPool& IconPool::GetInstance() {
  static IconPool instance;
  return instance;
}

void IconPool::Request(u32 slot, IconKind kind, u32 id) {
  if (slot >= kSlotCount) return;
  slots_[slot].wanted_kind = kind;
  slots_[slot].wanted_id = (u16)id;
  is_used_ = true;
}

IconState IconPool::Draw(u32 slot, IconKind kind, u32 id, s32 x, s32 y,
                         s32 width, s32 height, Color color) const {
  if (slot >= kSlotCount) return IconState::kEmpty;
  const Slot& entry = slots_[slot];
  if (entry.texture == nullptr || entry.kind != kind || entry.id != id) {
    return IconState::kLoading;
  }
  if (!entry.has_image) return IconState::kEmpty;
  sys::Graphics::DrawRectWithTexture(x, y, width, height, color,
                                     entry.texture);
  return IconState::kReady;
}

void IconPool::Prepare(bool can_create_texture) {
  if (!is_used_) return;

  // Make the texture of a slot that the menu uses: one in each frame.
  for (Slot& slot : slots_) {
    if (slot.texture != nullptr || slot.wanted_kind == IconKind::kNone) {
      continue;
    }
    if (!can_create_texture) break;
    memset(g_pixels, 0, sizeof(g_pixels));
    slot.texture = sys::Graphics::CreateTexture(kWidth, kHeight, g_pixels);
    break;
  }

  // Load the icons that changed.
  IconKind layout_kind = IconKind::kNone;
  u32 loads = 0;
  for (Slot& slot : slots_) {
    if (slot.texture == nullptr || slot.IsDone()) continue;
    if (slot.wanted_kind == IconKind::kNone) {
      slot.SetEmpty();
    } else if (IsLayoutKind(slot.wanted_kind)) {
      if (layout_kind == IconKind::kNone) layout_kind = slot.wanted_kind;
    } else if (loads < kLoadsPerFrame) {
      loads++;
      LoadFileIcon(slot);
    }
  }
  // A layout archive is a large file: read it alone in its frame.
  if (layout_kind != IconKind::kNone && loads == 0) {
    LoadLayoutIcons(layout_kind);
  }
}

void IconPool::LoadFileIcon(Slot& slot) {
  ArchiveId archive;
  u32 file_id;
  u32 item = slot.wanted_id;
  switch (slot.wanted_kind) {
    case IconKind::kBall:
      if (item >= SIZE(kBallItems)) {
        slot.SetEmpty();
        return;
      }
      // A ball shows the icon of its item.
      item = kBallItems[item];
      // fall through
    case IconKind::kItem:
      if (item == 0 || item >= SIZE(kItemIconFiles)) {
        slot.SetEmpty();
        return;
      }
      archive = ArchiveId::kItemIcon;
      file_id = kItemIconFiles[item];
      break;
    case IconKind::kPokemon:
      // Species 0 has an icon with an other format.
      if (item == 0 || item >= SIZE(kPokemonIconFiles)) {
        slot.SetEmpty();
        return;
      }
      archive = ArchiveId::kPokemonIcon;
      file_id = kPokemonIconFiles[item];
      break;
    default:
      slot.SetEmpty();
      return;
  }

  u32 size = 0;
  auto* file = (u8*)pokemon::ModelLoader::ReadFile(archive, file_id, true,
                                                   &size);
  if (file == nullptr || !WriteImage(slot, file, size, true)) {
    slot.SetEmpty();
  }
  if (file != nullptr) pokemon::ModelLoader::FreeBuffer(file);
}

void IconPool::LoadLayoutIcons(IconKind kind) {
  // The archive of the icons of `kind`: the same for all its values.
  ArchiveId archive = ArchiveId::kGraphicFontCommon;
  c8 name[32];
  bool has_archive = false;
  for (Slot& slot : slots_) {
    if (slot.wanted_kind != kind || slot.IsDone()) continue;
    if (!GetLayoutIcon(kind, slot.wanted_id, name, sizeof(name), archive)) {
      slot.SetEmpty();
    } else {
      has_archive = true;
    }
  }
  if (!has_archive) return;

  u32 size = 0;
  auto* file = (u8*)pokemon::ModelLoader::ReadFile(archive, 0, true, &size);
  const u32 name_count = file != nullptr && size >= 4 ? ReadU32(file) : 0;
  const u32 names_end = 4 + name_count * kLayoutNameSize;
  if (names_end > size) {
    for (Slot& slot : slots_) {
      if (slot.wanted_kind == kind && !slot.IsDone()) slot.SetEmpty();
    }
    if (file != nullptr) pokemon::ModelLoader::FreeBuffer(file);
    return;
  }

  for (Slot& slot : slots_) {
    if (slot.wanted_kind != kind || slot.IsDone()) continue;
    GetLayoutIcon(kind, slot.wanted_id, name, sizeof(name), archive);
    // Find the index of the name, then the image with the same index.
    u32 index = name_count;
    for (u32 i = 0; i < name_count; i++) {
      if (strncmp((const c8*)file + 4 + i * kLayoutNameSize, name,
                  strlen(name)) == 0) {
        index = i;
        break;
      }
    }
    bool is_written = false;
    u32 image = 0;
    for (u32 offset = names_end;
         index < name_count && offset + kFooterSize <= size; offset += 4) {
      if (ReadU32(file + offset) != kFooterSignature) continue;
      if (image++ != index) continue;
      const u32 image_size = ReadU32(file + offset + kFooterFileSizeOffset);
      const u32 end = offset + kFooterSize;
      if (image_size <= end) {
        is_written = WriteImage(slot, file + end - image_size, image_size,
                                false);
      }
      break;
    }
    if (!is_written) slot.SetEmpty();
  }
  pokemon::ModelLoader::FreeBuffer(file);
}

bool IconPool::WriteImage(Slot& slot, const u8* file, u32 size,
                          bool is_palette) {
  if (slot.texture == nullptr || size <= kFooterSize) return false;
  const u8* footer = file + size - kFooterSize;
  if (ReadU32(footer) != kFooterSignature) return false;
  const u32 width = ReadU16(footer + kFooterWidthOffset);
  const u32 height = ReadU16(footer + kFooterHeightOffset);
  const u32 format = ReadU32(footer + kFooterFormatOffset);
  const u32 texture_width = GetTextureSize(width);
  const u32 texture_height = GetTextureSize(height);
  const u32 data_size = size - kFooterSize;

  // The icon goes in the center of the texture.
  const s32 offset_x = ((s32)kWidth - (s32)width) / 2;
  const s32 offset_y = ((s32)kHeight - (s32)height) / 2;
  memset(g_pixels, 0, sizeof(g_pixels));

  if (format == kFormatEtc1a4) {
    // Blocks of 4 x 4 pixels (8 bytes of alpha, 8 bytes of color). Four
    // blocks make a tile of 8 x 8: top-left, top-right, bottom-left,
    // bottom-right.
    if (data_size < texture_width * texture_height) return false;
    const u8* block = file;
    for (u32 tile_y = 0; tile_y < texture_height; tile_y += 8) {
      for (u32 tile_x = 0; tile_x < texture_width; tile_x += 8) {
        for (u32 i = 0; i < 4; i++, block += 16) {
          const u32 x = tile_x + (i & 1) * 4;
          const u32 y = tile_y + (i >> 1) * 4;
          WriteEtc1a4Block(ReadU64(block), ReadU64(block + 8), x + offset_x,
                           y + offset_y, x, y, width, height);
        }
      }
    }
  } else if (format == kFormatRgba5551 || format == kFormatRgba4) {
    const u32 pixel_count = data_size / 2;
    // An image with a palette: two u16 (the size of one color, the number
    // of colors), the colors, then one index for each pixel (4 bits with 16
    // colors or less, else 8 bits). The first pixel is in the high bits.
    const u8* colors = nullptr;
    const u8* indexes = nullptr;
    u32 color_count = 0;
    if (is_palette) {
      const u32 color_size = ReadU16(file);
      color_count = ReadU16(file + 2);
      if (color_size != 2 || color_count == 0) return false;
      colors = file + 4;
      indexes = colors + color_size * color_count;
      const u32 index_bytes =
          color_count <= 16 ? (pixel_count + 1) / 2 : pixel_count;
      if (indexes + index_bytes > file + data_size) return false;
    }

    for (u32 i = 0; i < pixel_count; i++) {
      u32 x;
      u32 y;
      GetTilePosition(i % 64, x, y);
      const u32 tile = i / 64;
      x += (tile % (texture_width / 8)) * 8;
      y += (tile / (texture_width / 8)) * 8;
      if (x >= width || y >= height) continue;
      const s32 target_x = (s32)x + offset_x;
      const s32 target_y = (s32)y + offset_y;
      if (target_x < 0 || target_y < 0 || target_x >= kWidth ||
          target_y >= kHeight) {
        continue;
      }

      u16 raw;
      if (!is_palette) {
        raw = ReadU16(file + i * 2);
      } else {
        u32 color = color_count <= 16
                      ? (indexes[i / 2] >> ((i & 1) ? 0 : 4)) & 0xF
                      : indexes[i];
        if (color >= color_count) color = 0;
        raw = ReadU16(colors + color * 2);
      }
      WritePixel16(target_x, target_y, raw, format);
    }
  } else {
    return false;
  }

  // Replace the pixels of the texture, then write the cache of the CPU to
  // the memory: the GPU reads the memory.
  u8* pixels = GetPixelAddress(
      *(u32*)((uptr)slot.texture + kTexturePixelsOffset));
  if (pixels == nullptr) return false;
  memcpy(pixels, g_pixels, kTextureBytes);
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE, (u32)pixels, kTextureBytes);
  slot.kind = slot.wanted_kind;
  slot.id = slot.wanted_id;
  slot.has_image = true;
  return true;
}
} // namespace ui
