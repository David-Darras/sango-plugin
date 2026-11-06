//! The packs of parts: a two letter tag, a count, then the offsets.

use crate::garc::u32_at;

/// Splits a pack into its parts. Returns the tag and the parts.
pub fn split(data: &[u8]) -> Result<([u8; 2], Vec<&[u8]>), String> {
    let tag = [*data.first().ok_or("empty pack")?, *data.get(1).ok_or("empty pack")?];
    let count = u16::from_le_bytes([data[2], data[3]]) as usize;
    let mut offsets = Vec::with_capacity(count + 1);
    for i in 0..=count {
        offsets.push(u32_at(data, 4 + 4 * i)? as usize);
    }
    let mut parts = Vec::with_capacity(count);
    for pair in offsets.windows(2) {
        parts.push(data.get(pair[0]..pair[1]).ok_or("a part is outside the pack")?);
    }
    Ok((tag, parts))
}
