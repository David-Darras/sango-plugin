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
