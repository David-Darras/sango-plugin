# 4. Set up your editor

An editor with code completion helps you to find the functions of the library.
This guide sets up Visual Studio Code. Other editors use the same settings.

The editor does not build the plugin. You always build with `make`
in the devkitPro shell.

## Visual Studio Code

1. Install [Visual Studio Code](https://code.visualstudio.com).
2. Install the extension **C/C++** (by Microsoft).
3. Open the project folder: **File > Open Folder**.
4. Make the folder `.vscode` in the project folder.
5. Make the file `.vscode/c_cpp_properties.json` with this text:

```json
{
  "configurations": [
    {
      "name": "3DS",
      "compilerPath": "C:/devkitPro/devkitARM/bin/arm-none-eabi-g++.exe",
      "cppStandard": "gnu++11",
      "intelliSenseMode": "gcc-arm",
      "includePath": [
        "${workspaceFolder}/lib/include",
        "${workspaceFolder}/lib/src",
        "${workspaceFolder}/overlay/include",
        "${workspaceFolder}/kaizo/include",
        "${workspaceFolder}/undertow/include",
        "C:/devkitPro/libctru/include",
        "C:/devkitPro/libctrpf/include"
      ],
      "defines": [
        "__3DS__",
        "GAME_ORAS",
        "USE_SANGO_PLUGIN",
        "PLUGIN_NAME=\"Sango\"",
        "PLUGIN_VERSION=\"dev\"",
        "PLUGIN_CREATOR=\"you\""
      ]
    }
  ],
  "version": 4
}
```

On Linux, replace `C:/devkitPro` with `/opt/devkitpro`.
Use `.../devkitARM/bin/arm-none-eabi-g++` without `.exe`.

To work on Pokémon X, replace `GAME_ORAS` with `GAME_XY`.

Git ignores the folder `.vscode`.

## CLion

CLion needs a `CMakeLists.txt` file to understand the project.
This file is only for the code completion. It does not build the plugin.

Make the file `CMakeLists.txt` in the project folder with this text:

```cmake
cmake_minimum_required(VERSION 3.16)
project(sango-plugin)

set(CMAKE_CXX_STANDARD 11)

include_directories(lib/include lib/src
                    overlay/include kaizo/include undertow/include
                    "C:/devkitPro/libctru/include"
                    "C:/devkitPro/libctrpf/include")

add_definitions(-D__3DS__ -DGAME_ORAS -DUSE_SANGO_PLUGIN
                -DPLUGIN_NAME="Sango" -DPLUGIN_VERSION="dev"
                -DPLUGIN_CREATOR="you")

file(GLOB_RECURSE SOURCES lib/*.cc lib/*.h overlay/*.cc overlay/*.h
                          kaizo/*.cc kaizo/*.h undertow/*.cc undertow/*.h)
add_executable(${PROJECT_NAME} ${SOURCES})
```

Git ignores `CMakeLists.txt`.

## The API reference (Doxygen)

The headers of the library have Doxygen comments.
Doxygen makes a website from these comments.

1. Install [Doxygen](https://www.doxygen.nl/download.html).
2. In the project folder, type:

   ```bash
   doxygen
   ```

3. Open `doxygen/html/index.html` in your web browser.

The website shows all the classes, all the functions and all the constants.

---

**Next step:** read [the architecture](../concepts/architecture.md),
then do [the first tutorial](../tutorials/01-create-your-rom-hack.md).
