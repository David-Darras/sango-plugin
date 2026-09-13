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

#pragma once

#include "core/types.h"

struct Vec2 {
  f32 x, y;

  Vec2() : x(0), y(0) {}
  Vec2(f32 x0, f32 y0) : x(x0), y(y0) {}
};

struct Vec3 {
  f32 x, y, z;

  Vec3() : x(0), y(0), z(0) {}
  Vec3(f32 x0, f32 y0, f32 z0) : x(x0), y(y0), z(z0) {}

  static float Dot(const Vec3& a, const Vec3& b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
  }
};

struct Vec4 {
  f32 x, y, z, w;

  Vec4() : x(0), y(0), z(0), w(0) {}
  Vec4(f32 x0, f32 y0, f32 z0, f32 w0) : x(x0), y(y0), z(z0), w(w0) {}
};

struct Mtx33 {
  f32 m[3][3];
};

struct Mtx34 {
  f32 m[3][4];
};

struct Mtx44 {
  f32 m[4][4];
};

struct Aabb {
  Vec4 min;
  Vec4 max;
};
