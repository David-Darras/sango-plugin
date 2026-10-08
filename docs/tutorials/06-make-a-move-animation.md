# Tutorial 6: Make a move animation

In this tutorial, you change the animation of a move of the game.
Then you make a new animation for Thunderclap, the move of [Tutorial 4](04-add-a-move.md).

**Time:** 30 minutes.
**You need:** [Tutorial 4](04-add-a-move.md).

> **Note:** The move animation tools work only on Alpha Sapphire.

## How a move animation works

A move animation is a list of **steps**. Each step has:

- a start frame and an end frame (the game shows 30 frames each second),
- an action: move the camera, show an effect, play a sound, play a pose...
- a **group**: a number that links the steps of the same object.

For example, a step "spawn an effect in group 1" makes an effect.
The next steps "move group 1" and "resize group 1" change this effect.

| Word | Meaning |
|---|---|
| **Effect** | A particle effect of the game: lightning, fire, water drops... It has an id (`battle::EffectId`). |
| **Layer** | One part of an effect. An effect has up to 8 layers (`battle::EffectLayer`). |
| **Model** | A 3D object of an animation (`battle::EffectModelId`). |
| **Attacker**, **Defender** | The Pokémon that uses the move and the Pokémon that receives it (`battle::AnimationTarget`). |

The class `battle::MoveAnimation` has one function for each type of step.
The list is in `lib/include/battle/patch/move_animation_steps.inc`.

## Part 1: Change an animation of the game

`MoveAnimations::Edit(move, editor)` changes the animation of a move.
The editor is a function that receives the animation of the game.

Make the file `myhack/src/my_animations.cc`:

```cpp
/**
 * @file my_animations.cc
 * @brief The move animations of my ROM hack.
 */

#include "battle/patch/move_animation.h"

namespace myhack {
namespace {

// Thunderbolt becomes red, and its effects are 1.5 times bigger.
void EditThunderbolt(battle::MoveAnimation& a) {
  a.ColorEffects(Color8(255, 60, 60, 255));
  a.ScaleEffects(1.5f);
}

} // namespace

void RegisterAnimations() {
  battle::MoveAnimations::Edit(MoveId::kThunderbolt, EditThunderbolt);
}

} // namespace myhack
```

Call `myhack::RegisterAnimations()` in `Initialize()`, like in the other tutorials.

Build and test: use Thunderbolt in a battle. The lightning is red.

### Functions that change all the effects

| Function | What it does |
|---|---|
| `ColorEffects(color)` | Gives one color to all the effects. |
| `TintEffects(color)` | Multiplies the colors of all the effects. |
| `RecolorEffects(start, middle, end)` | Gives three colors: at the start, in the middle and at the end of the life of each particle. |
| `ScaleEffects(factor)` | Changes the size of all the effects. |
| `ScaleEffectsLife(factor)` | Changes how long the particles live. |
| `ScaleEffectsDensity(factor)` | Changes the number of particles. |
| `FadeEffects(in, out)` | Makes the particles appear and disappear slowly. |
| `ReplaceEffect(from, to)` | Uses a different effect. |

The same functions exist for one effect: `ColorEffect(effect, color)`,
`ScaleEffectSize(effect, factor)`, `DisableEffect(effect, layer)`...

## Part 2: Make a new animation

`MoveAnimations::Define(move, builder)` makes a new animation from zero.
The builder adds the steps one by one.

Add this to `myhack/src/my_animations.cc`:

```cpp
#include "myhack/my_moves.h"  // For kMoveThunderclap.

namespace myhack {
namespace {

using namespace battle;

void BuildThunderclapAnimation(MoveAnimation& a) {
  // Frame 0: put the camera in its normal place, set up the 3D sound,
  // and show only the health bar of the defender.
  a.CameraRestore(0, 0, false, MoveCurve::kLinear, false);
  a.Sound3dSetup(0, 0, 0, 1.0f, 11.3f, 200.0f, 200.0f, 300.0f);
  a.HealthBarShowAll(0, 0, false);
  a.HealthBarShow(0, 0, AnimationTarget::kDefender, true);

  // Frames 2 to 14: the camera moves to the defender.
  a.CameraGlideToPokemon(2, 14, AnimationTarget::kDefender, BodyPoint::kCenter,
                         Vec3(100.0f, 5.0f, 320.0f), Vec3(0.0f, 15.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kLinear,
                         false);

  // Frame 15: a sound at the defender (group 1).
  a.Sound3dPlay(15, 15, 852037, 589827, 90, 0, 40, false, 1);
  a.Sound3dGlideToPokemon(15, 15, AnimationTarget::kDefender, BodyPoint::kWaist,
                          Vec3(0.0f, 0.0f, 0.0f), PositionAdjust::kNormal, 1);

  // Frames 15 to 75: the lightning effect on the defender (group 0).
  a.EffectSpawn(15, 75, EffectId::kThunderboltZapCannonDischarge, false,
                EffectDrawOrder::kStandard, 0);
  a.EffectGlideToPokemon(15, 15, AnimationTarget::kDefender, BodyPoint::kCenter,
                         Vec3(0.0f, 0.0f, 0.0f), PositionAdjust::kNormal, 0);
  a.EffectResize(15, 15, Vec3(2.5f, 2.5f, 2.5f), 0);

  // Frame 20: the defender plays its "hurt" pose.
  a.PokemonPlayPose(20, 20, AnimationTarget::kDefender, PokemonPose::kHurt);

  // Frame 35: the health bar shows the damage.
  a.HealthBarApplyDamage(35, 35, AnimationTarget::kDefender, true);

  // Frames 55 to 75: the camera goes back.
  a.CameraRestore(55, 75, false, MoveCurve::kFastStart, false);

  // Make the lightning yellow and green.
  a.RecolorEffect(EffectId::kThunderboltZapCannonDischarge,
                  Color8(255, 255, 160, 255), Color8(160, 255, 80, 255),
                  Color8(40, 160, 40, 255));
}

} // namespace

void RegisterAnimations() {
  battle::MoveAnimations::Edit(MoveId::kThunderbolt, EditThunderbolt);
  battle::MoveAnimations::Define(kMoveThunderclap, BuildThunderclapAnimation);
}

} // namespace myhack
```

When a move has a defined animation, the library ignores its `patch_animation`.
In [Tutorial 4](04-add-a-move.md), you can set `patch_animation` to `nullptr`.

## Find the effects and the values

- **The effects:** `lib/include/battle/constant/effect_id.h`. The names come
  from the moves that use them: `kThunderboltZapCannonDischarge` is an effect
  of Thunderbolt and of Zap Cannon.
- **The models:** `lib/include/battle/constant/effect_model_id.h`.
- **The positions, poses and curves:** `lib/include/battle/constant/animation_enums.h`.
- **The values of a real animation:** copy the values of a move that looks
  like your move. The examples of the overlay are good models:
  `overlay/src/thunderbolt_animation.cc`, `overlay/src/absolute_zero_animation.cc`.
- **Preview all the effects:** `overlay/src/effect_showcase_animation.cc`
  shows the effects of the game 10 at a time.

## The limits of the engine

The game stops if an animation uses too many objects at the same time:

| Object | Groups at most |
|---|---|
| Effects | 10 (groups 0 to 9) |
| Models | 10 |
| Sounds | 6 |

Do not spawn a new object in a group that is still alive.

The library checks these limits before the animation plays.
If your animation breaks a limit, the library plays the animation of the game
instead, and your animation does not show. Make your animation smaller.

## What you learned

- An animation is a list of steps with frames and groups.
- `MoveAnimations::Edit` changes an animation of the game.
- `MoveAnimations::Define` makes a new animation.
- The engine has limits on the number of objects.

**Next:** [Tutorial 7: Change the wild Pokémon](07-change-wild-pokemon.md)
