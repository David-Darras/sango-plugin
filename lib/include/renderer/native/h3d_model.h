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
 * @file h3d_model.h
 * @brief A 3D model of the game (H3D format).
 */

#pragma once

#include "common.h"
namespace renderer {
/// The values of the texture combiner stages of the GPU.
namespace TevCombine {
/// The operation of a combiner stage. A, B and C are the three sources.
enum Combine : u8 {
  REPLACE = 0, ///< A
  MODULATE = 1, ///< A * B
  ADD = 2, ///< A + B
  ADD_SIGNED = 3, ///< A + B - 0.5
  INTERPOLATE = 4, ///< A * C + B * (1 - C)
  SUBTRACT = 5, ///< A - B
  DOT3_RGB = 6, ///< dot(A, B), copied to R, G and B.
  DOT3_RGBA = 7, ///< dot(A, B), copied to R, G, B and A.
  MULTIPLY_ADD = 8, ///< A * B + C
  ADD_MULTIPLY = 9 ///< (A + B) * C
};

enum Source : u8 {
  PRIMARY_COLOR = 0,
  FRAGMENT_PRIMARY_COLOR = 1,
  FRAGMENT_SECONDARY_COLOR = 2,
  TEXTURE0 = 3, TEXTURE1 = 4, TEXTURE2 = 5, TEXTURE3 = 6,
  PREVIOUS_BUFFER = 13,
  CONSTANT = 14, PREVIOUS = 15
};

enum OperandRgb : u8 {
  COLOR = 0, ONE_MINUS_COLOR = 1, ALPHA = 2, ONE_MINUS_ALPHA = 3,
  RED = 4, ONE_MINUS_RED = 5,
  GREEN = 8, ONE_MINUS_GREEN = 9,
  BLUE = 12, ONE_MINUS_BLUE = 13
};

enum Scale : u8 { SCALE_1 = 0, SCALE_2 = 1, SCALE_4 = 2 };

enum BlendMode : u8 { BLEND_OFF = 0, BLEND_ON = 1 };

enum BlendFactor : u8 {
  FACTOR_ZERO = 0, FACTOR_ONE = 1,
  FACTOR_SRC_COLOR = 2, FACTOR_ONE_MINUS_SRC_COLOR = 3,
  FACTOR_DST_COLOR = 4, FACTOR_ONE_MINUS_DST_COLOR = 5,
  FACTOR_SRC_ALPHA = 6, FACTOR_ONE_MINUS_SRC_ALPHA = 7,
  FACTOR_DST_ALPHA = 8, FACTOR_ONE_MINUS_DST_ALPHA = 9
};

enum BlendEquation : u8 { EQUATION_ADD = 0, EQUATION_SUBTRACT = 1 };
} // namespace TevCombine

using namespace TevCombine;

/// A 3D model of the game. The functions change its meshes and its materials.
struct H3dModel {
  uptr vtable;

  INLINE void SetMeshVisible(u8 index, bool is_visible) {
    return ((void(*)(H3dModel*, u8, bool))
      address::kH3dModelSetMeshVisible)(this, index, is_visible);
  }

  INLINE s32 GetMaterialCount() {
    return ((s32(*)(H3dModel*))
      address::kH3dModelGetMaterialCount)(this);
  }

  INLINE void SetCombineRgb(s32 mat, s32 step, s32 combine) {
    return ((void(*)(H3dModel*, s32, s32, s32))
      address::kH3dModelSetCombinerCombineRgb)(this, mat, step, combine);
  }

  INLINE void SetScaleRgb(s32 mat, s32 step, s32 scale) {
    return ((void(*)(H3dModel*, s32, s32, s32))
      address::kH3dModelSetCombinerScaleRgb)(this, mat, step, scale);
  }

  INLINE void SetSourceRgb(s32 mat, s32 step, s32 no, s32 source) {
    return ((void(*)(H3dModel*, s32, s32, s32, s32))
      address::kH3dModelSetCombinerSourceRgb)(this, mat, step, no, source);
  }

  INLINE void SetOperandRgb(s32 mat, s32 step, s32 no, s32 ope) {
    return ((void(*)(H3dModel*, s32, s32, s32, s32))
      address::kH3dModelSetCombinerOperandRgb)(this, mat, step, no, ope);
  }

  INLINE void SetCombinerConstantSlot(s32 mat, s32 step, s32 no) {
    return ((void(*)(H3dModel*, s32, s32, s32))
      address::kH3dModelSetCombinerConstant)(this, mat, step, no);
  }

  INLINE void SetColorConstant(s32 mat, s32 no, const Color& c) {
    return ((void(*)(H3dModel*, s32, s32, const Color&))
      address::kH3dModelSetColorConstant)(this, mat, no, c);
  }

  INLINE void SetCombineAlpha(s32 mat, s32 step, s32 combine) {
    return ((void(*)(H3dModel*, s32, s32, s32))
      address::kH3dModelSetCombinerCombineAlpha)(this, mat, step, combine);
  }

  INLINE void SetSourceAlpha(s32 mat, s32 step, s32 no, s32 source) {
    return ((void(*)(H3dModel*, s32, s32, s32, s32))
      address::kH3dModelSetCombinerSourceAlpha)(this, mat, step, no, source);
  }

  INLINE void SetOperandAlpha(s32 mat, s32 step, s32 no, s32 ope) {
    return ((void(*)(H3dModel*, s32, s32, s32, s32))
      address::kH3dModelSetCombinerOperandAlpha)(this, mat, step, no, ope);
  }

  INLINE void SetTranslucencyKind(s32 mat, s32 kind) {
    return ((void(*)(H3dModel*, s32, s32))
      address::kH3dModelSetTranslucencyKind)(this, mat, kind);
  }

  INLINE void SetBufferInputRgb(s32 mat, s32 step, u8 value) {
    return ((void(*)(H3dModel*, s32, s32, u8))
      address::kH3dModelSetBufferInputRgb)(this, mat, step, value);
  }

  INLINE void SetBufferInputAlpha(s32 mat, s32 step, u8 value) {
    return ((void(*)(H3dModel*, s32, s32, u8))
      address::kH3dModelSetBufferInputAlpha)(this, mat, step, value);
  }

  INLINE void SetBlendMode(s32 mat, s32 mode) {
    return ((void(*)(H3dModel*, s32, s32))
      address::kH3dModelSetBlendMode)(this, mat, mode);
  }

  INLINE void SetBlendFuncSourceRgb(s32 mat, s32 factor) {
    return ((void(*)(H3dModel*, s32, s32))
      address::kH3dModelSetBlendFuncSourceRgb)(this, mat, factor);
  }

  INLINE void SetBlendFuncDestRgb(s32 mat, s32 factor) {
    return ((void(*)(H3dModel*, s32, s32))
      address::kH3dModelSetBlendFuncDestRgb)(this, mat, factor);
  }

  INLINE void SetBlendEquationRgb(s32 mat, s32 equation) {
    return ((void(*)(H3dModel*, s32, s32))
      address::kH3dModelSetBlendEquationRgb)(this, mat, equation);
  }

  INLINE void SetAlphaTestEnable(s32 mat, bool enable) {
    return ((void(*)(H3dModel*, s32, bool))
      address::kH3dModelSetAlphaTestEnable)(this, mat, enable);
  }

  void ConfigureStage(s32 step, s32 combine,
                      s32 src0, s32 op0,
                      s32 src1 = -1, s32 op1 = 0,
                      s32 scale = SCALE_1,
                      s32 src2 = -1, s32 op2 = 0) {
    s32 matCnt = this->GetMaterialCount();
    for (s32 i = 0; i < matCnt; ++i) {
      this->SetCombineRgb(i, step, combine);
      this->SetSourceRgb(i, step, 0, src0);
      this->SetOperandRgb(i, step, 0, op0);
      if (src1 != -1) {
        this->SetSourceRgb(i, step, 1, src1);
        this->SetOperandRgb(i, step, 1, op1);
      }
      if (src2 != -1) {
        this->SetSourceRgb(i, step, 2, src2);
        this->SetOperandRgb(i, step, 2, op2);
      }
      this->SetScaleRgb(i, step, scale);
    }
  }

  void SetConstant(s32 step, s32 no, u8 r, u8 g, u8 b, u8 a = 255) {
    s32 matCnt = this->GetMaterialCount();
    Color c{r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
    for (s32 i = 0; i < matCnt; ++i) {
      this->SetColorConstant(i, no, c);
      this->SetCombinerConstantSlot(i, step, no);
    }
  }

  void ConfigureAlphaStage(s32 step, s32 combine,
                           s32 src0, s32 op0 = 0,
                           s32 src1 = -1, s32 op1 = 0,
                           s32 src2 = -1, s32 op2 = 0) {
    s32 matCnt = this->GetMaterialCount();
    for (s32 i = 0; i < matCnt; ++i) {
      this->SetCombineAlpha(i, step, combine);
      this->SetSourceAlpha(i, step, 0, src0);
      this->SetOperandAlpha(i, step, 0, op0);
      if (src1 != -1) {
        this->SetSourceAlpha(i, step, 1, src1);
        this->SetOperandAlpha(i, step, 1, op1);
      }
      if (src2 != -1) {
        this->SetSourceAlpha(i, step, 2, src2);
        this->SetOperandAlpha(i, step, 2, op2);
      }
    }
  }

  void EnableAlphaBlend() {
    s32 matCnt = this->GetMaterialCount();
    for (s32 i = 0; i < matCnt; ++i) {
      this->SetBlendMode(i, BLEND_ON);
      this->SetBlendEquationRgb(i, EQUATION_ADD);
      this->SetBlendFuncSourceRgb(i, FACTOR_SRC_ALPHA);
      this->SetBlendFuncDestRgb(i, FACTOR_ONE_MINUS_SRC_ALPHA);
      this->SetTranslucencyKind(i, 1);
      this->SetAlphaTestEnable(i, false);
    }
  }

  INLINE void ApplyPitchBlack() {
    this->ConfigureStage(5, SUBTRACT, PREVIOUS, COLOR, PREVIOUS, COLOR,
                         SCALE_1);
  }

  INLINE void ApplyInvert() {
    this->ConfigureStage(5, REPLACE, PREVIOUS, ONE_MINUS_COLOR);
  }

  INLINE void ApplyDarken() {
    this->ConfigureStage(5, MODULATE, PREVIOUS, COLOR, PREVIOUS, COLOR,
                         SCALE_1);
  }

  INLINE void ApplyOverexposed() {
    this->ConfigureStage(5, ADD, PREVIOUS, COLOR, PREVIOUS, COLOR, SCALE_1);
  }

  INLINE void ApplyPsychedelic() {
    this->ConfigureStage(5, MODULATE, PREVIOUS, COLOR, PREVIOUS,
                         ONE_MINUS_COLOR, SCALE_4);
  }

  INLINE void ApplySepia() {
    this->SetConstant(5, 0, 112, 66, 20);
    this->ConfigureStage(5, MODULATE, PREVIOUS, COLOR, CONSTANT, COLOR,
                         SCALE_1);
  }

  INLINE void ApplyObsidian() {
    this->ConfigureStage(5, REPLACE, FRAGMENT_PRIMARY_COLOR, COLOR);
  }

  INLINE void ApplyPlasma() {
    this->ConfigureStage(5, REPLACE, FRAGMENT_SECONDARY_COLOR, COLOR);
  }

  INLINE void ApplySketch() {
    this->ConfigureStage(5, MODULATE,
                         FRAGMENT_SECONDARY_COLOR, BLUE,
                         FRAGMENT_SECONDARY_COLOR, BLUE, SCALE_1);
  }

  INLINE void ApplyChromeMetallic() {
    this->ConfigureStage(4, MODULATE, PREVIOUS, COLOR, PREVIOUS, COLOR,
                         SCALE_4);
    this->SetConstant(5, 0, 180, 220, 255);
    this->ConfigureStage(5, INTERPOLATE, PREVIOUS_BUFFER, COLOR, CONSTANT,
                         ONE_MINUS_COLOR, SCALE_1);
  }

  INLINE void ApplyLiquidChrome() {
    this->SetConstant(5, 0, 225, 238, 255);
    this->ConfigureStage(5, INTERPOLATE,
                         CONSTANT, COLOR,
                         PREVIOUS, GREEN, SCALE_1,
                         FRAGMENT_SECONDARY_COLOR, BLUE);
  }

  void ApplyGhostMode() {
    this->EnableAlphaBlend();
    this->SetConstant(5, 0, 255, 255, 255, 110);
    this->ConfigureAlphaStage(5, MODULATE, PREVIOUS, 0, CONSTANT, 0);
  }

};

} // namespace renderer
