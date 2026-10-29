/*
 * Copyright (C) 2026  David Darras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file model_walk.cc
 * @brief Moves the legs and the body of a Pokémon model while it walks.
 *
 * The Pokémon of the game have no walk animation. The code reads the names of
 * the bones and their places, and it finds the kind of walk (two legs, four
 * legs, many legs, no leg). Each frame, it finds the line from one side of the
 * body to the other (it is at a right angle to the face), then it turns the top
 * of each leg around this line with a wave. The body leans and rolls a little
 * around the feet. The declarations are in pokemon/patch/model_walk.h.
 */

#include "pokemon/patch/model_walk.h"

#include <cmath>
#include <cstring>

#include "renderer/patch/skeleton_editor.h"
#include "ui/log.h"

namespace pokemon {
namespace {
constexpr f32 kPhaseStep = 0.28f; // Radians for each frame.
constexpr f32 kBlendStep = 0.12f; // The walk starts and stops in about 8 frames.
constexpr f32 kPi = 3.14159265f;
constexpr u32 kMaxBones = 512;
// The face bone is at least this far from the head, seen from above.
constexpr f32 kMinFaceLength = 2.0f;
// The hands of a quadruped are low (a part of the height) and in front of the
// feet (a part of the height).
constexpr f32 kQuadrupedMaxHandHeight = 0.22f;
constexpr f32 kQuadrupedMinHandAdvance = 0.10f;

// The angles of the swings, in radians.
constexpr f32 kBipedSwing = 0.55f;
constexpr f32 kQuadrupedSwing = 0.45f;
constexpr f32 kMultiLeggedSwing = 0.40f;
constexpr f32 kRollAngle = 0.07f; // The body rolls from side to side.
constexpr f32 kPitchAngle = 0.05f; // The body leans forward and back.

c8 Lower(c8 c) { return (c >= 'A' && c <= 'Z') ? static_cast<c8>(c + 32) : c; }

// Returns true when the text has the word. The comparison ignores the case.
bool Contains(const c8* text, const c8* word) {
  const u32 length = strlen(word);
  for (; *text != '\0'; text++) {
    u32 i = 0;
    while (i < length && text[i] != '\0' && Lower(text[i]) == word[i]) i++;
    if (i == length) return true;
  }
  return false;
}

bool StartsWith(const c8* text, const c8* prefix) {
  return strncmp(text, prefix, strlen(prefix)) == 0;
}

// The bones where a leg starts: a leg does not go above them.
bool IsBody(const c8* name) {
  return Contains(name, "origin") || Contains(name, "waist") ||
         Contains(name, "hip") || Contains(name, "center") ||
         Contains(name, "root") || Contains(name, "spine") ||
         Contains(name, "neck") || Contains(name, "chest") ||
         Contains(name, "head");
}

// Marks a bone and the bones under it.
void MarkTree(const u16* parents, u32 count, u32 bone, bool* is_in_tree) {
  for (u32 i = 0; i < count; i++) is_in_tree[i] = false;
  is_in_tree[bone] = true;
  for (u32 pass = 0; pass < 16; pass++) {
    bool changed = false;
    for (u32 i = 0; i < count; i++) {
      const u32 parent = parents[i];
      if (!is_in_tree[i] && parent < count && parent != i &&
          is_in_tree[parent]) {
        is_in_tree[i] = true;
        changed = true;
      }
    }
    if (!changed) break;
  }
}

// Turns the marked bones around an axis that goes through a point. A matrix
// has 3 rows of 4 numbers.
void Rotate(f32* worlds, u32 count, const bool* is_marked, const f32 pivot[3],
            const f32 axis[3], f32 angle) {
  const f32 c = std::cos(angle);
  const f32 s = std::sin(angle);
  const f32 t = 1.0f - c;
  const f32 x = axis[0], y = axis[1], z = axis[2];
  const f32 r[3][3] = {
      {t * x * x + c, t * x * y - s * z, t * x * z + s * y},
      {t * x * y + s * z, t * y * y + c, t * y * z - s * x},
      {t * x * z - s * y, t * y * z + s * x, t * z * z + c}};
  for (u32 i = 0; i < count; i++) {
    if (!is_marked[i]) continue;
    f32* m = worlds + i * 12;
    f32 out[12];
    const f32 d[3] = {m[3] - pivot[0], m[7] - pivot[1], m[11] - pivot[2]};
    for (u32 row = 0; row < 3; row++) {
      for (u32 col = 0; col < 3; col++) {
        out[row * 4 + col] = r[row][0] * m[col] + r[row][1] * m[4 + col] +
                             r[row][2] * m[8 + col];
      }
      out[row * 4 + 3] =
          r[row][0] * d[0] + r[row][1] * d[1] + r[row][2] * d[2] + pivot[row];
    }
    for (u32 k = 0; k < 12; k++) m[k] = out[k];
  }
}

// A leg that the search found.
struct Candidate {
  u16 top;
  u16 tip;
  u8 side; // 0 left, 1 right, 2 unknown (the name has no side)
  bool is_hand;
};

// The top of a leg: some bones above the foot or the hand, but not the body.
u32 FindLegTop(renderer::H3dModel* model, const u16* parents, u32 count,
               u32 tip, u32 steps) {
  u32 top = tip;
  for (u32 step = 0; step < steps; step++) {
    const u32 parent = parents[top];
    if (parent >= count || parent == top) break;
    if (IsBody(renderer::SkeletonEditor::GetBoneName(model, parent))) break;
    top = parent;
  }
  return top;
}

bool IsKnown(const Candidate* list, u32 size, u32 top) {
  for (u32 i = 0; i < size; i++) {
    if (list[i].top == top) return true;
  }
  return false;
}
} // namespace

void ModelWalk::Advance(bool is_walking) {
  const f32 target = is_walking && kind != Kind::kNone ? 1.0f : 0.0f;
  if (blend < target) {
    blend = blend + kBlendStep > target ? target : blend + kBlendStep;
  } else if (blend > target) {
    blend = blend - kBlendStep < target ? target : blend - kBlendStep;
  }
  if (blend > 0.0f) {
    phase += kPhaseStep;
    if (phase > 2.0f * kPi) phase -= 2.0f * kPi;
  }
}

void ModelWalk::UpdateForward(const f32* worlds, u32 count) {
  // The body: all the models of the game are made the same way, so the
  // direction that the Pokémon looks at is at a right angle to the line from
  // its right side to its left side. This line does not change when the head
  // moves, and it does not depend on the shape of the head.
  if (pair_count > 0) {
    f32 across_x = 0.0f;
    f32 across_z = 0.0f;
    for (u32 i = 0; i < pair_count; i++) {
      if (pair_left[i] >= count || pair_right[i] >= count) continue;
      across_x += worlds[pair_left[i] * 12 + 3] - worlds[pair_right[i] * 12 + 3];
      across_z +=
          worlds[pair_left[i] * 12 + 11] - worlds[pair_right[i] * 12 + 11];
    }
    if (across_x * across_x + across_z * across_z > 0.25f) {
      forward_angle = std::atan2(-across_z, across_x);
      has_forward = true;
      return;
    }
  }

  // Without pairs of bones (a snake, a fish), the face gives the direction.
  if (head_bone >= count || front_bone >= count) return;
  const f32 dx = worlds[front_bone * 12 + 3] - worlds[head_bone * 12 + 3];
  const f32 dz = worlds[front_bone * 12 + 11] - worlds[head_bone * 12 + 11];
  // When the head looks up or down the direction is not clear: the code keeps
  // the last direction, so it does not change at each frame.
  if (dx * dx + dz * dz < 0.25f) return;
  forward_angle = std::atan2(dx, dz);
  has_forward = true;
}

void ModelWalk::Search(renderer::H3dModel* model, const f32* worlds,
                       const u16* parents, u32 count) {
  is_searched = true;

  // The face: a bone in front of the head. The first of these names that is
  // far enough from the head (seen from above) gives the direction.
  static const c8* const kFaceNames[] = {"Jaw",     "Nose",   "Mouth",
                                         "UpperBeak", "Beak", "Snout",
                                         "EndNose", "EndJaw", "EndHead"};
  for (u32 bone = 0; bone < count; bone++) {
    if (strcmp(renderer::SkeletonEditor::GetBoneName(model, bone), "Head") ==
        0) {
      head_bone = static_cast<u16>(bone);
    }
  }
  if (head_bone < count) {
    f32 best_length = 0.0f;
    for (const c8* wanted : kFaceNames) {
      u32 found = count;
      for (u32 bone = 0; bone < count; bone++) {
        if (strcmp(renderer::SkeletonEditor::GetBoneName(model, bone),
                   wanted) == 0) {
          found = bone;
          break;
        }
      }
      if (found >= count) continue;
      const f32 dx = worlds[found * 12 + 3] - worlds[head_bone * 12 + 3];
      const f32 dz = worlds[found * 12 + 11] - worlds[head_bone * 12 + 11];
      const f32 length = std::sqrt(dx * dx + dz * dz);
      if (length > best_length) {
        best_length = length;
        front_bone = static_cast<u16>(found);
      }
      if (length >= kMinFaceLength) {
        front_bone = static_cast<u16>(found);
        break; // The first good name of the list wins.
      }
    }
  }
  // The pairs of bones of the two sides: LName and RName. The first bones of the
  // skeleton are the legs, the arms and the hips: they are the best ones.
  for (u32 bone = 0; bone < count && pair_count < kMaxPairs; bone++) {
    const c8* name = renderer::SkeletonEditor::GetBoneName(model, bone);
    if (name[0] != 'L' || name[1] < 'A' || name[1] > 'Z') continue;
    if (Contains(name, "toe") || Contains(name, "finger") ||
        Contains(name, "feeler") || Contains(name, "ear") ||
        Contains(name, "eye") || Contains(name, "iris")) {
      continue;
    }
    c8 right_name[48];
    strncpy(right_name, name, sizeof(right_name) - 1);
    right_name[sizeof(right_name) - 1] = '\0';
    right_name[0] = 'R';
    for (u32 other = 0; other < count; other++) {
      if (strcmp(renderer::SkeletonEditor::GetBoneName(model, other),
                 right_name) == 0) {
        pair_left[pair_count] = static_cast<u16>(bone);
        pair_right[pair_count] = static_cast<u16>(other);
        pair_count++;
        break;
      }
    }
  }
  UpdateForward(worlds, count);

  if (!can_walk) {
    kind = Kind::kNone;
    return;
  }

  // The feet and the hands of the skeleton.
  Candidate feet[kMaxLegs];
  Candidate hands[kMaxLegs];
  u32 foot_count = 0;
  u32 hand_count = 0;
  for (u32 bone = 0; bone < count; bone++) {
    const c8* name = renderer::SkeletonEditor::GetBoneName(model, bone);
    if (StartsWith(name, "End")) continue;
    const bool is_foot = Contains(name, "foot") && !Contains(name, "sub");
    const bool is_hand = Contains(name, "hand") && !Contains(name, "finger");
    if (!is_foot && !is_hand) continue;
    u8 side = 2;
    if (name[0] == 'L' && name[1] >= 'A' && name[1] <= 'Z') side = 0;
    if (name[0] == 'R' && name[1] >= 'A' && name[1] <= 'Z') side = 1;
    if (is_foot && foot_count < kMaxLegs) {
      const u32 top = FindLegTop(model, parents, count, bone, 2);
      if (!IsKnown(feet, foot_count, top)) {
        feet[foot_count++] = {static_cast<u16>(top), static_cast<u16>(bone),
                              side, false};
      }
    }
    if (is_hand && hand_count < kMaxLegs) {
      const u32 top = FindLegTop(model, parents, count, bone, 3);
      if (!IsKnown(hands, hand_count, top)) {
        hands[hand_count++] = {static_cast<u16>(top), static_cast<u16>(bone),
                               side, true};
      }
    }
  }

  // Four legs: the hands are low, and in front of the feet.
  bool is_quadruped = false;
  if (foot_count >= 2 && hand_count >= 2) {
    f32 foot_pos[3] = {0, 0, 0};
    f32 hand_pos[3] = {0, 0, 0};
    for (u32 i = 0; i < foot_count; i++) {
      for (u32 k = 0; k < 3; k++) {
        foot_pos[k] += worlds[feet[i].tip * 12 + 3 + 4 * k] / foot_count;
      }
    }
    for (u32 i = 0; i < hand_count; i++) {
      for (u32 k = 0; k < 3; k++) {
        hand_pos[k] += worlds[hands[i].tip * 12 + 3 + 4 * k] / hand_count;
      }
    }
    f32 height = 1.0f;
    if (head_bone < count) {
      height = worlds[head_bone * 12 + 7] - foot_pos[1];
    }
    if (height < 1.0f) height = 1.0f;
    const f32 hand_height = (hand_pos[1] - foot_pos[1]) / height;
    f32 advance = 1.0f; // Without a face, only the height tells.
    if (has_forward) {
      advance = ((hand_pos[0] - foot_pos[0]) * std::sin(forward_angle) +
                 (hand_pos[2] - foot_pos[2]) * std::cos(forward_angle)) /
                height;
    }
    is_quadruped = hand_height < kQuadrupedMaxHandHeight &&
                   advance > kQuadrupedMinHandAdvance;
  }

  // The legs of this skeleton.
  Candidate legs[kMaxLegs];
  u32 leg_total = 0;
  if (foot_count >= 2) {
    kind = is_quadruped ? Kind::kQuadruped : Kind::kBiped;
    for (u32 i = 0; i < foot_count && leg_total < kMaxLegs; i++) {
      legs[leg_total++] = feet[i];
    }
    if (is_quadruped) {
      for (u32 i = 0; i < hand_count && leg_total < kMaxLegs; i++) {
        legs[leg_total++] = hands[i];
      }
    }
  } else if (hand_count >= 4) {
    kind = Kind::kMultiLegged;
    for (u32 i = 0; i < hand_count && leg_total < kMaxLegs; i++) {
      legs[leg_total++] = hands[i];
    }
  } else {
    kind = Kind::kNone;
    return;
  }

  // The phase of each leg: a biped puts one leg forward while the other goes
  // back; a quadruped moves the legs in diagonal pairs; many legs go in turn.
  u32 rank[2] = {0, 0};
  u32 unnamed = 0;
  for (u32 i = 0; i < leg_total; i++) {
    // A leg whose name has no side (FootA, FootB...) takes the sides in turn.
    u8 side = legs[i].side;
    if (side > 1) side = static_cast<u8>(unnamed++ & 1);
    leg_bone[i] = legs[i].top;
    tip_bone[i] = legs[i].tip;
    leg_side[i] = side;
    u8 phase_bit = side;
    if (kind == Kind::kQuadruped && legs[i].is_hand) phase_bit ^= 1;
    if (kind == Kind::kMultiLegged) {
      phase_bit ^= static_cast<u8>(rank[side] & 1);
    }
    rank[side]++;
    leg_phase[i] = phase_bit;
  }
  leg_count = leg_total;

  ui::Log::Print(u"walk: kind %d, %d legs, %d bones", static_cast<u32>(kind),
                 leg_count, count);
}

void ModelWalk::Apply(renderer::H3dModel* model) {
#ifdef GAME_ORAS
  u32 count = 0;
  const u16* parents = nullptr;
  f32* worlds = renderer::SkeletonEditor::GetWorldMatrices(model, &count,
                                                           &parents);
  if (worlds == nullptr || count > kMaxBones) return;
  if (!is_searched) Search(model, worlds, parents, count);
  UpdateForward(worlds, count);
  if (blend <= 0.0f || kind == Kind::kNone || kind == Kind::kUnknown ||
      leg_count == 0) {
    return;
  }

  // The center of the feet, on the ground, and the line from one side of the
  // body to the other. The face gives the line: it is at a right angle to the
  // face (a Pokémon can stand with one foot in front of the other, so the line
  // between the feet is not always the right one). Without a face, the line
  // goes from the left legs to the right legs.
  f32 ground[3] = {0, 0, 0};
  f32 across[3] = {0, 0, 0};
  for (u32 i = 0; i < leg_count; i++) {
    const f32* m = worlds + tip_bone[i] * 12;
    const f32 sign = leg_side[i] != 0 ? 1.0f : -1.0f;
    across[0] += sign * m[3];
    across[2] += sign * m[11];
    ground[0] += m[3] / leg_count;
    ground[1] += m[7] / leg_count;
    ground[2] += m[11] / leg_count;
  }
  f32 side[3] = {0, 0, 0};
  if (has_forward) {
    side[0] = std::cos(forward_angle);
    side[2] = -std::sin(forward_angle);
  } else {
    const f32 length = std::sqrt(across[0] * across[0] + across[2] * across[2]);
    if (length < 0.001f) return;
    side[0] = across[0] / length;
    side[2] = across[2] / length;
  }
  const f32 front[3] = {-side[2], 0.0f, side[0]};

  // The body leans and rolls around the point under it.
  bool is_marked[kMaxBones];
  for (u32 i = 0; i < count; i++) is_marked[i] = true;
  Rotate(worlds, count, is_marked, ground, front,
         std::sin(phase) * kRollAngle * blend);
  Rotate(worlds, count, is_marked, ground, side,
         std::sin(phase * 2.0f) * kPitchAngle * blend);

  f32 swing = kBipedSwing;
  if (kind == Kind::kQuadruped) swing = kQuadrupedSwing;
  if (kind == Kind::kMultiLegged) swing = kMultiLeggedSwing;
  for (u32 i = 0; i < leg_count; i++) {
    if (leg_bone[i] >= count) continue;
    MarkTree(parents, count, leg_bone[i], is_marked);
    const f32* top = worlds + leg_bone[i] * 12;
    const f32 pivot[3] = {top[3], top[7], top[11]};
    const f32 angle = std::sin(phase + (leg_phase[i] != 0 ? kPi : 0.0f)) *
                      swing * blend;
    Rotate(worlds, count, is_marked, pivot, side, angle);
  }
#else
  (void)model;
#endif
}

} // namespace pokemon
