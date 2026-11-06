//! Small pictures of the objects of the catalogue, drawn by the processor.

use std::collections::HashMap;

use crate::bch::Texture;
use crate::catalog::PreviewPart;

/// Draws an object seen from the front, a little from above and turned, in a
/// square picture of `size` pixels. The background is transparent.
pub fn render(parts: &[PreviewPart], textures: &HashMap<String, Texture>, size: usize) -> Vec<u8> {
    let mut image = vec![0u8; size * size * 4];
    let mut depth = vec![f32::MAX; size * size];

    // The turn of the camera: 35 degrees around the vertical, 28 degrees down.
    let (sy, cy) = 35f32.to_radians().sin_cos();
    let (sp, cp) = 28f32.to_radians().sin_cos();
    let view = |p: [f32; 3]| -> [f32; 3] {
        let x = p[0] * cy + p[2] * sy;
        let z = -p[0] * sy + p[2] * cy;
        let y = p[1] * cp - z * sp;
        let d = p[1] * sp + z * cp;
        [x, y, d]
    };

    let mut projected: Vec<Vec<[f32; 3]>> = Vec::new();
    let (mut low, mut high) = ([f32::MAX; 2], [f32::MIN; 2]);
    for part in parts {
        let list: Vec<[f32; 3]> = part.positions.iter().map(|&p| view(p)).collect();
        for v in &list {
            for a in 0..2 {
                low[a] = low[a].min(v[a]);
                high[a] = high[a].max(v[a]);
            }
        }
        projected.push(list);
    }
    if low[0] > high[0] {
        return image;
    }
    let extent = (high[0] - low[0]).max(high[1] - low[1]).max(0.001);
    let margin = 0.08;
    let scale = size as f32 * (1.0 - 2.0 * margin) / extent;
    let offset = [
        size as f32 * 0.5 - (low[0] + high[0]) * 0.5 * scale,
        size as f32 * 0.5 + (low[1] + high[1]) * 0.5 * scale,
    ];
    let screen = |v: [f32; 3]| [v[0] * scale + offset[0], offset[1] - v[1] * scale, v[2]];

    for (part, list) in parts.iter().zip(&projected) {
        let texture = textures.get(&part.picture);
        for t in part.indices.chunks_exact(3) {
            let (ia, ib, ic) = (t[0] as usize, t[1] as usize, t[2] as usize);
            let (Some(&a), Some(&b), Some(&c)) = (list.get(ia), list.get(ib), list.get(ic)) else { continue };
            let (a, b, c) = (screen(a), screen(b), screen(c));
            let area = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]);
            if area.abs() < 1e-6 {
                continue;
            }
            let min_x = a[0].min(b[0]).min(c[0]).floor().max(0.0) as usize;
            let max_x = (a[0].max(b[0]).max(c[0]).ceil().max(0.0) as usize).min(size - 1);
            let min_y = a[1].min(b[1]).min(c[1]).floor().max(0.0) as usize;
            let max_y = (a[1].max(b[1]).max(c[1]).ceil().max(0.0) as usize).min(size - 1);
            let (uv_a, uv_b, uv_c) = (part.uvs[ia], part.uvs[ib], part.uvs[ic]);
            for y in min_y..=max_y {
                for x in min_x..=max_x {
                    let (px, py) = (x as f32 + 0.5, y as f32 + 0.5);
                    let w0 = ((b[0] - px) * (c[1] - py) - (b[1] - py) * (c[0] - px)) / area;
                    let w1 = ((c[0] - px) * (a[1] - py) - (c[1] - py) * (a[0] - px)) / area;
                    let w2 = 1.0 - w0 - w1;
                    if w0 < 0.0 || w1 < 0.0 || w2 < 0.0 {
                        continue;
                    }
                    let z = w0 * a[2] + w1 * b[2] + w2 * c[2];
                    let at = y * size + x;
                    if z >= depth[at] {
                        continue;
                    }
                    let u = w0 * uv_a[0] + w1 * uv_b[0] + w2 * uv_c[0];
                    let v = w0 * uv_a[1] + w1 * uv_b[1] + w2 * uv_c[1];
                    let rgba = match texture {
                        Some(tex) if tex.width > 0 && tex.height > 0 => {
                            let tx = ((u.rem_euclid(1.0)) * tex.width as f32) as usize % tex.width;
                            let ty = ((v.rem_euclid(1.0)) * tex.height as f32) as usize % tex.height;
                            let k = (ty * tex.width + tx) * 4;
                            [tex.rgba[k], tex.rgba[k + 1], tex.rgba[k + 2], tex.rgba[k + 3]]
                        }
                        _ => [170, 170, 170, 255],
                    };
                    if rgba[3] < 128 {
                        continue;
                    }
                    depth[at] = z;
                    image[at * 4..at * 4 + 4].copy_from_slice(&[rgba[0], rgba[1], rgba[2], 255]);
                }
            }
        }
    }
    image
}
