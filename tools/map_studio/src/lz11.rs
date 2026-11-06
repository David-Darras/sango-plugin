//! The LZ11 compression of the 3DS.

/// Decompresses data that starts with the byte 0x11.
pub fn decompress(data: &[u8]) -> Result<Vec<u8>, String> {
    if data.len() < 4 || data[0] != 0x11 {
        return Err("the data is not LZ11".into());
    }
    let size = data[1] as usize | (data[2] as usize) << 8 | (data[3] as usize) << 16;
    let mut out: Vec<u8> = Vec::with_capacity(size);
    let mut pos = 4;
    let byte = |p: usize| data.get(p).copied().ok_or("the LZ11 data is too short");
    while out.len() < size {
        let flags = byte(pos)?;
        pos += 1;
        for bit in 0..8 {
            if out.len() >= size {
                break;
            }
            if flags & (0x80 >> bit) == 0 {
                out.push(byte(pos)?);
                pos += 1;
                continue;
            }
            let first = byte(pos)? as usize;
            let (length, distance);
            match first >> 4 {
                0 => {
                    let b1 = byte(pos + 1)? as usize;
                    let b2 = byte(pos + 2)? as usize;
                    length = ((first & 15) << 4 | b1 >> 4) + 0x11;
                    distance = ((b1 & 15) << 8 | b2) + 1;
                    pos += 3;
                }
                1 => {
                    let b1 = byte(pos + 1)? as usize;
                    let b2 = byte(pos + 2)? as usize;
                    let b3 = byte(pos + 3)? as usize;
                    length = ((first & 15) << 12 | b1 << 4 | b2 >> 4) + 0x111;
                    distance = ((b2 & 15) << 8 | b3) + 1;
                    pos += 4;
                }
                kind => {
                    let b1 = byte(pos + 1)? as usize;
                    length = kind + 1;
                    distance = ((first & 15) << 8 | b1) + 1;
                    pos += 2;
                }
            }
            if distance > out.len() {
                return Err("the LZ11 data points before its start".into());
            }
            for _ in 0..length {
                let value = out[out.len() - distance];
                out.push(value);
            }
        }
    }
    Ok(out)
}
