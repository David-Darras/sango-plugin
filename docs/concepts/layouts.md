# The 2D layouts

The menus of the game (the party, the bag, the PC...) are 2D layouts.

## The tree of panes

A layout is a tree of **panes**. The first pane is the **root**. Each pane has
a parent, children and brothers (the other children of its parent).

| Kind | What it is |
|---|---|
| Group | A pane without a picture. It moves its children together. |
| Text Box | A text. It has a top color and a bottom color. |
| Picture | An image. It has four colors, one for each corner. |
| Window | A frame. |
| Touch Area | The area that a button reads with the stylus. |

Each pane has a name (16 characters), a position (translate), a rotation, a
scale, a size, an alpha (opacity) and a visible flag.

## How the plugin finds the panes

`renderer::LayoutEditor` hooks the function that calculates the matrix of a
pane. The game calls it
for each pane, one time for each frame, after its animations and before the
drawing. The hook does two things:

1. It keeps each pane that has no parent: the roots of the screen.
2. It writes the changes of the player into the pane, before the game
   calculates its matrix. The animations of the game cannot undo them.

The plugin works for Alpha Sapphire only. See `lib/include/renderer/address.h`
for the address.

## The menu

The page **Graphics > Layouts** lists the layouts of the screen. A layout opens
the page of its root pane. This page shows one pane: its values, its parent, its
children and its brothers. Touch a pane of the tree to go to it.

The page has these entries:

- **Visible**, **Alpha**: show or hide the pane.
- **Translate**, **Rotate**, **Scale**, **Width**, **Height**: move, turn and
  resize the pane.
- **Colors**: the colors of a text box or of a picture.
- **Reset This Pane**: removes the change.
- **Save To The SD Card**: writes the changes to `sdmc:/sango/layouts.ini`.

## The saved changes

A change is for one pane on one screen. The key is the name of the screen (the
class of the process) and the path of the pane, for example
`field::FieldmapProc` and `RootPane@400/N_Menu/T_Title`. The root has the
width of its layout (`@400` for the top screen, `@320` for the bottom screen):
the two screens of one process often have panes with the same names.

```ini
[layouts]
edit1 = app::pokelist::Process|RootPane@400/N_Top/T_Title|0.000,12.000,0.000,0.000,0.000,0.000,1.500,1.500,120.000,30.000,255,1,2,FF0000FF,0000FFFF
```

The values are: translate (3), rotate (3), scale (2), size (2), alpha, visible,
the number of colors, then the colors as `RRGGBBAA`. The plugin reads the file
at the start. It keeps 24 changes at most.

Two panes can have the same name. The path tells them apart.
