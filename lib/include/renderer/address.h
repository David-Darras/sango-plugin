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

#include "core/game.h"
#include "core/types.h"

namespace renderer {
namespace address {

constexpr uptr kFadeRequestIn = GAME_ADDRESS(0x00359CD8, 0x00371518);

constexpr uptr kGraphicsGetFramebuffer = GAME_ADDRESS(0x0012F9F8, 0x0012FA94);
constexpr uptr kGraphicsBindFramebuffer = GAME_ADDRESS(0x00359FA0, 0x003717E0);
constexpr uptr kGraphicsEnableScissor = GAME_ADDRESS(0x0038C7B0, 0x003A30A8);
constexpr uptr kGraphicsDisableScissor = GAME_ADDRESS(0x0038C98C, 0x003A3284);
constexpr uptr kGraphicsBeginRender = GAME_ADDRESS(0x00174B20, 0x00175A58);
constexpr uptr kGraphicsSetTextScale = GAME_ADDRESS(0x0038CBA0, 0x003A3498);
constexpr uptr kGraphicsDrawText = GAME_ADDRESS(0x0038DB54, 0x003A44C4);
constexpr uptr kGraphicsDrawRect = GAME_ADDRESS(0x00174CA4, 0x00175BDC);
constexpr uptr kStereoCameraUpdateMatrices = GAME_ADDRESS(0x003609A8, 0x00377C10);
constexpr uptr kStereoCameraUpdateLookAt = GAME_ADDRESS(0x00148FD8, 0x001483DC);
constexpr uptr kStereoCameraSetProjectionMatrix = GAME_ADDRESS(0x0037CE5C, 0x003939CC);
constexpr uptr kModelPlayAnimation = GAME_ADDRESS(0x003DB040, 0x003FB1B4);
constexpr uptr kModelUpdateMotion = GAME_ADDRESS(0x006F8028, 0x00774D9C); // XY: unverified (vtable slot, 3 candidates)
constexpr uptr kChangeOutlineScale = GAME_ADDRESS(0x00362BD4, 0x00379E3C);
constexpr uptr kChangeOutlineColor = GAME_ADDRESS(0x00362A14, 0x00379C7C);
constexpr uptr kChangeAmbientLightColor = GAME_ADDRESS(0x0013A140, 0x00139710);
constexpr uptr kChangeDiffuseLightColor = GAME_ADDRESS(0x0013A1F0, 0x001397C0);
constexpr uptr kDrawPicture = GAME_ADDRESS(0x0048EE34, 0x004BE3DC);
constexpr uptr kDrawTextBox = GAME_ADDRESS(0x00312A70, 0x00326FB8);
constexpr uptr kAppStatusSetupGraphicsParams = GAME_ADDRESS(0, 0x007144FC);
constexpr uptr kAppStatusSetupGraphicsMoves = GAME_ADDRESS(0, 0x0070DDC0);
constexpr uptr kAppStatusSetupGraphicsContest = GAME_ADDRESS(0, 0x0070D3D4);
constexpr uptr kAppStatusSetupGraphicsInfos = GAME_ADDRESS(0, 0x007116E4);
constexpr uptr kH3dModelGetMaterialCount = GAME_ADDRESS(0x0049D284, 0x004CD168);
constexpr uptr kH3dModelSetCombinerCombineRgb = GAME_ADDRESS(0x00382E9C, 0x00399920);
constexpr uptr kH3dModelSetCombinerScaleRgb = GAME_ADDRESS(0x00382CE0, 0x003997AC);
constexpr uptr kH3dModelSetCombinerSourceRgb = GAME_ADDRESS(0x00382DD8, 0x00399880);
constexpr uptr kH3dModelSetCombinerOperandRgb = GAME_ADDRESS(0x00382EC8, 0x0039994C);
constexpr uptr kH3dModelSetCombinerConstant = GAME_ADDRESS(0x00382C30, 0x003996FC);
constexpr uptr kH3dModelSetColorConstant = GAME_ADDRESS(0x00381EFC, 0x003988C8);
constexpr uptr kH3dModelSetMeshVisible = GAME_ADDRESS(0x0037FF7C, 0x0039694C);
constexpr uptr kH3dModelSetCombinerCombineAlpha = GAME_ADDRESS(0x00383148, 0x00399C44);
constexpr uptr kH3dModelSetCombinerSourceAlpha = GAME_ADDRESS(0x00382FB8, 0x00399A94);
constexpr uptr kH3dModelSetCombinerOperandAlpha = GAME_ADDRESS(0x00383174, 0x00399C70);
constexpr uptr kH3dModelSetTranslucencyKind = GAME_ADDRESS(0x003822B0, 0x00398C7C);
constexpr uptr kH3dModelSetAlphaTestEnable = GAME_ADDRESS(0x003821E8, 0x00398BB4);

constexpr uptr kH3dResourceInitialize = GAME_ADDRESS(0x0035D490, 0x00374E7C);
constexpr uptr kH3dResourceSetData = GAME_ADDRESS(0x0035C900, 0x00374310);
constexpr uptr kH3dResourceSetup = GAME_ADDRESS(0x0035D294, 0x00374C80);
constexpr uptr kH3dResourceRemoveData = GAME_ADDRESS(0x0035C91C, 0x0037432C);
constexpr uptr kH3dShaderModelInitialize = GAME_ADDRESS(0x00452B08, 0x00482458);
constexpr uptr kH3dShaderModelCreate = GAME_ADDRESS(0x0045295C, 0x004822E0);
constexpr uptr kH3dShaderModelVtable = GAME_ADDRESS(0x0059E724, 0x005E118C);
constexpr uptr kH3dPokemonShaderModelVtable = GAME_ADDRESS(0x0059E89C, 0x005E1318);

constexpr uptr kH3dModelSetTranslate = GAME_ADDRESS(0x0037FDA4, 0x00396774);
constexpr uptr kH3dModelGetTranslate = GAME_ADDRESS(0x0049D104, 0x004CCFD8);
constexpr uptr kH3dModelSetRotate = GAME_ADDRESS(0x0037FD10, 0x003966E0);
constexpr uptr kH3dModelSetScale = GAME_ADDRESS(0x00384B78, 0x0039B594);
constexpr uptr kH3dModelSetShaderSymbol = GAME_ADDRESS(0x003812D0, 0x00397BCC);
constexpr uptr kH3dModelDestroy = GAME_ADDRESS(0x003843B0, 0x0039AED4);

constexpr uptr kSceneRegister0 = GAME_ADDRESS(0x00374B94, 0x0038B600);
constexpr uptr kSceneRegister1 = GAME_ADDRESS(0x00374EBC, 0x0038B928);
constexpr uptr kSceneUnregister = GAME_ADDRESS(0x0037AC84, 0x003917D0);

constexpr uptr kWeatherEffectRain = GAME_ADDRESS(0x005EF13C, 0x00631144); // XY: unverified

constexpr uptr kParticleGetModelCount = GAME_ADDRESS(0x0049E164, 0x004CE068);
constexpr uptr kParticleGetModel = GAME_ADDRESS(0x0049E4B4, 0x004CE3B8);
constexpr uptr kParticleSetScale = GAME_ADDRESS(0x003869F8, 0x0039D260);
constexpr uptr kParticleCreate = GAME_ADDRESS(0x00385D90, 0x0039C5F4);
constexpr uptr kResourceAttachBufferAndSetup = GAME_ADDRESS(0x0038710C, 0x0039D974);

// DO NOT EXIST ?
constexpr uptr kGraphicsSetMaterial = GAME_ADDRESS(0x0014E5E4, 0x0014DA94);

constexpr uptr kApplyShader = GAME_ADDRESS(0x00381FE4, 0x003989B0);
constexpr uptr kApplyShader2 = GAME_ADDRESS(0x003717B0, 0x003881EC);

} // namespace address
} // namespace renderer
