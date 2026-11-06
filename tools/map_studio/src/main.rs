//! Map Studio: an editor for the maps of Omega Ruby and Alpha Sapphire.
//!
//! This version only looks at the data of the game.

mod bch;
mod data;
mod garc;
mod lz11;
mod pack;
mod tex;
mod viewer;

use std::fmt::Write as _;

const USAGE: &str = "usage:
  map-studio meshes <arc dir> <block id> [out.obj]
  map-studio view <arc dir> <block id> [area id]
  map-studio sheet <arc dir> <block id> <out.png> [area id]
  map-studio picture <arc dir> <block id> <name> <out.png>";

/// Prints the meshes of one block. With a path, also writes them as an OBJ file.
fn meshes(arc_dir: &str, block: usize, obj_path: Option<&str>) -> Result<(), String> {
    let parts = data::read_block(arc_dir, block)?;
    let meshes = data::block_meshes(&parts)?;
    let mut obj = String::new();
    let mut base = 1usize;
    println!("{} meshes", meshes.len());
    for (i, mesh) in meshes.iter().enumerate() {
        let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
        for p in &mesh.positions {
            for a in 0..3 {
                low[a] = low[a].min(p[a]);
                high[a] = high[a].max(p[a]);
            }
        }
        println!(
            "  {i:2} {:<22} tex {:<22} {:5} vertices {:5} triangles  x {:.0}..{:.0}  y {:.0}..{:.0}  z {:.0}..{:.0}",
            mesh.material, data::picture_of(mesh), mesh.positions.len(), mesh.indices.len() / 3,
            low[0], high[0], low[1], high[1], low[2], high[2]
        );
        if !mesh.uvs.is_empty() {
            let (mut lo, mut hi) = ([f32::MAX; 2], [f32::MIN; 2]);
            for t in &mesh.uvs {
                for a in 0..2 {
                    lo[a] = lo[a].min(t[a]);
                    hi[a] = hi[a].max(t[a]);
                }
            }
            println!("       uv  u {:.2}..{:.2}  v {:.2}..{:.2}", lo[0], hi[0], lo[1], hi[1]);
        }
        if mesh.positions.is_empty() {
            continue;
        }
        let _ = writeln!(obj, "o {i}_{}", mesh.material);
        for p in &mesh.positions {
            let _ = writeln!(obj, "v {} {} {}", p[0], p[1], p[2]);
        }
        for t in &mesh.uvs {
            let _ = writeln!(obj, "vt {} {}", t[0], 1.0 - t[1]);
        }
        let has_uv = mesh.uvs.len() == mesh.positions.len();
        for t in mesh.indices.chunks_exact(3) {
            let v = |k: usize| {
                let j = t[k] as usize + base;
                if has_uv { format!("{j}/{j}") } else { j.to_string() }
            };
            let _ = writeln!(obj, "f {} {} {}", v(0), v(1), v(2));
        }
        base += mesh.positions.len();
    }
    if let Some(path) = obj_path {
        std::fs::write(path, obj).map_err(|e| format!("{path}: {e}"))?;
        println!("written {path}");
    }
    Ok(())
}

/// Opens a block in a window.
fn view(arc_dir: &str, block: usize, area: Option<usize>) -> Result<(), String> {
    let scene = data::load_scene(arc_dir, block, area)?;
    let with_picture = scene
        .meshes
        .iter()
        .filter(|m| scene.textures.contains_key(data::picture_of(m)))
        .count();
    println!(
        "area {:?}: {} pictures, {} of {} meshes have a picture",
        scene.area, scene.textures.len(), with_picture, scene.meshes.len()
    );
    let title = format!("Block {block}");
    viewer::run(scene, title)
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let number = |i: usize| args.get(i).and_then(|s| s.parse::<usize>().ok());
    let result = match args.get(1).map(String::as_str) {
        Some("meshes") if args.len() >= 4 => meshes(&args[2], number(3).unwrap_or(0), args.get(4).map(String::as_str)),
        Some("view") if args.len() >= 4 => view(&args[2], number(3).unwrap_or(0), number(4)),
        Some("sheet") if args.len() >= 5 => sheet(&args[2], number(3).unwrap_or(0), number(5), &args[4]),
        Some("picture") if args.len() >= 6 => picture(&args[2], number(3).unwrap_or(0), &args[4], &args[5]),
        Some("groups") if args.len() >= 5 => groups(&args[2], number(3).unwrap_or(0), &args[4]),
        _ => Err(USAGE.to_string()),
    };
    if let Err(message) = result {
        eprintln!("{message}");
        std::process::exit(1);
    }
}

/// Writes the pictures that the meshes of a block use in one PNG file, to
/// check the decoding.
fn sheet(arc_dir: &str, block: usize, area: Option<usize>, path: &str) -> Result<(), String> {
    let scene = data::load_scene(arc_dir, block, area)?;
    let cell = 128usize;
    let mut used: Vec<&bch::Texture> = Vec::new();
    for mesh in &scene.meshes {
        if let Some(t) = scene.textures.get(data::picture_of(mesh)) {
            if !used.iter().any(|u| u.name == t.name) {
                used.push(t);
            }
        }
    }
    let columns = 6usize;
    let rows = used.len().div_ceil(columns).max(1);
    let (w, h) = (columns * cell, rows * cell);
    let mut image = vec![40u8; w * h * 4];
    for (n, t) in used.iter().enumerate() {
        let (ox, oy) = ((n % columns) * cell, (n / columns) * cell);
        for y in 0..cell {
            for x in 0..cell {
                let (sx, sy) = (x * t.width / cell, y * t.height / cell);
                let src = (sy * t.width + sx) * 4;
                let dst = ((oy + y) * w + ox + x) * 4;
                image[dst..dst + 4].copy_from_slice(&t.rgba[src..src + 4]);
            }
        }
        println!("{n}: {} {}x{}", t.name, t.width, t.height);
    }
    let file = std::fs::File::create(path).map_err(|e| format!("{path}: {e}"))?;
    let mut encoder = png::Encoder::new(file, w as u32, h as u32);
    encoder.set_color(png::ColorType::Rgba);
    encoder.set_depth(png::BitDepth::Eight);
    encoder
        .write_header()
        .and_then(|mut writer| writer.write_image_data(&image))
        .map_err(|e| e.to_string())
}

/// Writes one picture of an area pack at its real size, on a checkerboard
/// so that the transparent parts are visible.
fn picture(arc_dir: &str, block: usize, name: &str, path: &str) -> Result<(), String> {
    let scene = data::load_scene(arc_dir, block, None)?;
    let t = scene.textures.get(name).ok_or("no such picture")?;
    let mut image = vec![0u8; t.width * t.height * 4];
    for y in 0..t.height {
        for x in 0..t.width {
            let i = (y * t.width + x) * 4;
            let back = if (x / 8 + y / 8) % 2 == 0 { 90 } else { 150 };
            let a = t.rgba[i + 3] as u32;
            for c in 0..3 {
                image[i + c] = ((t.rgba[i + c] as u32 * a + back * (255 - a)) / 255) as u8;
            }
            image[i + 3] = 255;
        }
    }
    let file = std::fs::File::create(path).map_err(|e| format!("{path}: {e}"))?;
    let mut encoder = png::Encoder::new(file, t.width as u32, t.height as u32);
    encoder.set_color(png::ColorType::Rgba);
    encoder.set_depth(png::BitDepth::Eight);
    encoder
        .write_header()
        .and_then(|mut writer| writer.write_image_data(&image))
        .map_err(|e| e.to_string())
}

/// Prints, for each connected group of triangles of a mesh (a tree, a bush),
/// where it is and which part of its picture it uses.
fn groups(arc_dir: &str, block: usize, name: &str) -> Result<(), String> {
    let parts = data::read_block(arc_dir, block)?;
    let meshes = data::block_meshes(&parts)?;
    let mesh = meshes.iter().find(|m| m.material == name).ok_or("no such mesh")?;
    let n = mesh.positions.len();
    let mut parent: Vec<usize> = (0..n).collect();
    fn find(p: &mut Vec<usize>, mut x: usize) -> usize {
        while p[x] != x {
            p[x] = p[p[x]];
            x = p[x];
        }
        x
    }
    for t in mesh.indices.chunks_exact(3) {
        let (a, b, c) = (t[0] as usize, t[1] as usize, t[2] as usize);
        let (ra, rb) = (find(&mut parent, a), find(&mut parent, b));
        parent[ra] = rb;
        let (rb2, rc) = (find(&mut parent, b), find(&mut parent, c));
        parent[rb2] = rc;
    }
    let mut map: std::collections::BTreeMap<usize, Vec<usize>> = Default::default();
    for v in 0..n {
        let r = find(&mut parent, v);
        map.entry(r).or_default().push(v);
    }
    for (_, vs) in map {
        let (mut lo, mut hi) = ([f32::MAX; 3], [f32::MIN; 3]);
        let (mut ulo, mut uhi) = ([f32::MAX; 2], [f32::MIN; 2]);
        for &v in &vs {
            for a in 0..3 {
                lo[a] = lo[a].min(mesh.positions[v][a]);
                hi[a] = hi[a].max(mesh.positions[v][a]);
            }
            for a in 0..2 {
                ulo[a] = ulo[a].min(mesh.uvs[v][a]);
                uhi[a] = uhi[a].max(mesh.uvs[v][a]);
            }
        }
        println!(
            "{:4} verts at x {:5.0} y {:3.0}..{:3.0} z {:5.0}   u {:5.2}..{:5.2}  v {:4.2}..{:4.2}",
            vs.len(), (lo[0] + hi[0]) / 2.0, lo[1], hi[1], (lo[2] + hi[2]) / 2.0, ulo[0], uhi[0], ulo[1], uhi[1]
        );
    }
    Ok(())
}
