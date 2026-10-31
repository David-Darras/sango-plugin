#!/bin/sh
# Installs libctrpf and 3gxtool in the devkitARM image of devkitPro.
#
# The Dockerfile of this folder and the GitHub workflow
# (.github/workflows/build.yml) use this script. The image already has
# devkitARM, libctru and make.
set -e

# The package manager of devkitPro: dkp-pacman, or pacman in some images.
if command -v dkp-pacman >/dev/null 2>&1; then
  PACMAN=dkp-pacman
else
  PACMAN=pacman
fi
CONF="${DEVKITPRO:-/opt/devkitpro}/pacman/etc/pacman.conf"

# The package source of libctrpf and 3gxtool (ThePixellizerOSS).
if ! grep -q "thepixellizeross-lib" "$CONF"; then
  cat >> "$CONF" <<'EOF'

[thepixellizeross-lib]
Server = https://thepixellizeross.gitlab.io/packages/any
SigLevel = Optional

[thepixellizeross-linux]
Server = https://thepixellizeross.gitlab.io/packages/x86_64/linux
SigLevel = Optional
EOF
fi

$PACMAN -Sy --noconfirm
$PACMAN -S --noconfirm --needed libctrpf 3gxtool

# Stop now if a tool is missing, not in the middle of a build.
test -x "${DEVKITPRO:-/opt/devkitpro}/tools/bin/3gxtool" || {
  echo "error: 3gxtool is not installed" >&2
  exit 1
}
test -f "${DEVKITPRO:-/opt/devkitpro}/libctrpf/lib/libctrpf.a" || {
  echo "error: libctrpf is not installed" >&2
  exit 1
}
