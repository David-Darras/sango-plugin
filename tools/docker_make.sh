#!/bin/sh
# Builds Sango Plugin with Docker. You do not need devkitPro on your computer.
#
#   tools/docker_make.sh myhack        builds your ROM hack (make myhack)
#   tools/docker_make.sh -j8           builds all the products
#   tools/docker_make.sh --update      makes the image again (new tools)
#
# The first time, the script makes the Docker image `sango-build`
# (tools/docker/Dockerfile). This takes some minutes.
# On Windows, use tools\docker_make.bat.
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
IMAGE=sango-build

build_image() {
  docker build --pull -t "$IMAGE" "$ROOT/tools/docker"
}

if [ "$1" = "--update" ]; then
  build_image
  exit 0
fi

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
  build_image
fi

# On Linux and macOS, the files of the build belong to you, not to root.
USER_OPTION="--user $(id -u):$(id -g)"
case "$(uname -s)" in
  MINGW* | MSYS* | CYGWIN*)
    # Git Bash on Windows: keep the paths as they are.
    USER_OPTION=""
    export MSYS_NO_PATHCONV=1
    ;;
esac

# shellcheck disable=SC2086
exec docker run --rm $USER_OPTION -v "$ROOT":/project "$IMAGE" make "$@"
