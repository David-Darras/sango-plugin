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
 * @file camera_sections.h
 * @brief The entries of the camera that the camera pages of the overworld and
 *        of the battle share. Only the files of the pages use it.
 */

#pragma once

#include "common.h"
#include "overworld/patch/camera.h"
#include "ui/main_application.h"

namespace ui {

/// The names of overworld::CameraState, for WithArray().
extern const c8* kCameraStateNames[6];

/// Adds the "Free Camera" button, then the entry of the state of the camera.
/// `state` is the state that the page changes (overworld or battle).
void AddCameraState(MainApplication& app, overworld::CameraState& state);

/// Adds the section of the settings of `state`: the free position, the third
/// person, the first person or the orbit.
void AddCameraStateSection(MainApplication& app, overworld::CameraState state);

} // namespace ui
