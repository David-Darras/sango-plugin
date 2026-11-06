//! Decodes the pictures of the 3DS.
//!
//! The graphics chip stores a picture in tiles of 8 x 8 pixels. Inside a tile
//! the pixels follow a Z-order curve. The first row of tiles is the bottom
//! of the picture.

/// The sizes of the tables of the colour modifiers of ETC1.
const ETC1_TABLE: [[i32; 4]; 8] = [
    [2, 8, -2, -8],
    [5, 17, -5, -17],
    [9, 29, -9, -29],
    [13, 42, -13, -42],
    [18, 60, -18, -60],
    [24, 80, -24, -80],
    [33, 106, -33, -106],
    [47, 183, -47, -183],
];

fn clamp(v: i32) -> u8 {
    v.clamp(0, 255) as u8
}

fn extend5(v: i32) -> i32 {
    (v << 3) | (v >> 2)
}

fn signed3(v: u64) -> i32 {
    if v >= 4 { v as i32 - 8 } else { v as i32 }
}

/// Decodes one ETC1 block. The result has 16 colours, row after row.
fn etc1_block(block: u64) -> [[u8; 3]; 16] {
    let differential = (block >> 33) & 1 == 1;
    let flip = (block >> 32) & 1 == 1;
    let tables = [((block >> 37) & 7) as usize, ((block >> 34) & 7) as usize];
    let (base1, base2): ([i32; 3], [i32; 3]);
    if differential {
        let r = ((block >> 59) & 31) as i32;
        let g = ((block >> 51) & 31) as i32;
        let b = ((block >> 43) & 31) as i32;
        let r2 = (r + signed3((block >> 56) & 7)).clamp(0, 31);
        let g2 = (g + signed3((block >> 48) & 7)).clamp(0, 31);
        let b2 = (b + signed3((block >> 40) & 7)).clamp(0, 31);
        base1 = [extend5(r), extend5(g), extend5(b)];
        base2 = [extend5(r2), extend5(g2), extend5(b2)];
    } else {
        let c = |shift: u32| ((block >> shift) & 15) as i32 * 17;
        base1 = [c(60), c(52), c(44)];
        base2 = [c(56), c(48), c(40)];
    }
    let low = (block & 0xffff) as u32;
    let high = ((block >> 16) & 0xffff) as u32;
    let mut out = [[0u8; 3]; 16];
    for x in 0..4 {
        for y in 0..4 {
            let k = x * 4 + y;
            let index = (((high >> k) & 1) << 1 | ((low >> k) & 1)) as usize;
            let second = if flip { y >= 2 } else { x >= 2 };
            let (base, table) = if second { (base2, tables[1]) } else { (base1, tables[0]) };
            let m = ETC1_TABLE[table][index];
            out[y * 4 + x] = [clamp(base[0] + m), clamp(base[1] + m), clamp(base[2] + m)];
        }
    }
    out
}

/// The position of the pixel `n` of a tile of 8 x 8 pixels.
fn morton(n: usize) -> (usize, usize) {
    let x = (n & 1) | ((n >> 1) & 2) | ((n >> 2) & 4);
    let y = ((n >> 1) & 1) | ((n >> 2) & 2) | ((n >> 3) & 4);
    (x, y)
}

fn expand(v: u32, bits: u32) -> u8 {
    let max = (1u32 << bits) - 1;
    (v * 255 / max) as u8
}

/// The bytes that a picture needs, for a format of the chip.
pub fn byte_size(format: u32, width: usize, height: usize) -> usize {
    let pixels = width * height;
    match format {
        0 => pixels * 4,
        1 => pixels * 3,
        2..=6 => pixels * 2,
        7..=9 => pixels,
        0xa..=0xc => pixels / 2,
        _ => pixels,
    }
}

/// Decodes a picture to RGBA, with the first row at the top.
/// Returns `None` for a format that is not known.
pub fn decode(format: u32, width: usize, height: usize, data: &[u8]) -> Option<Vec<u8>> {
    if width == 0 || height == 0 || width % 8 != 0 || height % 8 != 0 {
        return None;
    }
    if data.len() < byte_size(format, width, height) {
        return None;
    }
    let mut out = vec![0u8; width * height * 4];
    let mut put = |x: usize, y: usize, rgba: [u8; 4]| {
        // The first row of the data is the bottom of the picture.
        let row = height - 1 - y;
        let at = (row * width + x) * 4;
        out[at..at + 4].copy_from_slice(&rgba);
    };
    let tiles_x = width / 8;
    let tiles_y = height / 8;
    match format {
        0xc | 0xd => {
            let block_bytes = if format == 0xc { 8 } else { 16 };
            let mut at = 0;
            for ty in 0..tiles_y {
                for tx in 0..tiles_x {
                    for n in 0..4 {
                        let (bx, by) = (n & 1, n >> 1);
                        let chunk = &data[at..at + block_bytes];
                        at += block_bytes;
                        let colour_bytes = &chunk[block_bytes - 8..];
                        let block = u64::from_le_bytes(colour_bytes.try_into().ok()?);
                        let alpha = if format == 0xd { u64::from_le_bytes(chunk[..8].try_into().ok()?) } else { 0 };
                        let pixels = etc1_block(block);
                        for py in 0..4 {
                            for px in 0..4 {
                                let c = pixels[py * 4 + px];
                                let a = if format == 0xd {
                                    expand(((alpha >> (4 * (px * 4 + py))) & 15) as u32, 4)
                                } else {
                                    255
                                };
                                put(tx * 8 + bx * 4 + px, ty * 8 + by * 4 + py, [c[0], c[1], c[2], a]);
                            }
                        }
                    }
                }
            }
        }
        _ => {
            let mut at = 0;
            let mut nibble = false;
            for ty in 0..tiles_y {
                for tx in 0..tiles_x {
                    for n in 0..64 {
                        let (x, y) = morton(n);
                        let rgba = match format {
                            0 => {
                                let p = &data[at..at + 4];
                                at += 4;
                                [p[3], p[2], p[1], p[0]]
                            }
                            1 => {
                                let p = &data[at..at + 3];
                                at += 3;
                                [p[2], p[1], p[0], 255]
                            }
                            2 | 3 | 4 => {
                                let v = u16::from_le_bytes([data[at], data[at + 1]]) as u32;
                                at += 2;
                                match format {
                                    2 => [expand(v >> 11, 5), expand((v >> 6) & 31, 5), expand((v >> 1) & 31, 5), if v & 1 == 1 { 255 } else { 0 }],
                                    3 => [expand(v >> 11, 5), expand((v >> 5) & 63, 6), expand(v & 31, 5), 255],
                                    _ => [expand(v >> 12, 4), expand((v >> 8) & 15, 4), expand((v >> 4) & 15, 4), expand(v & 15, 4)],
                                }
                            }
                            5 => {
                                let (a, l) = (data[at], data[at + 1]);
                                at += 2;
                                [l, l, l, a]
                            }
                            7 => {
                                let l = data[at];
                                at += 1;
                                [l, l, l, 255]
                            }
                            8 => {
                                let a = data[at];
                                at += 1;
                                [255, 255, 255, a]
                            }
                            9 => {
                                let v = data[at];
                                at += 1;
                                let (l, a) = (expand((v >> 4) as u32, 4), expand((v & 15) as u32, 4));
                                [l, l, l, a]
                            }
                            0xa | 0xb => {
                                let byte = data[at];
                                let v = if nibble { byte >> 4 } else { byte & 15 };
                                if nibble { at += 1; }
                                nibble = !nibble;
                                let e = expand(v as u32, 4);
                                if format == 0xa { [e, e, e, 255] } else { [255, 255, 255, e] }
                            }
                            _ => return None,
                        };
                        put(tx * 8 + x, ty * 8 + y, rgba);
                    }
                }
            }
        }
    }
    Some(out)
}
