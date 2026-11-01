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
 * @file test_maker_files.cc
 * @brief Reads the example files of maker/data and the files of
 * tests/maker_cases. The editor of tools/maker_editor reads the same files
 * (tests/maker_editor_test.js): the two must find the same mistakes.
 *
 * In a file of tests/maker_cases, a line with the comment "# ERR" must have a
 * mistake, and the other lines must have no mistake.
 */

#include <cstdio>
#include <set>
#include <string>

#include "core/maker_reader.h"
#include "test.h"

namespace {
using namespace core::maker;

struct ProblemLines : Sink {
  std::set<u32> lines;
  void OnProblem(const c8* file, u32 line, const c8* what,
                 const c8* word) override {
    lines.insert(line);
    std::printf("      %s %u: %s %s\n", file, line, what, word);
  }
};

std::string ReadFile(const std::string& path) {
  std::string text;
  FILE* file = std::fopen(path.c_str(), "rb");
  if (file == nullptr) return text;
  char buffer[4096];
  size_t size;
  while ((size = std::fread(buffer, 1, sizeof(buffer), file)) > 0) {
    text.append(buffer, size);
  }
  std::fclose(file);
  return text;
}

// The numbers of the lines with the comment "# ERR".
std::set<u32> ExpectedLines(const std::string& text) {
  std::set<u32> lines;
  u32 number = 1;
  size_t start = 0;
  while (start <= text.size()) {
    size_t end = text.find('\n', start);
    if (end == std::string::npos) end = text.size();
    if (text.compare(start, end - start, "") != 0 &&
        text.substr(start, end - start).find("# ERR") != std::string::npos) {
      lines.insert(number);
    }
    number++;
    start = end + 1;
  }
  return lines;
}

typedef void (*ReadFunction)(const c8* text, u32 size, Sink& sink);

const struct {
  const char* name;
  ReadFunction read;
} kFiles[] = {
    {"encounters.txt", ReadEncounters}, {"trainers.txt", ReadTrainers},
    {"species.txt", ReadSpecies},       {"story.txt", ReadStory},
    {"characters.txt", ReadCharacters},
};

// Checks the lines of the mistakes of one file.
void CheckFile(const std::string& path, ReadFunction read) {
  const std::string text = ReadFile(path);
  CHECK(!text.empty());
  Clear();
  ProblemLines problems;
  read(text.data(), static_cast<u32>(text.size()), problems);
  const std::set<u32> expected = ExpectedLines(text);
  if (problems.lines != expected) {
    std::printf("    %s: the lines of the mistakes are not the expected lines\n",
                path.c_str());
  }
  CHECK(problems.lines == expected);
}
} // namespace

TEST(TheExampleFilesOfMakerHaveNoMistake) {
  for (const auto& file : kFiles) {
    CheckFile(std::string(SANGO_ROOT "/maker/data/") + file.name, file.read);
  }
}

TEST(TheCasesFindTheSameMistakesAsTheEditor) {
  for (const auto& file : kFiles) {
    CheckFile(std::string(SANGO_ROOT "/tests/maker_cases/") + file.name,
              file.read);
  }
}
