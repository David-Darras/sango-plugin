# Tests

The project has two kinds of automatic checks. GitHub runs them for each push
(`.github/workflows/build.yml`).

## The tests on a computer

The tests of `tests/` check the code that does not need the game:
`core::FixedList` and the UTF-8 texts. They use the `g++` of your computer:
you do not need devkitPro or the game.

```bash
make -C tests
```

| File | What it tests |
|---|---|
| `tests/test_core.cc` | `core::FixedList` and `core::Utf8ToUtf16`. |

### Write a test

```cpp
#include "test.h"

TEST(AddsTwoNumbers) {
  CHECK_EQ(1 + 1, 2);
  CHECK(1 < 2);
}
```

A test file includes only pure files: the files that include `core/types.h`
and no game structure (no `common.h`). Move the logic that you want to test
into such a file, like `core/utf8.cc`.

## The check of the singletons

After the build, `tools/check_static_guards.sh` checks that no singleton has
a new guard. A singleton with a member that is not constant gets a guard,
and the guard can call `abort()` at the start of the game (see
`core/singleton.h`). `tests/static_guards.txt` lists the singletons that
already have a guard.

```bash
make
sh tools/check_static_guards.sh
```

## The check of the size

After the build, `tools/check_plugin_size.sh` shows the size of each plugin:
the code, the data and the bss. Luma gives a plugin the memory of the
`MemorySize` line of its `.plgInfo` (5 MiB). The check fails when a plugin
uses more than this memory minus 512 KiB for its heap. It also shows a
warning when the code and the data are bigger than 2 MiB, like 3gxtool.

```bash
make
sh tools/check_plugin_size.sh
HEAP_KIB=1024 sh tools/check_plugin_size.sh   # Keep 1 MiB for the heap.
```

The tests cannot check the hooks, the menu or the game itself: test them in
the game.
