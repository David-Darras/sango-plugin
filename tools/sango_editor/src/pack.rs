//! The packs of parts: a two letter tag, a count, then the offsets.

use crate::garc::u32_at;

/// Splits a pack into its parts. Returns the tag and the parts.
pub fn split(data: &[u8]) -> Result<([u8; 2], Vec<&[u8]>), String> {
    let tag = [*data.first().ok_or("empty pack")?, *data.get(1).ok_or("empty pack")?];
    let parts = ranges(data)?
        .into_iter()
        .map(|(start, end)| data.get(start..end).ok_or("a part is outside the pack"))
        .collect::<Result<Vec<_>, _>>()?;
    Ok((tag, parts))
}

/// The start and the end of each part of a pack, in the data of the pack.
pub fn ranges(data: &[u8]) -> Result<Vec<(usize, usize)>, String> {
    let count = u16::from_le_bytes([*data.get(2).ok_or("empty pack")?, *data.get(3).ok_or("empty pack")?]) as usize;
    let mut offsets = Vec::with_capacity(count + 1);
    for i in 0..=count {
        offsets.push(u32_at(data, 4 + 4 * i)? as usize);
    }
    Ok(offsets.windows(2).map(|p| (p[0], p[1])).collect())
}

/// The alignment of the parts of a pack.
pub const ALIGNMENT: usize = 0x80;

/// Returns a pack where one part has new data. The parts after it move, and
/// the table of offsets follows. `new_part` must have a size that is a
/// multiple of the alignment.
pub fn replace_part(pack: &[u8], index: usize, new_part: &[u8]) -> Result<Vec<u8>, String> {
    replace_part_with(pack, index, new_part, ALIGNMENT)
}

/// Like `replace_part`, for a pack whose parts have another alignment (the
/// packs of the maps: 4 bytes).
pub fn replace_part_with(pack: &[u8], index: usize, new_part: &[u8], alignment: usize) -> Result<Vec<u8>, String> {
    let ranges = ranges(pack)?;
    if index >= ranges.len() || new_part.len() % alignment != 0 {
        return Err("cannot replace this part".into());
    }
    let header_size = ranges[0].0;
    let mut out = pack[..header_size].to_vec();
    let mut offsets = Vec::with_capacity(ranges.len() + 1);
    for (i, &(start, end)) in ranges.iter().enumerate() {
        offsets.push(out.len());
        if i == index {
            out.extend_from_slice(new_part);
        } else {
            out.extend_from_slice(&pack[start..end]);
        }
    }
    offsets.push(out.len());
    for (i, offset) in offsets.iter().enumerate() {
        out[4 + 4 * i..8 + 4 * i].copy_from_slice(&(*offset as u32).to_le_bytes());
    }
    Ok(out)
}
