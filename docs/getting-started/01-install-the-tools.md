# 1. Install the tools

This guide starts from a computer with no tools.
At the end, you can build the plugin.

You need these tools:

| Tool | What it does |
|---|---|
| **devkitPro** (devkitARM + libctru) | Compiles C++ code for the Nintendo 3DS. |
| **libctrpf** | The CTRPluginFramework library. The plugin uses it to run inside the game. |
| **3gxtool** | Changes the compiled program into a `.3gx` plugin file. |
| **Git** | Downloads the source code and keeps the history of your changes. |
| **Azahar** (optional) | A 3DS emulator. You can test the plugin on your computer. |

Time: about 20 minutes.

- [Windows](#windows)
- [Linux (Debian, Ubuntu)](#linux-debian-ubuntu)
- [macOS](#macos)
- [Check the installation](#check-the-installation)

---

## Windows

### Step 1: Install devkitPro

1. Go to the [devkitPro installer page](https://github.com/devkitPro/installer/releases).
2. Download the latest `devkitProUpdater-x.x.x.exe` file.
3. Run the file.
4. On the component page, select **3DS Development**.
5. Keep the default folder: `C:\devkitPro`.
6. Complete the installation. The installer downloads the packages. This can take some minutes.

> **Note:** Do not install devkitPro in a folder with spaces in its name.

### Step 2: Open the devkitPro shell

The devkitPro shell is a terminal. You use it for all the next commands.

1. Open the folder `C:\devkitPro\msys2`.
2. Double-click `msys2_shell.bat`.

A black window opens. This is the devkitPro shell.

> **Tip:** The Start menu also has a shortcut: **devkitPro > MSYS2**.

In this shell, the disk `C:\` is the folder `/c/`.
For example, `C:\Users\Ash` is `/c/Users/Ash`.

### Step 3: Add the package source of libctrpf and 3gxtool

The team ThePixellizerOSS supplies libctrpf and 3gxtool.
You must tell `pacman` (the package manager) where to find them.

1. Open the file `C:\devkitPro\msys2\etc\pacman.conf` in a text editor (for example Notepad).
2. Add these lines at the end of the file:

   ```ini
   [thepixellizeross-lib]
   Server = https://thepixellizeross.gitlab.io/packages/any
   SigLevel = Optional

   [thepixellizeross-win]
   Server = https://thepixellizeross.gitlab.io/packages/x86_64/win
   SigLevel = Optional
   ```

3. Save the file.

### Step 4: Install libctrpf, 3gxtool and Git

In the devkitPro shell, type these commands. Press **Enter** after each command.

```bash
pacman -Sy
```

```bash
pacman -S libctrpf 3gxtool git
```

When `pacman` asks a question, type `Y` and press **Enter**.

> **Note:** `pacman -Sy` downloads the list of packages. If you do not run it,
> `pacman` does not find libctrpf.

Go to [Check the installation](#check-the-installation).

---

## Linux (Debian, Ubuntu)

### Step 1: Install devkitPro

Open a terminal. Type these commands:

```bash
wget https://apt.devkitpro.org/install-devkitpro-pacman
```

```bash
chmod +x ./install-devkitpro-pacman
```

```bash
sudo ./install-devkitpro-pacman
```

```bash
sudo dkp-pacman -S 3ds-dev
```

When `dkp-pacman` asks a question, press **Enter** to select all the packages.

Then close the terminal and open a new terminal.
The new terminal knows the variables `DEVKITPRO` and `DEVKITARM`.

> For other Linux distributions, read the
> [devkitPro getting started page](https://devkitpro.org/wiki/Getting_Started).

### Step 2: Add the package source of libctrpf and 3gxtool

1. Open the file `/opt/devkitpro/pacman/etc/pacman.conf` as an administrator:

   ```bash
   sudo nano /opt/devkitpro/pacman/etc/pacman.conf
   ```

2. Add these lines at the end of the file:

   ```ini
   [thepixellizeross-lib]
   Server = https://thepixellizeross.gitlab.io/packages/any
   SigLevel = Optional

   [thepixellizeross-linux]
   Server = https://thepixellizeross.gitlab.io/packages/x86_64/linux
   SigLevel = Optional
   ```

3. Save the file (**Ctrl + O**, **Enter**) and close the editor (**Ctrl + X**).

### Step 3: Install libctrpf, 3gxtool, Git and make

```bash
sudo dkp-pacman -Sy
```

```bash
sudo dkp-pacman -S libctrpf 3gxtool
```

```bash
sudo apt install git make
```

Go to [Check the installation](#check-the-installation).

---

## macOS

devkitPro supports macOS. ThePixellizerOSS does not supply a 3gxtool package for macOS.
You must compile 3gxtool from its [source code](https://gitlab.com/thepixellizeross/3gxtool).

The project does not test macOS. Use Windows or Linux if you can.

---

## Check the installation

Type each command in the devkitPro shell (Windows) or in a terminal (Linux).

```bash
arm-none-eabi-gcc --version
```

The result starts with `arm-none-eabi-gcc`.

```bash
3gxtool
```

The result starts with `3DS Game eXtension Tool`.

```bash
ls $DEVKITPRO/libctrpf/lib
```

The result shows `libctrpf.a` and `libctrpfd.a`.

```bash
git --version
```

The result starts with `git version`.

If a command shows "command not found", do the steps of your system again.
Then read [Troubleshooting](05-troubleshooting.md).

---

## Install the Azahar emulator (optional)

Azahar is a 3DS emulator. It can load `.3gx` plugins.

1. Go to the [Azahar website](https://azahar-emu.org).
2. Download the version for your system.
3. Install it.

You can also test the plugin on a real console with Luma3DS.
[Run the plugin](03-run-the-plugin.md) explains the two methods.

---

**Next step:** [2. Build the plugin](02-build-the-plugin.md)
