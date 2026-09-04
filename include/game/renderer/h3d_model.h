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

#include "common.h"
#include "game/core/process_manager.h"
#include "game/renderer/h3d_model.h"

namespace ui {
class LogApplication;
}

namespace feature {
namespace TevCombine {
enum Combine {
  REPLACE = 0, MODULATE = 1, ADD = 2, ADD_SIGNED = 3,
  INTERPOLATE = 4, SUBTRACT = 5, DOT3_RGB = 6, DOT3_RGBA = 7,
  MULTIPLY_ADD = 8, ADD_MULTIPLY = 9
};

enum Source {
  PRIMARY_COLOR = 0, TEXTURE0 = 3, TEXTURE1 = 4, TEXTURE2 = 5,
  PREVIOUS_BUFFER = 13, CONSTANT = 14, PREVIOUS = 15
};

enum OperandRgb {
  COLOR = 0, ONE_MINUS_COLOR = 1, ALPHA = 2, ONE_MINUS_ALPHA = 3,
  RED = 4, ONE_MINUS_RED = 5,
  GREEN = 8, ONE_MINUS_GREEN = 9,
  BLUE = 12, ONE_MINUS_BLUE = 13
};

enum Scale { SCALE_1 = 0, SCALE_2 = 1, SCALE_4 = 2 };
}
} // namespace feature


namespace renderer {
using namespace feature::TevCombine;

struct H3dModel {
  uptr vtable;

  INLINE void SetMeshVisible(u8 index, bool is_visible) {
    return ((void(*)(H3dModel*, u8, bool))
      ADDRESS_H3D_MODEL_SET_MESH_VISIBLE)(this, index, is_visible);
  }

  INLINE s32 GetMaterialCount() {
    return ((s32(*)(H3dModel*))
      ADDRESS_H3D_MODEL_GET_MATERIAL_COUNT)(this);
  }

  INLINE void SetCombineRgb(s32 mat, s32 step, s32 combine) {
    return ((void(*)(H3dModel*, s32, s32, s32))
      ADDRESS_H3D_MODEL_SET_COMBINER_COMBINE_RGB)(this, mat, step, combine);
  }

  INLINE void SetScaleRgb(s32 mat, s32 step, s32 scale) {
    return ((void(*)(H3dModel*, s32, s32, s32))
      ADDRESS_H3D_MODEL_SET_COMBINER_SCALE_RGB)(this, mat, step, scale);
  }

  INLINE void SetSourceRgb(s32 mat, s32 step, s32 no, s32 source) {
    return ((void(*)(H3dModel*, s32, s32, s32, s32))
      ADDRESS_H3D_MODEL_SET_COMBINER_SOURCE_RGB)(this, mat, step, no, source);
  }

  INLINE void SetOperandRgb(s32 mat, s32 step, s32 no, s32 ope) {
    return ((void(*)(H3dModel*, s32, s32, s32, s32))
      ADDRESS_H3D_MODEL_SET_COMBINER_OPERAND_RGB)(this, mat, step, no, ope);
  }

  INLINE void SetCombinerConstantSlot(s32 mat, s32 step, s32 no) {
    return ((void(*)(H3dModel*, s32, s32, s32))
      ADDRESS_H3D_MODEL_SET_COMBINER_CONSTANT)(this, mat, step, no);
  }

  INLINE void SetColorConstant(s32 mat, s32 no, const Color& c) {
    return ((void(*)(H3dModel*, s32, s32, const Color&))
      ADDRESS_H3D_MODEL_SET_COLOR_CONSTANT)(this, mat, no, c);
  }

  INLINE void SetCombineAlpha(s32 mat, s32 step, s32 combine) {
    return ((void(*)(H3dModel*, s32, s32, s32))
      ADDRESS_H3D_MODEL_SET_COMBINER_COMBINE_ALPHA)(this, mat, step, combine);
  }

  INLINE void SetSourceAlpha(s32 mat, s32 step, s32 no, s32 source) {
    return ((void(*)(H3dModel*, s32, s32, s32, s32))
      ADDRESS_H3D_MODEL_SET_COMBINER_SOURCE_ALPHA)(this, mat, step, no, source);
  }

  INLINE void SetOperandAlpha(s32 mat, s32 step, s32 no, s32 ope) {
    return ((void(*)(H3dModel*, s32, s32, s32, s32))
      ADDRESS_H3D_MODEL_SET_COMBINER_OPERAND_ALPHA)(this, mat, step, no, ope);
  }

  INLINE void SetTranslucencyKind(s32 mat, s32 kind) {
    return ((void(*)(H3dModel*, s32, s32))
      ADDRESS_H3D_MODEL_SET_TRANSLUCENCY_KIND)(this, mat, kind);
  }

  INLINE void SetAlphaTestEnable(s32 mat, bool enable) {
    return ((void(*)(H3dModel*, s32, bool))
      ADDRESS_H3D_MODEL_SET_ALPHA_TEST_ENABLE)(this, mat, enable);
  }

  void ConfigureStage(s32 step, s32 combine,
                      s32 src0, s32 op0,
                      s32 src1 = -1, s32 op1 = 0,
                      s32 scale = SCALE_1) {
    s32 matCnt = this->GetMaterialCount();
    for (s32 i = 0; i < matCnt; ++i) {
      this->SetCombineRgb(i, step, combine);
      this->SetSourceRgb(i, step, 0, src0);
      this->SetOperandRgb(i, step, 0, op0);
      if (src1 != -1) {
        this->SetSourceRgb(i, step, 1, src1);
        this->SetOperandRgb(i, step, 1, op1);
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
                           s32 src1 = -1, s32 op1 = 0) {
    s32 matCnt = this->GetMaterialCount();
    for (s32 i = 0; i < matCnt; ++i) {
      this->SetCombineAlpha(i, step, combine);
      this->SetSourceAlpha(i, step, 0, src0);
      this->SetOperandAlpha(i, step, 0, op0);
      if (src1 != -1) {
        this->SetSourceAlpha(i, step, 1, src1);
        this->SetOperandAlpha(i, step, 1, op1);
      }
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

  INLINE void ApplyTrueSaturation() {
    this->SetConstant(4, 0, 77, 150, 29);
    this->ConfigureStage(4, DOT3_RGB, CONSTANT, COLOR, PREVIOUS, COLOR,
                         SCALE_1);
    this->ConfigureStage(5, ADD_SIGNED, PREVIOUS, COLOR, PREVIOUS_BUFFER,
                         ONE_MINUS_COLOR, SCALE_2);
  }

  INLINE void ApplySepia() {
    this->SetConstant(5, 0, 112, 66, 20);
    this->ConfigureStage(5, MODULATE, PREVIOUS, COLOR, CONSTANT, COLOR,
                         SCALE_1);
  }

  INLINE void ApplyTintRed() {
    this->SetConstant(5, 0, 255, 90, 90);
    this->ConfigureStage(5, MODULATE, PREVIOUS, COLOR, CONSTANT, COLOR,
                         SCALE_1);
  }

  INLINE void ApplyTintBlue() {
    this->SetConstant(5, 0, 90, 90, 255);
    this->ConfigureStage(5, MODULATE, PREVIOUS, COLOR, CONSTANT, COLOR,
                         SCALE_1);
  }

  INLINE void ApplyTintGreen() {
    this->SetConstant(5, 0, 90, 255, 90);
    this->ConfigureStage(5, MODULATE, PREVIOUS, COLOR, CONSTANT, COLOR,
                         SCALE_1);
  }

  INLINE void ApplyNightVision() {
    this->SetConstant(4, 0, 40, 255, 40);
    this->ConfigureStage(4, MODULATE, PREVIOUS, COLOR, CONSTANT, COLOR,
                         SCALE_1);
    this->ConfigureStage(5, MODULATE, PREVIOUS_BUFFER, COLOR, PREVIOUS_BUFFER,
                         COLOR, SCALE_2);
  }

  INLINE void ApplyVintage() {
    this->SetConstant(5, 0, 200, 180, 150, 90);
    this->ConfigureStage(5, INTERPOLATE, PREVIOUS, COLOR, CONSTANT,
                         ONE_MINUS_COLOR, SCALE_1);
  }

  INLINE void ApplySolarize() {
    this->ConfigureStage(5, ADD_SIGNED, PREVIOUS, COLOR, PREVIOUS,
                         ONE_MINUS_COLOR, SCALE_2);
  }

  INLINE void ApplyChromeMetallic() {
    this->ConfigureStage(4, MODULATE, PREVIOUS, COLOR, PREVIOUS, COLOR,
                         SCALE_4);
    this->SetConstant(5, 0, 180, 220, 255);
    this->ConfigureStage(5, INTERPOLATE, PREVIOUS_BUFFER, COLOR, CONSTANT,
                         ONE_MINUS_COLOR, SCALE_1);
  }

  INLINE void ApplyGoldMetallic() {
    this->ConfigureStage(4, MODULATE, PREVIOUS, COLOR, PREVIOUS, COLOR,
                         SCALE_4);
    this->SetConstant(5, 0, 255, 215, 120);
    this->ConfigureStage(5, INTERPOLATE, PREVIOUS_BUFFER, COLOR, CONSTANT,
                         ONE_MINUS_COLOR, SCALE_1);
  }

  INLINE void ApplyDuotoneOceanFire() {
    this->SetConstant(4, 0, 77, 150, 29);
    this->ConfigureStage(4, DOT3_RGB, CONSTANT, COLOR, PREVIOUS, COLOR,
                         SCALE_1);
    this->SetConstant(5, 1, 255, 140, 40);
    this->ConfigureStage(5, INTERPOLATE, CONSTANT, COLOR, PREVIOUS_BUFFER,
                         COLOR, SCALE_1);
  }

  INLINE void ApplyThermalCam() {
    this->SetConstant(4, 0, 77, 150, 29);
    this->ConfigureStage(4, DOT3_RGB, CONSTANT, COLOR, PREVIOUS, COLOR,
                         SCALE_1);
    this->SetConstant(5, 1, 255, 0, 120);
    this->ConfigureStage(5, MODULATE, PREVIOUS_BUFFER, COLOR, CONSTANT, COLOR,
                         SCALE_2);
  }

  INLINE void ApplyFilmNoir() {
    this->SetConstant(4, 0, 77, 150, 29);
    this->ConfigureStage(4, DOT3_RGB, CONSTANT, COLOR, PREVIOUS, COLOR,
                         SCALE_1);
    this->ConfigureStage(5, INTERPOLATE, PREVIOUS, COLOR, PREVIOUS_BUFFER,
                         ONE_MINUS_COLOR, SCALE_2);
  }

  INLINE void ApplyBlueprint() {
    this->SetConstant(4, 0, 77, 150, 29);
    this->ConfigureStage(4, DOT3_RGB, CONSTANT, COLOR, PREVIOUS, COLOR,
                         SCALE_1);
    this->SetConstant(5, 1, 40, 80, 200);
    this->ConfigureStage(5, MODULATE, PREVIOUS_BUFFER, ONE_MINUS_COLOR,
                         CONSTANT, COLOR, SCALE_1);
  }

  INLINE void ApplyXray() {
    this->ConfigureStage(4, REPLACE, PREVIOUS, ONE_MINUS_COLOR);
    this->SetConstant(5, 0, 150, 220, 255);
    this->ConfigureStage(5, MODULATE, PREVIOUS_BUFFER, COLOR, CONSTANT, COLOR,
                         SCALE_2);
  }

  void ApplyToon() {
    s32 matCnt = this->GetMaterialCount();
    for (s32 i = 0; i < matCnt; ++i) {
      this->SetCombineRgb(i, 1, MODULATE);
    }
  }

  void ApplyGhostMode() {
    s32 matCnt = this->GetMaterialCount();
    for (s32 i = 0; i < matCnt; ++i) {
      this->SetAlphaTestEnable(i, true);
      this->SetTranslucencyKind(i, 1);
    }
    this->SetConstant(5, 0, 255, 255, 255, 128);
    this->ConfigureAlphaStage(5, MODULATE, PREVIOUS, 0, CONSTANT, 0);
  }
};
} // namespace renderer