# 6. Build without devkitPro

You can build the plugin without devkitPro, libctrpf and 3gxtool on your
computer. Select one of these two methods:

| Method | You need | Use it when |
|---|---|---|
| [GitHub](#method-1-build-on-github) | A GitHub account and a web browser. | You do not want to install any tool. |
| [Docker](#method-2-build-with-docker) | Docker Desktop (Windows, macOS) or Docker (Linux). | You build often, on your computer. |

Both methods use the same tools as a normal build. The `.3gx` files are the same.

---

## Method 1: Build on GitHub

GitHub builds the plugin each time that you send a change.
The file `.github/workflows/build.yml` gives the steps.

### Step 1: Make your copy of the project

1. Sign in to [GitHub](https://github.com).
2. Go to the [Sango Plugin page](https://github.com/David-Darras/sango-plugin).
3. Click **Fork**, then click **Create fork**.

You now have your own copy of the project.

### Step 2: Enable the workflows

1. In your copy, click the **Actions** tab.
2. Click **I understand my workflows, go ahead and enable them**.

### Step 3: Add your ROM hack

Make the files of your ROM hack. Two methods:

- On your computer: clone your copy with Git, type
  `python tools/new_hack.py myhack`, then commit and push the new folder.
- In the web browser: click **Add file > Create new file**. Type the name
  `myhack/src/entrypoint.cc`, write the code, then click **Commit changes**.
  Do the same for `myhack/myhack.plgInfo`. Copy the contents of the two files
  from [Tutorial 1](../tutorials/01-create-your-rom-hack.md).

### Step 4: Get the plugin file

1. Click the **Actions** tab. Each change has a run of the workflow **Build**.
2. Click the last run. Wait for the green check mark (some minutes).
3. At the bottom of the page, in **Artifacts**, click **plugins**.
4. GitHub downloads `plugins.zip`. It contains the `.3gx` files of all the products.

If the run has a red cross, click it: the log shows the error of the compiler.

### Build one product only

All the products take more time. To build only your ROM hack:

1. Click the **Actions** tab, then the workflow **Build**.
2. Click **Run workflow**. Type the name of your product (for example `myhack`).
3. Click the green **Run workflow** button.

---

## Method 2: Build with Docker

Docker runs a small Linux system with all the tools. The file
`tools/docker/Dockerfile` makes this system (the image `sango-build`).

### Step 1: Install Docker

- Windows and macOS: install [Docker Desktop](https://www.docker.com/products/docker-desktop/).
  Start it before each build.
- Linux: install Docker with the package manager of your distribution.

### Step 2: Build

Open a terminal in the project folder.

On Windows (Command Prompt or PowerShell):

```bat
tools\docker_make.bat myhack
```

On Linux and macOS:

```bash
sh tools/docker_make.sh myhack
```

The first time, the script makes the image `sango-build`. This takes some
minutes. The next builds start at once.

The arguments are the arguments of `make`:

| Command | What it does |
|---|---|
| `tools\docker_make.bat myhack` | Builds your ROM hack. |
| `tools\docker_make.bat -j8` | Builds all the products, 8 files at the same time. |
| `tools\docker_make.bat clean` | Removes all the build output. |
| `tools\docker_make.bat --update` | Makes the image again, with the latest tools. |

The `.3gx` files are in the project folder, as with a normal build.

> **Note:** `make run-<product>` does not work in Docker: Docker cannot start
> the emulator of your computer. Copy the `.3gx` file yourself
> (see [Run the plugin](03-run-the-plugin.md)).

---

**Back to:** [1. Install the tools](01-install-the-tools.md)
