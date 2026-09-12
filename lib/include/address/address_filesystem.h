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

#define ADDRESS_FILE_OPEN (0x00123FA8)
#define ADDRESS_FILE_READ (0x00145CF0)
#define ADDRESS_FILE_WRITE (0x001F5BE0)
#define ADDRESS_FS_MOUNT_SDMC (0x001F7CFC)
#define ADDRESS_FS_CREATE_FILE (0x001F5C50)
#define ADDRESS_FS_DELETE_FILE (0x001F5CCC)
#define ADDRESS_FS_CREATE_DIRECTORY (0x001F5F0C)
#define ADDRESS_ARCHIVE_FILENAME_TABLE (0x005F5050)
#define ADDRESS_ARCHIVE_READ_FILE_ASYNC (0x0036DC68)
#define ADDRESS_ARCHIVE_READ_FILE_ASYNC_2 (0x0036ED10)
#define ADDRESS_ARCHIVE_LOAD_DATA (0x0036F030)
#define ADDRESS_ARCHIVE_LOAD_DATA_2 (0x00128E4C)
#define ADDRESS_ARCHIVE_INITIALIZE (0x0011CA2C)
#define ADDRESS_ARCHIVE_GET_FILE_SIZE (0x0011CA10)
#define ADDRESS_ARCHIVE_LOAD_FILE (0x00128B34)
#define ADDRESS_ARCHIVE_LOAD_COMPRESSED_FILE (0x00137A30)
