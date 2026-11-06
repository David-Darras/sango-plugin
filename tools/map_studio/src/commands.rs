//! The commands of the program that work without a window.

use std::fmt::Write as _;
use std::path::Path;

use crate::edit::{mesh_groups, CopyRequest, EditableBlock};
use crate::{bch, data};

/// Prints the meshes of one block. With a path, also writes them as an OBJ file.
pub fn meshes(arc_dir: &str, block: usize, obj_path: Option<&str>) -> Result<(), String> {
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

fn write_png(path: &str, width: usize, height: usize, rgba: &[u8]) -> Result<(), String> {
    let file = std::fs::File::create(path).map_err(|e| format!("{path}: {e}"))?;
    let mut encoder = png::Encoder::new(file, width as u32, height as u32);
    encoder.set_color(png::ColorType::Rgba);
    encoder.set_depth(png::BitDepth::Eight);
    encoder
        .write_header()
        .and_then(|mut writer| writer.write_image_data(rgba))
        .map_err(|e| e.to_string())
}

/// Writes the pictures that the meshes of a block use in one PNG file, to
/// check the decoding.
pub fn sheet(arc_dir: &str, block: usize, area: Option<usize>, path: &str) -> Result<(), String> {
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
    write_png(path, w, h, &image)
}

/// Writes one picture at its real size, on a checkerboard so that the
/// transparent parts are visible.
pub fn picture(arc_dir: &str, block: usize, name: &str, path: &str) -> Result<(), String> {
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
    write_png(path, t.width, t.height, &image)
}

/// Prints the groups of vertices of the meshes that have a name: the objects
/// of a block (a tree, a bush, a house) with their place.
pub fn groups(block: &EditableBlock, name: &str) {
    for (index, (_, mesh)) in block.meshes.iter().enumerate() {
        if mesh.material != name {
            continue;
        }
        println!("mesh {index} ({})", mesh.material);
        for (number, vertices) in mesh_groups(mesh).iter().enumerate() {
            let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
            for &v in vertices {
                for a in 0..3 {
                    low[a] = low[a].min(mesh.positions[v][a]);
                    high[a] = high[a].max(mesh.positions[v][a]);
                }
            }
            println!(
                "  group {number:4}: {:4} vertices  x {:6.0}..{:6.0}  y {:4.0}..{:4.0}  z {:6.0}..{:6.0}",
                vertices.len(), low[0], high[0], low[1], high[1], low[2], high[2]
            );
        }
    }
}

/// Lists the groups of a block of the archive.
pub fn list_groups(arc_dir: &str, block: usize, name: &str) -> Result<(), String> {
    groups(&EditableBlock::load(arc_dir, block)?, name);
    Ok(())
}

/// Writes a block as it is, compressed again. This checks that the game
/// accepts a file that the tool wrote.
pub fn roundtrip(arc_dir: &str, block: usize, out_dir: &str) -> Result<(), String> {
    let edited = EditableBlock::load(arc_dir, block)?;
    let path = edited.save(out_dir)?;
    println!("written {}", path.display());
    Ok(())
}

/// The groups of vertices whose centre is near a point (seen from above), in
/// the meshes that have one of the materials. A tree is made of many groups
/// in several meshes, so a place selects all of them. Returns the number of
/// the mesh and the vertices of each group.
fn groups_near(block: &EditableBlock, materials: &str, place: [f32; 2], radius: f32) -> Vec<(usize, Vec<usize>)> {
    let wanted: Vec<&str> = materials.split(',').collect();
    let mut found = Vec::new();
    for (index, (_, mesh)) in block.meshes.iter().enumerate() {
        if !wanted.contains(&mesh.material.as_str()) {
            continue;
        }
        for vertices in mesh_groups(mesh) {
            let (mut low, mut high) = ([f32::MAX; 2], [f32::MIN; 2]);
            for &v in &vertices {
                low[0] = low[0].min(mesh.positions[v][0]);
                high[0] = high[0].max(mesh.positions[v][0]);
                low[1] = low[1].min(mesh.positions[v][2]);
                high[1] = high[1].max(mesh.positions[v][2]);
            }
            let centre = [(low[0] + high[0]) / 2.0, (low[1] + high[1]) / 2.0];
            let distance = ((centre[0] - place[0]).powi(2) + (centre[1] - place[1]).powi(2)).sqrt();
            // A group with no triangle is what a deleted object leaves.
            let has_triangle = mesh.indices.iter().any(|&i| vertices.contains(&(i as usize)));
            if distance <= radius && has_triangle {
                found.push((index, vertices));
            }
        }
    }
    found
}

/// Moves what is near a point and writes the block.
pub fn move_near(
    arc_dir: &str,
    block: usize,
    materials: &str,
    place: [f32; 2],
    radius: f32,
    delta: [f32; 3],
    out_dir: &str,
) -> Result<(), String> {
    let mut edited = EditableBlock::load(arc_dir, block)?;
    let found = groups_near(&edited, materials, place, radius);
    if found.is_empty() {
        return Err("nothing near this place".into());
    }
    let mut moved = 0;
    for (mesh, vertices) in &found {
        edited.translate(*mesh, vertices, delta)?;
        moved += vertices.len();
    }
    let path = edited.save(out_dir)?;
    println!("moved {moved} vertices; written {}", path.display());
    Ok(())
}

/// Copies what is near a point to another place and writes the block.
pub fn copy_near(
    arc_dir: &str,
    block: usize,
    materials: &str,
    place: [f32; 2],
    radius: f32,
    delta: [f32; 3],
    out_dir: &str,
) -> Result<(), String> {
    let mut edited = EditableBlock::load(arc_dir, block)?;
    let found = groups_near(&edited, materials, place, radius);
    if found.is_empty() {
        return Err("nothing near this place".into());
    }
    let groups = found.len();
    let added = edited.copy_groups(&[CopyRequest { source: found, delta }])?;
    let vertices: usize = added[0].iter().map(|(_, _, count)| count).sum();
    let path = edited.save(out_dir)?;
    println!("added {vertices} vertices in {groups} groups; written {}", path.display());
    Ok(())
}

/// Deletes what is near a point and writes the block.
pub fn delete_near(
    arc_dir: &str,
    block: usize,
    materials: &str,
    place: [f32; 2],
    radius: f32,
    out_dir: &str,
) -> Result<(), String> {
    let mut edited = EditableBlock::load(arc_dir, block)?;
    let found = groups_near(&edited, materials, place, radius);
    if found.is_empty() {
        return Err("nothing near this place".into());
    }
    let mut removed = 0;
    for (mesh, vertices) in &found {
        removed += edited.delete_group(*mesh, vertices)?;
    }
    let path = edited.save(out_dir)?;
    println!("removed {removed} triangles in {} groups; written {}", found.len(), path.display());
    Ok(())
}

/// Reads a block file that the tool wrote and prints its groups, to check
/// that the file can be read again.
pub fn check(file: &str, block: usize, material: &str) -> Result<(), String> {
    let edited = EditableBlock::from_file(Path::new(file), block)?;
    println!("{} meshes", edited.meshes.len());
    for (_, mesh) in &edited.meshes {
        println!("  {:<22} {:5} triangles", mesh.material, mesh.indices.len() / 3);
    }
    if !material.is_empty() && material != "-" {
        groups(&edited, material);
    }
    Ok(())
}

/// A check of the editing session without a window: copies one object,
/// moves it, deletes another, undoes and does again, then writes the block.
pub fn editor_test(arc_dir: &str, block: usize, out_dir: &str) -> Result<(), String> {
    use crate::editor::{Editor, Instance};
    let mut editor = Editor::new(arc_dir, block)?;
    println!("{} objects in {} selectable meshes", editor.clusters.len(), editor.selectable.iter().filter(|s| **s).count());
    // The object that holds the pine at (-321, -25).
    let wanted = editor
        .clusters
        .iter()
        .position(|c| {
            let (x, z) = ((c.low[0] + c.high[0]) / 2.0, (c.low[2] + c.high[2]) / 2.0);
            (x + 321.0).abs() < 40.0 && (z + 25.0).abs() < 40.0
        })
        .ok_or("no object there")?;
    let c = &editor.clusters[wanted];
    println!("pine object {wanted}: {} parts, box x {:.0}..{:.0} z {:.0}..{:.0}", c.parts.len(), c.low[0], c.high[0], c.low[2], c.high[2]);
    let triangles = |e: &Editor| e.meshes().iter().map(|(_, m)| m.indices.len() / 3).sum::<usize>();
    let before = triangles(&editor);
    editor.change(|v| v.push(Instance { cluster: wanted, offset: [54.0, 0.0, 0.0], copy: true, deleted: false }))?;
    println!("after a copy: {} -> {} triangles", before, triangles(&editor));
    let copy = editor.instances.len() - 1;
    editor.change(|v| v[copy].offset[0] += 18.0)?;
    println!("after moving the copy: {} triangles", triangles(&editor));
    editor.change(|v| v[wanted].deleted = true)?;
    println!("after deleting the original: {} triangles", triangles(&editor));
    editor.undo()?;
    println!("after undo: {} triangles", triangles(&editor));
    editor.redo()?;
    println!("after redo: {} triangles", triangles(&editor));
    println!("{}", editor.export(out_dir)?);
    Ok(())
}

/// Prints the groups of a mesh in the order of their first vertex, to see how
/// the objects follow each other in the file.
pub fn order(arc_dir: &str, block: usize, material: &str, limit: usize) -> Result<(), String> {
    let edited = EditableBlock::load(arc_dir, block)?;
    for (index, (_, mesh)) in edited.meshes.iter().enumerate() {
        if mesh.material != material {
            continue;
        }
        let mut groups = mesh_groups(mesh);
        groups.sort_by_key(|g| g[0]);
        println!("mesh {index} {material}: {} groups", groups.len());
        for g in groups.iter().take(limit) {
            let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
            for &v in g {
                for a in 0..3 {
                    low[a] = low[a].min(mesh.positions[v][a]);
                    high[a] = high[a].max(mesh.positions[v][a]);
                }
            }
            println!(
                "  first {:5} count {:3}  centre x {:6.0} z {:6.0}  y {:3.0}..{:3.0}",
                g[0], g.len(), (low[0] + high[0]) / 2.0, (low[2] + high[2]) / 2.0, low[1], high[1]
            );
        }
    }
    Ok(())
}

/// Prints how the objects of a block are made: how many objects have each
/// composition (the meshes that they use and how many vertices in each).
pub fn objects(arc_dir: &str, block: usize) -> Result<(), String> {
    let editor = crate::editor::Editor::new(arc_dir, block)?;
    let mut kinds: std::collections::BTreeMap<String, (usize, f32)> = Default::default();
    for cluster in &editor.clusters {
        let mut parts: Vec<String> =
            cluster.parts.iter().map(|(m, v)| format!("{}:{}", editor.material(*m), v.len())).collect();
        parts.sort();
        let width = (cluster.high[0] - cluster.low[0]).max(cluster.high[2] - cluster.low[2]);
        let entry = kinds.entry(parts.join(" + ")).or_insert((0, 0.0));
        entry.0 += 1;
        entry.1 = entry.1.max(width);
    }
    println!("{} objects, {} different compositions", editor.clusters.len(), kinds.len());
    let mut list: Vec<_> = kinds.into_iter().collect();
    list.sort_by_key(|(_, (count, _))| std::cmp::Reverse(*count));
    for (composition, (count, width)) in list.iter().take(25) {
        println!("{count:4} x (widest {width:3.0}) {composition}");
    }
    Ok(())
}
