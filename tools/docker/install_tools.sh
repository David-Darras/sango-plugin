#!/bin/sh
# Installs 3gxtool in the devkitARM image of devkitPro.
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

# The server of devkitPro refuses the automatic downloads (error 403 on
# GitHub), and the image already has its packages. A second configuration
# keeps the [options] of the image and has only the package source of
# 3gxtool (ThePixellizerOSS).
ONLY_TPO=/tmp/pacman-thepixellizeross.conf
awk '/^\[/ { keep = ($0 == "[options]") } keep' "$CONF" > "$ONLY_TPO"
cat >> "$ONLY_TPO" <<'EOF'

[thepixellizeross-lib]
Server = https://thepixellizeross.gitlab.io/packages/any
SigLevel = Optional

[thepixellizeross-linux]
Server = https://thepixellizeross.gitlab.io/packages/x86_64/linux
SigLevel = Optional
EOF

$PACMAN --config "$ONLY_TPO" -Sy --noconfirm
$PACMAN --config "$ONLY_TPO" -S --noconfirm --needed 3gxtool
rm -f "$ONLY_TPO"

# Stop now if a tool is missing, not in the middle of a build.
test -x "${DEVKITPRO:-/opt/devkitpro}/tools/bin/3gxtool" || {
  echo "error: 3gxtool is not installed" >&2
  exit 1
}
