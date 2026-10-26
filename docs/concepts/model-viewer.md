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
