//! Reads the archives (GARC files) of the game.

use std::fs;
use std::path::Path;

fn u16_at(data: &[u8], pos: usize) -> Result<u16, String> {
    data.get(pos..pos + 2)
        .map(|b| u16::from_le_bytes([b[0], b[1]]))
        .ok_or_else(|| format!("read past the end at {pos:#x}"))
}

pub fn u32_at(data: &[u8], pos: usize) -> Result<u32, String> {
    data.get(pos..pos + 4)
        .map(|b| u32::from_le_bytes([b[0], b[1], b[2], b[3]]))
        .ok_or_else(|| format!("read past the end at {pos:#x}"))
}

/// Returns the files of an archive. Each file is a list of parts.
/// Most files have one part, so `files[3][0]` is the data of file 3.
pub fn read(path: &Path) -> Result<Vec<Vec<Vec<u8>>>, String> {
    let data = fs::read(path).map_err(|e| format!("{}: {e}", path.display()))?;
    if data.get(..4) != Some(b"CRAG") {
        return Err(format!("{} is not an archive", path.display()));
    }
    let header_size = u32_at(&data, 4)? as usize;
    let data_offset = u32_at(&data, 16)? as usize;
    let mut pos = header_size;
    if data.get(pos..pos + 4) != Some(b"OTAF") {
        return Err("the table of files is missing".into());
    }
    let size = u32_at(&data, pos + 4)? as usize;
    let count = u16_at(&data, pos + 8)? as usize;
    let mut offsets = Vec::with_capacity(count);
    for i in 0..count {
        offsets.push(u32_at(&data, pos + 12 + 4 * i)? as usize);
    }
    pos += size;
    if data.get(pos..pos + 4) != Some(b"BTAF") {
        return Err("the table of sizes is missing".into());
    }
    let base = pos + 12;
    let mut files = Vec::with_capacity(count);
    for offset in offsets {
        let mut cursor = base + offset;
        let mask = u32_at(&data, cursor)?;
        cursor += 4;
        let mut parts = Vec::new();
        for bit in 0..32 {
            if mask & (1 << bit) != 0 {
                let start = u32_at(&data, cursor)? as usize;
                let length = u32_at(&data, cursor + 8)? as usize;
                cursor += 12;
                let from = data_offset + start;
                let part = data
                    .get(from..from + length)
                    .ok_or("a file is outside the archive")?;
                parts.push(part.to_vec());
            }
        }
        files.push(parts);
    }
    Ok(files)
}

/// The archives of the game that the tool reads. Each one is a file of the
/// RomFS, found by its number: 13 is the file `a/0/1/3`.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Archive {
    /// The maps: settings, events.
    Maps,
    /// The tilesets: pictures, light, models of the buildings.
    Tilesets,
    /// The 3D models of the characters.
    CharacterModels,
    /// For each character model: which model file it uses.
    CharacterModelParams,
    /// The 3D models of the doors, the trees and the other props that a block places.
    Props,
    /// The blocks of the map.
    Blocks,
    /// The matrices: which block is where.
    Matrices,
}

impl Archive {
    /// The number of the archive in the game.
    pub fn number(self) -> usize {
        match self {
            Archive::Maps => 13,
            Archive::Tilesets => 14,
            Archive::CharacterModels => 21,
            Archive::CharacterModelParams => 22,
            Archive::Props => 23,
            Archive::Blocks => 39,
            Archive::Matrices => 40,
        }
    }
}

/// The file of an archive in the dump of the RomFS: `dir` is the folder of the
/// dump, and the files are `a/0/1/3`, `a/0/3/9`...
pub fn locate(dir: &str, archive: Archive) -> std::path::PathBuf {
    let n = archive.number();
    std::path::PathBuf::from(dir).join("a").join("0").join((n / 10).to_string()).join((n % 10).to_string())
}
