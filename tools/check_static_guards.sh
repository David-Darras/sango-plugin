#!/bin/sh
# Fails when a singleton has a new guard.
#
# A singleton with a member that is not constant (no constexpr constructor)
# gets a guard: the program builds it at the first call of GetInstance(). The
# guard calls abort() when the game and the plugin call GetInstance() at the
# same time (see lib/include/core/singleton.h). tests/static_guards.txt lists
# the singletons that already have a guard.
#
# Run it after `make`, in the folder of the project:
#   sh tools/check_static_guards.sh
set -e

NM="${DEVKITARM:-/opt/devkitpro/devkitARM}/bin/arm-none-eabi-nm"
LIST="tests/static_guards.txt"

OBJECTS=$(find . -path './release-*' -name '*.o' -o -path './debug-*' -name '*.o')
if [ -z "$OBJECTS" ]; then
  echo "error: no object file: run make first" >&2
  exit 1
fi

FOUND=$("$NM" -C $OBJECTS 2>/dev/null |
  grep 'guard variable for .*::GetInstance()::instance' |
  sed 's/.*guard variable for //; s/::GetInstance()::instance//' | sort -u)

NEW=$(echo "$FOUND" | grep -v '^$' | grep -vxF -f "$LIST" || true)
if [ -n "$NEW" ]; then
  echo "error: these singletons have a new guard (see core/singleton.h):" >&2
  echo "$NEW" >&2
  exit 1
fi
echo "The singletons have no new guard."
