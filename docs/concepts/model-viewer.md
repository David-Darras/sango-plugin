# The 3D models of the screen

Each character, Pokémon and object that the game draws in 3D is a model. A
model has parts and materials.

- A **part** is a mesh that the game can show or hide, for example a hat.
- A **material** is the look of a surface: its colors, its culling and its alpha
  test.

`renderer::ModelViewer` keeps a list of the models that the game puts in its
scenes. It removes a model from the list when the game destroys it. It works
with Alpha Sapphire and Omega Ruby only.

## The menu

Open **Graphics > Model Viewer**.

1. The first page lists the models of the screen.
2. Open a model. **Visible** hides the whole model. Each part has an entry:
   turn it off to hide it. **Show All Parts** and **Hide All Parts** change all
   the parts.
3. Open a material. Change its ambient color, its diffuse color, its culling
   and its alpha test.

If you do not know which model is which, hide it: the model that leaves the
screen is the one. The position of the model also helps.

A change stays until the game destroys the model. For example, a new battle
makes new models.

## The textures

The game makes the textures of the Pokémon while it runs, in the memory of the
GPU. A material page lists the textures of its three units. For a texture that
the game made, the entry is **Paint Unit N**.

The paint program opens on the bottom screen. The stylus draws on the texture,
and the model on the top screen changes at once.

- **Palette**: touch a color.
- **- / +**: the size of the brush.
- **Paint / Pick / Erase**: the tool. Pick takes the color of a pixel. Erase
  makes a pixel transparent, for the formats that have an alpha channel.
- **Undo**: goes back one stroke. **Reset**: gives the original texture.
- **Done**, B or START: back to the menu.

`renderer::TextureEditor` reads and writes the pixels in place. It supports the
formats RGBA8, RGB8, RGBA5551, RGB565, RGBA4 and L8, with sides from 8 to 256
pixels. The paint program changes only the first level of the texture. The
change stays until the game makes the texture again, for example in the next
battle.

## Saving a texture

In the paint program, **Save** writes the texture to the SD card:
`sdmc:/sango/textures/XXXXXXXX.tex`. The name comes from a hash of the original
pixels. After a restart, the plugin gives the saved pixels to every model that
makes the same original texture. **Del** deletes the saved file.

The list of the saved textures is in `sdmc:/sango/textures/index.bin`. The
plugin keeps 256 textures at most.
