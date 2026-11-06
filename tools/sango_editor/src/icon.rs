//! The small logo of the application: a block of the map (an isometric cube)
//! on a rounded blue square. It is drawn here, so the tool needs no picture file.

/// Is the point inside the convex shape with these corners (in any order of turning)?
fn inside(corners: &[(f32, f32)], x: f32, y: f32) -> bool {
    let mut sign = 0.0f32;
    for k in 0..corners.len() {
        let (a, b) = (corners[k], corners[(k + 1) % corners.len()]);
        let cross = (b.0 - a.0) * (y - a.1) - (b.1 - a.1) * (x - a.0);
        if cross != 0.0 {
            if sign != 0.0 && cross.signum() != sign.signum() {
                return false;
            }
            sign = cross;
        }
    }
    true
}

/// The colour of the logo at a point of the square [0, 1] x [0, 1], and how
/// much of the point is covered (the rounded corners are empty).
fn sample(u: f32, v: f32) -> Option<[f32; 3]> {
    // The rounded square.
    let (cx, cy, half, radius) = (0.5, 0.5, 0.47, 0.19);
    let (dx, dy) = ((u - cx).abs() - (half - radius), (v - cy).abs() - (half - radius));
    let outside = (dx.max(0.0).powi(2) + dy.max(0.0).powi(2)).sqrt() + dx.max(dy).min(0.0) - radius;
    if outside > 0.0 {
        return None;
    }
    // A blue that gets darker toward the bottom.
    let shade = |top: [f32; 3], bottom: [f32; 3], t: f32| [top[0] + (bottom[0] - top[0]) * t, top[1] + (bottom[1] - top[1]) * t, top[2] + (bottom[2] - top[2]) * t];
    let mut colour = shade([0.45, 0.72, 1.0], [0.18, 0.40, 0.80], v);
    // The cube: its top, its left side and its right side.
    let top = [(0.5, 0.17), (0.80, 0.335), (0.5, 0.50), (0.20, 0.335)];
    let left = [(0.20, 0.335), (0.5, 0.50), (0.5, 0.83), (0.20, 0.665)];
    let right = [(0.5, 0.50), (0.80, 0.335), (0.80, 0.665), (0.5, 0.83)];
    if inside(&top, u, v) {
        colour = [1.0, 1.0, 1.0];
    } else if inside(&left, u, v) {
        colour = [0.80, 0.88, 0.98];
    } else if inside(&right, u, v) {
        colour = [0.62, 0.76, 0.94];
    }
    // A small mark on the top of the cube: where an object goes.
    let (mx, my) = (u - 0.5, (v - 0.335) * 1.8);
    if (mx * mx + my * my).sqrt() < 0.07 {
        colour = [0.95, 0.45, 0.25];
    }
    Some(colour)
}

/// The logo as RGBA bytes, `size` pixels on each side (with smooth edges).
pub fn render(size: usize) -> Vec<u8> {
    let mut out = vec![0u8; size * size * 4];
    let samples = 3;
    for y in 0..size {
        for x in 0..size {
            let (mut sum, mut covered) = ([0.0f32; 3], 0.0f32);
            for sy in 0..samples {
                for sx in 0..samples {
                    let u = (x as f32 + (sx as f32 + 0.5) / samples as f32) / size as f32;
                    let v = (y as f32 + (sy as f32 + 0.5) / samples as f32) / size as f32;
                    if let Some(c) = sample(u, v) {
                        for a in 0..3 {
                            sum[a] += c[a];
                        }
                        covered += 1.0;
                    }
                }
            }
            if covered > 0.0 {
                let at = (y * size + x) * 4;
                for a in 0..3 {
                    out[at + a] = (sum[a] / covered * 255.0).round() as u8;
                }
                out[at + 3] = (covered / (samples * samples) as f32 * 255.0).round() as u8;
            }
        }
    }
    out
}

/// Writes the logo in a PNG file.
pub fn write_png(path: &str, size: usize) -> Result<(), String> {
    let file = std::fs::File::create(path).map_err(|e| format!("{path}: {e}"))?;
    let mut encoder = png::Encoder::new(std::io::BufWriter::new(file), size as u32, size as u32);
    encoder.set_color(png::ColorType::Rgba);
    encoder.set_depth(png::BitDepth::Eight);
    let mut writer = encoder.write_header().map_err(|e| e.to_string())?;
    writer.write_image_data(&render(size)).map_err(|e| e.to_string())
}
