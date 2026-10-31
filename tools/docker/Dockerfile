# The tools to build Sango Plugin, without devkitPro on your computer.
#
#   docker build -t sango-build tools/docker
#   docker run --rm -v "$PWD":/project sango-build make myhack
#
# tools/docker_make.sh and tools/docker_make.bat do these two steps for you.
# To use an other version of devkitARM, add
# --build-arg DEVKITARM_IMAGE=devkitpro/devkitarm:<tag> to `docker build`.
ARG DEVKITARM_IMAGE=devkitpro/devkitarm:latest
FROM ${DEVKITARM_IMAGE}

COPY install_tools.sh /tmp/install_tools.sh
RUN sh /tmp/install_tools.sh && rm /tmp/install_tools.sh

# The Makefile calls 3gxtool without its path.
ENV PATH=/opt/devkitpro/tools/bin:${PATH}

# The project folder. `docker run -v` puts your project here.
WORKDIR /project
CMD ["make"]
