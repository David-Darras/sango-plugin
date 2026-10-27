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
the game made, the entry is **Paint Unit N**, with the format and the size of
the texture.

The paint program opens on the bottom screen. The stylus draws on the texture,
and the model on the top screen changes at once. The palette is always on the
right. The button under the tools changes the tab.

- **Draw**: **- / +** is the size of the brush. The tool button changes between
  Paint, Pick (takes the color of a pixel), Erase (makes a pixel transparent, for
  the formats with an alpha channel) and Fill (the bucket fills the area of the
  same color). **Mirror** draws the same stroke on the other side of the
  texture. **Undo** goes back one action, **Reset** gives the original texture.
- **Color**: **Hue**, **Sat** and **Val** change the hue, the saturation and the
  brightness of all the texture, one step for each touch (it is a way to make a
  shiny form). **Replace** changes the color that you picked with the Pick tool
  to the color of the palette, and keeps the shades.
- **File**: **Export** writes the texture to
  `sdmc:/sango/textures/export/XXXXXXXX.tga` (32 bits). **Import** reads this
  file (24 or 32 bits, same size). **Live** exports the texture, then reloads the
  file each time it changes: open the file in a program on your computer, save
  it, and the model changes in the game. **Save** and **Del** keep or delete the
  texture (see below).

**Done**, B or START goes back to the menu.

`renderer::TextureEditor` reads and writes the pixels in place. It supports the
formats RGBA8, RGB8, RGBA5551, RGB565, RGBA4 and L8, with sides from 8 to 256
pixels. The paint program changes only the first level of the texture. The
change stays until the game makes the texture again, for example in the next
battle.

## The texture coordinates

The page of a material has a section **Texture N UV** for each texture. **Scale
U/V**, **Rotate** and **Move U/V** change how the texture covers the surface. For
example, move the UV of the eyes to change the look of a Pokémon.

## Saving a texture

In the paint program, **Save** writes the texture to the SD card:
`sdmc:/sango/textures/XXXXXXXX.tex`. The name comes from a hash of the original
pixels. After a restart, the plugin gives the saved pixels to every model that
makes the same original texture. **Del** deletes the saved file.

The list of the saved textures is in `sdmc:/sango/textures/index.bin`. The
plugin keeps 256 textures at most.

## The skeleton

A model has a skeleton: bones that the animations of the game move. In
**Model Viewer > a model > Skeleton > Bones**, the list shows all the bones.
Open a bone to move it, turn it (in degrees) or scale it. The bones under it
follow, as in a 3D program: scale the bone of the head to make a big head.
A bone with a star has a change.

Not all the bones have vertices: a change of a bone without vertices shows
only through the bones under it. Try a bone of the body, the head or a limb.

`renderer::SkeletonEditor` changes the matrices of the bones when the game
starts to draw the model. It works for all the models, also for the
characters, because it does not depend on who calculates the bones.

**Save To The SD Card** writes `sdmc:/sango/skeleton.ini`. A change is for all
the models that have the same skeleton: the key is a hash of the names of the
bones. After a restart, every Pokémon of the same species gets its changes
again. The file keeps 32 changes at most.
