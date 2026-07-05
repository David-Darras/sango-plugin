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

#include <sstream>
#include <string>

#include "system/file.h"

#pragma pack(push, 1)

struct AMXFILEHEADER {
  uint32_t FileSize;
  uint16_t MagicCode;
  uint8_t FileVersion;
  uint8_t AmxVersion;
  uint16_t Flags;
  uint16_t DefSize;
  uint32_t CodeOffset;
  uint32_t DataOffset;
  uint32_t HeapOffset;
  uint32_t StackTopValue;
  uint32_t StartingAddr;
  uint32_t PublicFunc;
  uint32_t NativeFunc;
  uint32_t Lib;
  uint32_t PublicVal;
  uint32_t PublicTags;
  uint32_t SymbolName;
  uint32_t Overlay;
};

struct TABLE {
  uint32_t Address;
  uint32_t NameOffset;
};

#pragma pack(pop)

static std::string ReadAmxString(
    const uint8_t* buffer,
    size_t size,
    uint32_t offset)
{
  std::string result;

  while (offset < size)
  {
    char c = (char)buffer[offset++];

    if (c == '\0')
      break;

    result += c;
  }

  return result;
}

static void DumpTable(
    std::stringstream& out,
    const char* title,
    const uint8_t* buffer,
    size_t size,
    uint32_t table_offset,
    uint32_t count)
{
  out << "\n=== " << title << " ===\n";

  auto table =
      reinterpret_cast<const TABLE*>(
          buffer + table_offset);

  for (uint32_t i = 0; i < count; i++)
  {
    std::string name =
        ReadAmxString(
            buffer,
            size,
            table[i].NameOffset);

    out
        << "[" << i << "] "
        << "Address=0x"
        << std::hex
        << table[i].Address
        << std::dec
        << " Name=\""
        << name
        << "\"\n";
  }
}

bool DumpAmxToTxt(
    const void* amx_buffer,
    size_t amx_size,
    const c16* txt_path)
{
    if (!amx_buffer)
        return false;

    if (amx_size < sizeof(AMXFILEHEADER))
        return false;

    const uint8_t* buffer =
        static_cast<const uint8_t*>(amx_buffer);

    const auto* amx =
        reinterpret_cast<
            const AMXFILEHEADER*>(
                buffer);

    std::stringstream report;

    report << "=== AMX HEADER ===\n";

    report << "FileSize      : "
           << amx->FileSize
           << "\n";

    report << "MagicCode     : 0x"
           << std::hex
           << amx->MagicCode
           << std::dec
           << "\n";

    report << "FileVersion   : "
           << (int)amx->FileVersion
           << "\n";

    report << "AmxVersion    : "
           << (int)amx->AmxVersion
           << "\n";

    report << "Flags         : "
           << amx->Flags
           << "\n";

    report << "CodeOffset    : "
           << amx->CodeOffset
           << "\n";

    report << "DataOffset    : "
           << amx->DataOffset
           << "\n";

    report << "HeapOffset    : "
           << amx->HeapOffset
           << "\n";

    report << "EntryPoint    : "
           << amx->StartingAddr
           << "\n";

    uint32_t public_count =
        (amx->NativeFunc -
         amx->PublicFunc) / 8;

    uint32_t native_count =
        (amx->Lib -
         amx->NativeFunc) / 8;

    uint32_t lib_count =
        (amx->PublicVal -
         amx->Lib) / 8;

    uint32_t pubval_count =
        (amx->PublicTags -
         amx->PublicVal) / 8;

    uint32_t tag_count =
        (amx->SymbolName -
         amx->PublicTags) / 8;

    DumpTable(
        report,
        "PUBLIC FUNCTIONS",
        buffer,
        amx_size,
        amx->PublicFunc,
        public_count);

    DumpTable(
        report,
        "NATIVE FUNCTIONS",
        buffer,
        amx_size,
        amx->NativeFunc,
        native_count);

    DumpTable(
        report,
        "LIBRARIES",
        buffer,
        amx_size,
        amx->Lib,
        lib_count);

    DumpTable(
        report,
        "PUBLIC VALUES",
        buffer,
        amx_size,
        amx->PublicVal,
        pubval_count);

    DumpTable(
        report,
        "PUBLIC TAGS",
        buffer,
        amx_size,
        amx->PublicTags,
        tag_count);

    std::string text = report.str();

    File out(txt_path, true);

    out.Write(
        text.data(),
        text.size());

    return true;
}