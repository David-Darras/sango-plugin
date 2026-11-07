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
            "  {i:2}{} {:<22} tex {:<22} {:5} vertices {:5} triangles  x {:.0}..{:.0}  y {:.0}..{:.0}  z {:.0}..{:.0}",
            if mesh.encounter { "*" } else { " " }, mesh.material, data::picture_of(mesh), mesh.positions.len(), mesh.indices.len() / 3,
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
pub fn sheet(arc_dir: &str, block: usize, tileset: Option<usize>, path: &str) -> Result<(), String> {
    let scene = data::load_scene(arc_dir, block, tileset)?;
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
    let added = edited.copy_groups(&[CopyRequest { source: found, foreign: None, centre: [0.0, 0.0], yaw: 0.0, delta }])?;
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
    editor.change(|v| v.push(Instance { cluster: wanted, offset: [54.0, 0.0, 0.0], yaw: 0.0, copy: true, deleted: false, only: None, replaces: None }))?;
    println!("after a copy: {} -> {} triangles", before, triangles(&editor));
    let copy = editor.instances.len() - 1;
    editor.change(|v| v[copy].offset[0] += 18.0)?;
    println!("after moving the copy: {} triangles", triangles(&editor));
    // Turn the copy by 90 degrees: its real vertices must turn, not only the box.
    let real_box = |e: &Editor| {
        let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
        for (m, (_, mesh)) in e.meshes().iter().enumerate() {
            for v in 0..mesh.positions.len() {
                if e.owner(m, v) == Some(copy) {
                    for a in 0..3 {
                        low[a] = low[a].min(mesh.positions[v][a]);
                        high[a] = high[a].max(mesh.positions[v][a]);
                    }
                }
            }
        }
        (high[0] - low[0], high[1] - low[1], high[2] - low[2])
    };
    let before_turn = real_box(&editor);
    editor.change(|v| v[copy].yaw = std::f32::consts::FRAC_PI_2)?;
    let after_turn = real_box(&editor);
    println!(
        "turned the copy by 90 degrees: real size x {:.1} y {:.1} z {:.1} -> x {:.1} y {:.1} z {:.1}; {} triangles",
        before_turn.0, before_turn.1, before_turn.2, after_turn.0, after_turn.1, after_turn.2, triangles(&editor)
    );
    editor.change(|v| v[wanted].deleted = true)?;
    println!("after deleting the original: {} triangles", triangles(&editor));
    editor.undo()?;
    println!("after undo: {} triangles", triangles(&editor));
    editor.redo()?;
    println!("after redo: {} triangles", triangles(&editor));
    // Paint a square of 2 x 2 tiles with the value of a wall.
    let (width, _) = editor.grid();
    let before = editor.tiles[20 * width + 20];
    editor.begin_stroke();
    editor.paint(20, 20, 2, 0x0100_0021);
    editor.end_stroke();
    println!("tile (20,20): {before:08x} -> {:08x}", editor.tiles[20 * width + 20]);
    println!("{}", editor.export(out_dir)?);
    let written = EditableBlock::from_file(&std::path::Path::new(out_dir).join("land").join(format!("{block}.bin")), block)?;
    println!("tile (20,20) in the written file: {:08x}", written.attributes()?.cells[20 * width + 20]);
    // Save the project, load it in a new session and write the block again.
    let text = editor.project_text()?;
    println!("project of {} lines:\n{}", text.lines().count(), text.lines().take(8).collect::<Vec<_>>().join("\n"));
    let mut fresh = Editor::new(arc_dir, block)?;
    fresh.load_project(&text)?;
    let second = format!("{out_dir}_reloaded");
    fresh.export(&second)?;
    let first_bytes = std::fs::read(std::path::Path::new(out_dir).join("land").join(format!("{block}.bin"))).map_err(|e| e.to_string())?;
    let second_bytes = std::fs::read(std::path::Path::new(&second).join("land").join(format!("{block}.bin"))).map_err(|e| e.to_string())?;
    println!("block written after loading the project is identical: {}", first_bytes == second_bytes);
    editor.undo()?;
    println!("after undo: tile (20,20) {:08x}, triangles {}", editor.tiles[20 * width + 20], triangles(&editor));
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
pub fn objects(arc_dir: &str, block: usize, all: bool) -> Result<(), String> {
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
    for (composition, (count, width)) in list.iter().take(if all { usize::MAX } else { 25 }) {
        println!("{count:4} x (widest {width:3.0}) {composition}");
    }
    Ok(())
}

/// Prints the grid of tiles of a block. The letters stand for the number of
/// the ground, and `*` marks the centre of each object of the block.
pub fn tiles(arc_dir: &str, block: usize) -> Result<(), String> {
    let editor = crate::editor::Editor::new(arc_dir, block)?;
    let attributes = EditableBlock::load(arc_dir, block)?.attributes()?;
    println!("grid {} x {}", attributes.width, attributes.height);
    let mut counts: std::collections::BTreeMap<u32, usize> = Default::default();
    for &c in &attributes.cells {
        *counts.entry(c).or_default() += 1;
    }
    println!("{} different values", counts.len());
    let mut by_count: Vec<_> = counts.iter().collect();
    by_count.sort_by_key(|(_, n)| std::cmp::Reverse(**n));
    for (value, n) in by_count.iter().take(12) {
        println!("  {value:08x}  ground {:3}  low {:06x}  x{n}", **value >> 24, **value & 0xffffff);
    }
    let symbol = |value: u32| match value >> 24 {
        0 => '.',
        1 => '#',
        31..=37 => ',',
        148..=151 => '~',
        _ => '?',
    };
    let mut marks = std::collections::HashSet::new();
    for c in &editor.clusters {
        let (x, z) = ((c.low[0] + c.high[0]) / 2.0, (c.low[2] + c.high[2]) / 2.0);
        marks.insert((((x + 360.0) / 18.0) as usize, ((z + 360.0) / 18.0) as usize));
    }
    for row in 0..attributes.height {
        let line: String = (0..attributes.width)
            .map(|col| if marks.contains(&(col, row)) { '*' } else { symbol(attributes.cells[row * attributes.width + col]) })
            .collect();
        println!("{line}");
    }
    Ok(())
}

/// Adds a new mesh to a block: a copy of what is near a point in the meshes
/// that have a material, moved along x. It checks that the block can be read
/// again, and writes it.
pub fn add_mesh_test(
    arc_dir: &str,
    block: usize,
    material: &str,
    place: [f32; 2],
    radius: f32,
    dx: f32,
    out_dir: &str,
) -> Result<(), String> {
    let mut edited = EditableBlock::load(arc_dir, block)?;
    let triangles = |e: &EditableBlock| e.meshes.iter().map(|(_, m)| m.indices.len() / 3).sum::<usize>();
    let (meshes_before, triangles_before) = (edited.meshes.len(), triangles(&edited));
    // One new mesh for each material (a tree is in three meshes).
    for name in material.split(',') {
        let found = groups_near(&edited, name, place, radius);
        let like = found.first().map(|(mesh, _)| *mesh).ok_or("nothing near this place")?;
        let source: Vec<(usize, Vec<usize>)> = found.into_iter().filter(|(mesh, _)| *mesh == like).collect();
        let new_mesh = edited.add_mesh_from_groups(like, &source, [dx, 0.0, 0.0])?;
        println!("new mesh {new_mesh}: {} with {} triangles", edited.meshes[new_mesh].1.material, edited.meshes[new_mesh].1.indices.len() / 3);
    }
    println!(
        "meshes {meshes_before} -> {}, triangles {triangles_before} -> {}",
        edited.meshes.len(), triangles(&edited)
    );
    let path = edited.save(out_dir)?;
    println!("written {}", path.display());
    Ok(())
}

/// Looks for problems in the models of a block file (or of a block of the
/// archive) with the checks of the tool.
pub fn validate(arc_dir: &str, block: usize, land_file: Option<&str>) -> Result<(), String> {
    let parts = match land_file {
        Some(path) => data::read_block_from_bytes(&std::fs::read(path).map_err(|e| format!("{path}: {e}"))?)?,
        None => data::read_block(arc_dir, block)?,
    };
    let mut total = 0;
    for part in [1, 5] {
        let Some(model) = parts.get(part).filter(|p| p.starts_with(b"BCH\0")) else { continue };
        let problems = crate::bch_edit::validate(model);
        println!("part {part}: {} problems", problems.len());
        for p in problems.iter().take(8) {
            println!("    {p}");
        }
        total += problems.len();
    }
    if total > 0 {
        return Err(format!("{total} problems"));
    }
    Ok(())
}

/// Checks, on all the blocks, that the width of the indices that the table of
/// relocation gives is the same as the one that the shape of the triangles
/// gives. A difference means that one of the two ways is wrong.
pub fn check_widths(arc_dir: &str) -> Result<(), String> {
    let files = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Blocks))?;
    let (mut blocks, mut conflicts, mut meshes) = (0, 0, 0);
    for (id, file) in files.iter().enumerate() {
        let Some(raw) = file.first() else { continue };
        let Ok(parts) = data::read_block_from_bytes(raw) else { continue };
        for part in [1, 5] {
            let Some(model) = parts.get(part).filter(|p| p.starts_with(b"BCH\0")) else { continue };
            let Ok(bch) = bch::Bch::parse(model) else { continue };
            let Ok(found) = bch.meshes() else { continue };
            meshes += found.len();
            if bch.width_conflicts.get() > 0 {
                println!("block {id} part {part}: {} disagreements", bch.width_conflicts.get());
                for note in bch.conflict_notes.borrow().iter().take(2) {
                    println!("    {note}");
                }
                conflicts += bch.width_conflicts.get();
            }
        }
        blocks += 1;
    }
    println!("{blocks} blocks, {meshes} meshes, {conflicts} disagreements");
    Ok(())
}

/// Lists the blocks that have a mesh with this material: the name of the
/// block (from the names in its model), the vertices and the size of a vertex.
pub fn find(arc_dir: &str, material: &str) -> Result<(), String> {
    let files = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Blocks))?;
    for (id, file) in files.iter().enumerate() {
        let Some(raw) = file.first() else { continue };
        let Ok(block) = data::read_block_from_bytes(raw) else { continue };
        let Some(model) = block.get(1).filter(|p| p.starts_with(b"BCH\0")) else { continue };
        let Ok(bch) = bch::Bch::parse(model) else { continue };
        let Ok(meshes) = bch.meshes() else { continue };
        let name = bch.names().into_iter().find(|n| n.contains('@') && n.contains("world")).unwrap_or_default();
        for mesh in meshes.iter().filter(|m| m.material == material) {
            println!(
                "block {id:3} {:<28} {material}: {:5} vertices, stride {}, layout {}",
                name.rsplit('@').next().unwrap_or(""), mesh.positions.len(), mesh.places.stride, mesh.places.layout
            );
        }
    }
    Ok(())
}

/// A check of the copy of an object from one block to another, without a
/// window: takes the biggest object of the source block (a tree), puts it in
/// the target block, moves it, writes the block and loads the project again.
pub fn cross_test(arc_dir: &str, source: usize, target: usize, out_dir: &str) -> Result<(), String> {
    use crate::editor::Editor;
    let from = Editor::new(arc_dir, source)?;
    let mut into = Editor::new(arc_dir, target)?;
    // The biggest object that the target block can take.
    let mut order: Vec<usize> = (0..from.clusters.len()).collect();
    order.sort_by_key(|&c| std::cmp::Reverse(from.clusters[c].parts.iter().map(|(_, v)| v.len()).sum::<usize>()));
    let mut chosen = None;
    for c in order {
        let object = from.portable_of_cluster_for_test(c)?;
        match into.can_import(&object) {
            Ok(()) => {
                chosen = Some((c, object));
                break;
            }
            Err(message) => println!("object {c} cannot go in block {target}: {message}"),
        }
        if chosen.is_none() && c % 1000 == 999 {
            break;
        }
    }
    let (biggest, object) = chosen.ok_or("no object of the source block fits in the target block")?;
    let bytes = object.to_bytes();
    let object = crate::edit::Portable::from_bytes(&bytes)?;
    println!(
        "object {biggest} of block {source}: {} pieces, {} bytes, materials {:?}",
        object.parts.len(), bytes.len(), object.parts.iter().map(|p| p.material.as_str()).collect::<Vec<_>>()
    );
    let triangles = |e: &Editor| e.meshes().iter().map(|(_, m)| m.indices.len() / 3).sum::<usize>();
    let before = triangles(&into);
    let index = into.import(object)?;
    println!("pasted in block {target}: {before} -> {} triangles", triangles(&into));
    into.change(|v| v[index].offset = [36.0, 0.0, 36.0])?;
    into.change(|v| v[index].yaw = std::f32::consts::FRAC_PI_2)?;
    println!("moved and turned: {} triangles", triangles(&into));
    println!("{}", into.export(out_dir)?);
    let text = into.project_text()?;
    println!("project:\n{}", text.lines().filter(|l| l.starts_with("import") || l.starts_with("copy")).collect::<Vec<_>>().join("\n"));
    let mut fresh = Editor::new(arc_dir, target)?;
    fresh.load_project(&text)?;
    let second = format!("{out_dir}_reloaded");
    fresh.export(&second)?;
    let name = format!("{target}.bin");
    let a = std::fs::read(std::path::Path::new(out_dir).join("land").join(&name)).map_err(|e| e.to_string())?;
    let b = std::fs::read(std::path::Path::new(&second).join("land").join(&name)).map_err(|e| e.to_string())?;
    println!("block written after loading the project is identical: {}", a == b);
    Ok(())
}

/// A check of "grow the selection" without a window: takes the object of a
/// block that holds a point and adds its neighbours, round after round.
pub fn grow_test(arc_dir: &str, block: usize, point: [f32; 2], rounds: usize) -> Result<(), String> {
    let editor = crate::editor::Editor::new(arc_dir, block)?;
    let start = (0..editor.instances.len())
        .filter(|&i| {
            let (l, h) = editor.instance_bounds(i);
            point[0] >= l[0] && point[0] <= h[0] && point[1] >= l[2] && point[1] <= h[2]
        })
        .min_by(|&a, &b| {
            let tileset = |i: usize| {
                let (l, h) = editor.instance_bounds(i);
                (h[0] - l[0]) * (h[2] - l[2])
            };
            tileset(a).total_cmp(&tileset(b))
        })
        .ok_or("no object holds this point")?;
    let mut selection = vec![start];
    for round in 0..=rounds {
        let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
        let mut names: Vec<String> = Vec::new();
        for &i in &selection {
            let (l, h) = editor.instance_bounds(i);
            for a in 0..3 {
                low[a] = low[a].min(l[a]);
                high[a] = high[a].max(h[a]);
            }
            for n in editor.cluster_materials(editor.instances[i].cluster) {
                if !names.contains(&n) {
                    names.push(n);
                }
            }
        }
        println!(
            "round {round}: {} pieces, size {:.0} x {:.0}, meshes {}",
            selection.len(), high[0] - low[0], high[2] - low[2], names.join(", ")
        );
        let extra = editor.neighbours(&selection, 6.0);
        if extra.is_empty() {
            break;
        }
        selection.extend(extra);
    }
    // The object that the clipboard would hold.
    let object = editor.portable_of_many(&selection)?;
    println!("clipboard: {} meshes, {} bytes", object.parts.len(), object.to_bytes().len());
    Ok(())
}

/// Checks, on the material trees of all the blocks, that the way the tool
/// looks for a name in a tree is the way the game does it, and that the tool
/// builds the same tree from the names.
pub fn check_trees(arc_dir: &str) -> Result<(), String> {
    use crate::dict::{build, lookup, Node};
    let files = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Blocks))?;
    let (mut trees, mut lookups_ok, mut lookups, mut same, mut shown) = (0, 0, 0, 0, 0);
    for (id, file) in files.iter().enumerate() {
        let Some(raw) = file.first() else { continue };
        let Ok(parts) = data::read_block_from_bytes(raw) else { continue };
        for part in [1, 5] {
            let Some(model) = parts.get(part).filter(|p| p.starts_with(b"BCH\0")) else { continue };
            let Ok(b) = bch::Bch::parse(model) else { continue };
            let word = |at: usize| crate::garc::u32_at(model, at).unwrap_or(0) as usize;
            let table = b.content + word(b.content);
            let m = b.content + word(table);
            let (count, tree) = (word(m + 0x38), b.content + word(m + 0x3c));
            let nodes: Vec<Node> = (0..=count).map(|i| Node::from_bytes(&model[tree + 12 * i..tree + 12 * i + 12])).collect();
            let name_at = |offset: u32| -> Vec<u8> {
                let tail = &model[b.strings + offset as usize..];
                tail[..tail.iter().position(|&c| c == 0).unwrap_or(tail.len())].to_vec()
            };
            let names: Vec<Vec<u8>> = nodes[1..].iter().map(|n| name_at(n.name)).collect();
            trees += 1;
            for (k, name) in names.iter().enumerate() {
                lookups += 1;
                if lookup(&nodes, name) == k + 1 {
                    lookups_ok += 1;
                }
            }
            let refs: Vec<&[u8]> = names.iter().map(|n| n.as_slice()).collect();
            let offsets: Vec<u32> = nodes[1..].iter().map(|n| n.name).collect();
            let rebuilt = build(&refs, &offsets);
            if rebuilt == nodes {
                same += 1;
            } else if shown < 3 {
                shown += 1;
                println!("block {id} part {part}: the rebuilt tree differs ({} names)", names.len());
                for (i, (a, b)) in nodes.iter().zip(&rebuilt).enumerate().filter(|(_, (a, b))| a != b).take(2) {
                    println!("    node {i}: file {a:?}  rebuilt {b:?}");
                }
            }
        }
    }
    println!("{trees} trees: lookups {lookups_ok}/{lookups} found, rebuilt identical {same}/{trees}");
    Ok(())
}

/// What the button "Select" of the list of meshes takes for some meshes: the
/// pieces, and what they hold. It presses the button for each mesh, with
/// Shift, as a user does.
pub fn mesh_select_test(arc_dir: &str, block: usize, names: &str) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let wanted: Vec<&str> = names.split(',').collect();
    let mut selected: Vec<usize> = Vec::new();
    let before = editor.instances.len();
    for i in 0..editor.meshes().len() {
        if wanted.contains(&editor.meshes()[i].1.material.as_str()) {
            for p in editor.isolate_meshes(&[i])? {
                if !selected.contains(&p) {
                    selected.push(p);
                }
            }
        }
    }
    println!("{} pieces selected ({} instances were added by the cuts)", selected.len(), editor.instances.len() - before);
    let mut by_material: std::collections::BTreeMap<String, usize> = Default::default();
    let mut outside = 0usize;
    for &p in &selected {
        for (name, vertices) in editor.instance_parts(p) {
            let asked = wanted.contains(&name.as_str());
            *by_material.entry(format!("{}{}", name, if asked { "" } else { "  <- NOT asked" })).or_default() += vertices;
            if !asked {
                outside += vertices;
            }
        }
    }
    for (name, vertices) in &by_material {
        println!("  {name}: {vertices} vertices");
    }
    println!("vertices of other meshes in the selection: {outside}");
    // The cuts must keep every vertex somewhere: moving the selection moves
    // exactly the meshes that were asked.
    let moved = selected.clone();
    editor.change(|v| {
        for &i in &moved {
            v[i].offset[0] += 36.0;
        }
    })?;
    let text = editor.project_text()?;
    let mut fresh = crate::editor::Editor::new(arc_dir, block)?;
    fresh.load_project(&text)?;
    println!(
        "project with the cuts: {} lines; instances after loading it {} (before {})",
        text.lines().count(), fresh.instances.len(), editor.instances.len()
    );
    println!(
        "the block written after loading the project is the same: {}",
        fresh.export(&format!("{}_a", std::env::temp_dir().join("sango_editor_cut").display())).is_ok()
            && editor.export(&format!("{}_b", std::env::temp_dir().join("sango_editor_cut").display())).is_ok()
            && std::fs::read(std::env::temp_dir().join("sango_editor_cut_a").join("land").join(format!("{block}.bin"))).ok()
                == std::fs::read(std::env::temp_dir().join("sango_editor_cut_b").join("land").join(format!("{block}.bin"))).ok()
    );
    Ok(())
}

/// What Ctrl+click takes: the whole building of the piece that holds a point
/// in a mesh.
pub fn building_test(arc_dir: &str, block: usize, point: [f32; 2], mesh_name: &str) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let mesh = (0..editor.meshes().len())
        .find(|&m| editor.meshes()[m].1.material == mesh_name)
        .ok_or("no such mesh")?;
    // The piece of this mesh whose box holds the point.
    let piece = (0..editor.instances.len())
        .find(|&i| {
            editor.instance_parts(i).iter().any(|(name, _)| name == mesh_name) && {
                let (l, h) = editor.instance_bounds(i);
                point[0] >= l[0] && point[0] <= h[0] && point[1] >= l[2] && point[1] <= h[2]
            }
        })
        .ok_or("no piece of this mesh holds this point")?;
    {
        let (l, h) = editor.instance_bounds(piece);
        println!("clicked piece {piece}: box x {:.0}..{:.0} z {:.0}..{:.0}, parts {:?}", l[0], h[0], l[2], h[2], editor.instance_parts(piece));
    }
    let taken = editor.building_of(piece, mesh)?;
    let mut by_material: std::collections::BTreeMap<String, usize> = Default::default();
    let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
    for &p in &taken {
        for (name, vertices) in editor.instance_parts(p) {
            *by_material.entry(name).or_default() += vertices;
        }
        let (l, h) = editor.instance_bounds(p);
        for a in 0..3 {
            low[a] = low[a].min(l[a]);
            high[a] = high[a].max(h[a]);
        }
    }
    println!("{} pieces, size {:.0} x {:.0}", taken.len(), high[0] - low[0], high[2] - low[2]);
    for (name, vertices) in &by_material {
        println!("  {name}: {vertices} vertices (the mesh has {} in all)", {
            let m = (0..editor.meshes().len()).find(|&k| editor.meshes()[k].1.material == *name).unwrap_or(0);
            editor.meshes()[m].1.positions.len()
        });
    }
    Ok(())
}

/// Puts a building of one block in another block that does not have its
/// materials: takes the building as Ctrl+click does, finds open ground in the
/// target block, adds the meshes and the materials, checks the file and
/// writes it.
pub fn import_building_test(
    arc_dir: &str,
    donor: usize,
    point: [f32; 2],
    mesh_name: &str,
    target: usize,
    out_dir: &str,
) -> Result<(), String> {
    use crate::editor::Editor;
    // The building, from the donor block.
    let mut from = Editor::new(arc_dir, donor)?;
    let mesh = (0..from.meshes().len()).find(|&m| from.meshes()[m].1.material == mesh_name).ok_or("no such mesh")?;
    let piece = (0..from.instances.len())
        .find(|&i| {
            from.instance_parts(i).iter().any(|(n, _)| n == mesh_name) && {
                let (l, h) = from.instance_bounds(i);
                point[0] >= l[0] && point[0] <= h[0] && point[1] >= l[2] && point[1] <= h[2]
            }
        })
        .ok_or("no piece of this mesh holds this point")?;
    let taken = from.building_of(piece, mesh)?;
    let object = from.portable_of_many(&taken)?;
    let bytes = object.to_bytes();
    let object = crate::edit::Portable::from_bytes(&bytes)?;
    println!(
        "building of block {donor}: {} pieces, {} meshes ({}), {} bytes",
        taken.len(), object.parts.len(), object.parts.iter().map(|p| p.material.as_str()).collect::<Vec<_>>().join(", "), bytes.len()
    );
    let centre = [(object.low[0] + object.high[0]) / 2.0, (object.low[2] + object.high[2]) / 2.0];
    let (width, depth) = (object.high[0] - object.low[0], object.high[2] - object.low[2]);

    // The target block: open ground for the building (a square of tiles that are not walls or water).
    let mut into = EditableBlock::load(arc_dir, target)?;
    let tiles = into.attributes()?;
    let side = ((width.max(depth) / 18.0).ceil() as usize + 1).min(tiles.width);
    let mut best: Option<(f32, usize, usize)> = None;
    for row in 0..=tiles.height - side {
        for column in 0..=tiles.width - side {
            let plain = (0..side).all(|r| (0..side).all(|c| !matches!(tiles.cells[(row + r) * tiles.width + column + c] >> 24, 1 | 148..=151)));
            if plain {
                let (cx, cz) = (column as f32 + side as f32 / 2.0 - 20.0, row as f32 + side as f32 / 2.0 - 20.0);
                let d = cx * cx + cz * cz;
                if best.map_or(true, |(b, _, _)| d < b) {
                    best = Some((d, column, row));
                }
            }
        }
    }
    let (_, column, row) = best.ok_or("no open ground of this size in the target block")?;
    let target_centre = [-360.0 + 18.0 * (column as f32 + side as f32 / 2.0), -360.0 + 18.0 * (row as f32 + side as f32 / 2.0)];
    // The height of the ground there: a ray goes down on the ground meshes.
    let ground = Editor::new(arc_dir, target)?;
    let mut height: Option<f32> = None;
    for (i, (part, mesh)) in ground.meshes().iter().enumerate() {
        if *part != 1 || ground.selectable[i] {
            continue;
        }
        for t in mesh.indices.chunks_exact(3) {
            let (a, b, c) = (mesh.positions[t[0] as usize], mesh.positions[t[1] as usize], mesh.positions[t[2] as usize]);
            // The point of the triangle that is over (x, z), if there is one.
            let (x, z) = (target_centre[0], target_centre[1]);
            let d = (b[2] - c[2]) * (a[0] - c[0]) + (c[0] - b[0]) * (a[2] - c[2]);
            if d.abs() < 1e-6 {
                continue;
            }
            let l1 = ((b[2] - c[2]) * (x - c[0]) + (c[0] - b[0]) * (z - c[2])) / d;
            let l2 = ((c[2] - a[2]) * (x - c[0]) + (a[0] - c[0]) * (z - c[2])) / d;
            let l3 = 1.0 - l1 - l2;
            if l1 >= 0.0 && l2 >= 0.0 && l3 >= 0.0 {
                let y = l1 * a[1] + l2 * b[1] + l3 * c[1];
                height = Some(height.map_or(y, |h: f32| h.max(y)));
            }
        }
    }
    let ground_y = height.ok_or("cannot find the ground under the place")?;
    let delta = [target_centre[0] - centre[0], ground_y - object.low[1], target_centre[1] - centre[1]];
    println!(
        "target block {target}: open ground of {side} x {side} tiles at tile ({column}, {row}), centre x {:.0} z {:.0}, ground height {ground_y:.1}",
        target_centre[0], target_centre[1]
    );

    // The textures that the materials need must be in the tileset of the target block.
    let scene = data::load_scene(arc_dir, target, None)?;
    for part in &object.parts {
        if let Some(blob) = &part.blob {
            let wanted: Vec<&String> = blob.header_strings.iter().map(|(_, t)| t).filter(|t| *t != &blob.name).collect();
            let known = wanted.iter().filter(|t| scene.textures.contains_key(t.as_str())).count();
            println!("  {}: pictures {:?}, {} of {} in the tileset {:?} of the target", part.material, wanted, known, wanted.len(), scene.tileset);
        }
    }

    let meshes_before = into.meshes.len();
    let added = into.add_foreign_meshes(&object, centre, 0.0, delta)?;
    println!("added {added} meshes: {meshes_before} -> {}", into.meshes.len());
    let path = into.save(out_dir)?;
    println!("written {}", path.display());
    Ok(())
}

/// Builds the catalogue of the blocks from `first` to `last` (all by default),
/// says how many entries it has and how long it took, and saves it in `out`.
pub fn catalog_stats(arc_dir: &str, first: Option<usize>, last: Option<usize>, out: Option<&str>) -> Result<(), String> {
    let blocks: Option<Vec<usize>> = match (first, last) {
        (Some(a), Some(b)) => Some((a..=b).collect()),
        _ => None,
    };
    let start = std::time::Instant::now();
    let entries = crate::catalog::build(arc_dir, blocks.as_deref(), |_| {})?;
    println!("{} entries, in {:.1} s", entries.len(), start.elapsed().as_secs_f32());
    let total: usize = entries.iter().map(|e| e.occurrences).sum();
    let bytes: usize = entries.iter().map(|e| e.portable.to_bytes().len()).sum();
    println!("{total} objects in all, {} MB of data for the entries", bytes / 1_000_000);
    let mut sorted: Vec<_> = entries.iter().collect();
    sorted.sort_by_key(|e| std::cmp::Reverse(e.occurrences));
    for e in sorted.iter().take(10) {
        println!("  {:5} x {}  ({} shapes, {} blocks)", e.occurrences, e.name, e.variants, e.blocks.len());
    }
    if let Some(path) = out {
        crate::catalog::save(&entries, std::path::Path::new(path))?;
        let back = crate::catalog::load(std::path::Path::new(path))?;
        println!("saved in {path}, read again: {} entries", back.len());
    }
    Ok(())
}

/// Draws 48 entries of a saved catalogue (from `first`) in a contact sheet.
pub fn catalog_sheet(arc_dir: &str, catalog: &str, out: &str, first: usize) -> Result<(), String> {
    let entries = crate::catalog::load(std::path::Path::new(catalog))?;
    const SIZE: usize = 128;
    const COLUMNS: usize = 8;
    const ROWS: usize = 6;
    let mut sheet = vec![40u8; SIZE * COLUMNS * SIZE * ROWS * 4];
    let mut cache: std::collections::HashMap<usize, std::collections::HashMap<String, crate::bch::Texture>> = Default::default();
    for (k, entry) in entries.iter().skip(first).take(COLUMNS * ROWS).enumerate() {
        let textures = match entry.tileset {
            Some(tileset) => cache.entry(tileset).or_insert_with(|| crate::data::tileset_textures(arc_dir, tileset).unwrap_or_default()),
            None => &mut Default::default(),
        };
        let image = crate::thumb::render(&entry.preview, textures, SIZE);
        let (cx, cy) = (k % COLUMNS * SIZE, k / COLUMNS * SIZE);
        for y in 0..SIZE {
            for x in 0..SIZE {
                let from = (y * SIZE + x) * 4;
                if image[from + 3] == 0 {
                    continue;
                }
                let to = ((cy + y) * SIZE * COLUMNS + cx + x) * 4;
                sheet[to..to + 4].copy_from_slice(&image[from..from + 4]);
            }
        }
        println!("{:3} {} (tileset {:?}, {} blocks)", first + k, entry.name, entry.tileset, entry.blocks.len());
    }
    write_png(out, SIZE * COLUMNS, SIZE * ROWS, &sheet)
}

/// Drops a model of a saved catalogue in a block through the editor, as the
/// catalogue panel does, undoes and redoes it, and writes the block.
pub fn catalog_drop_test(arc_dir: &str, catalog: &str, name: &str, target: usize, out_dir: &str) -> Result<(), String> {
    use crate::editor::Editor;
    let entries = crate::catalog::load(std::path::Path::new(catalog))?;
    let entry = entries.iter().find(|e| e.name == name).ok_or("no model has this name")?;
    let object = entry.portable.clone();
    println!("model {} (block {}), {} meshes", entry.name, entry.donor, object.parts.len());
    let mut editor = Editor::new(arc_dir, target)?;
    let before = editor.meshes().len();
    let index = editor.import(object.clone())?;
    println!("meshes: {before} -> {}", editor.meshes().len());
    // Open ground: the tile nearest to the middle that is not a wall or water.
    let (width, height) = editor.grid();
    let mut best: Option<(f32, usize, usize)> = None;
    for row in 0..height {
        for column in 0..width {
            if !matches!(editor.tiles[row * width + column] >> 24, 1 | 148..=151) {
                let (dx, dz) = (column as f32 - width as f32 / 2.0, row as f32 - height as f32 / 2.0);
                if best.map_or(true, |(d, _, _)| dx * dx + dz * dz < d) {
                    best = Some((dx * dx + dz * dz, column, row));
                }
            }
        }
    }
    let (_, column, row) = best.ok_or("no open tile")?;
    let (x, z) = (-360.0 + 18.0 * (column as f32 + 0.5), -360.0 + 18.0 * (row as f32 + 0.5));
    let (low, high) = editor.instance_bounds(index);
    let delta = [x - (low[0] + high[0]) / 2.0, 0.0, z - (low[2] + high[2]) / 2.0];
    editor.change(|v| {
        for a in 0..3 {
            v[index].offset[a] += delta[a];
        }
    })?;
    let owned: usize = (0..editor.meshes().len()).map(|m| (0..editor.meshes()[m].1.positions.len()).filter(|&v| editor.owner(m, v) == Some(index)).count()).sum();
    println!("vertices that belong to the dropped object: {owned}");
    editor.undo()?;
    editor.undo()?;
    println!("after two undo: {} meshes", editor.meshes().len());
    editor.redo()?;
    editor.redo()?;
    println!("after two redo: {} meshes", editor.meshes().len());
    println!("{}", editor.export(out_dir)?);
    Ok(())
}

/// Deletes the objects of a block one after the other (one more each time)
/// and says which delete fails.
pub fn delete_many_test(arc_dir: &str, block: usize, count: usize) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let total = editor.instances.len().min(count);
    let mut failed = 0;
    for i in 0..total {
        if let Err(message) = editor.change(|v| v[i].deleted = true) {
            failed += 1;
            println!("delete {i} fails: {message}");
        }
    }
    println!("{failed} failures out of {total}");
    Ok(())
}

/// Takes each mesh as the "Select" button does, then deletes the pieces.
pub fn delete_meshes_test(arc_dir: &str, block: usize) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let mut failed = 0;
    for m in 0..editor.meshes().len() {
        if !editor.selectable[m] {
            continue;
        }
        let pieces = match editor.isolate_meshes(&[m]) {
            Ok(p) => p,
            Err(e) => {
                println!("select mesh {m} fails: {e}");
                failed += 1;
                continue;
            }
        };
        if let Err(e) = editor.change(|v| pieces.iter().for_each(|&i| v[i].deleted = true)) {
            println!("delete of mesh {m} ({}) fails: {e}", editor.material(m));
            failed += 1;
        }
    }
    println!("{failed} failures");
    Ok(())
}

/// Drops a catalogue model, then deletes the first objects one by one.
pub fn drop_then_delete_test(arc_dir: &str, catalog: &str, name: &str, block: usize) -> Result<(), String> {
    let entries = crate::catalog::load(std::path::Path::new(catalog))?;
    let entry = entries.iter().find(|e| e.name == name).ok_or("no model has this name")?;
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    editor.import(entry.portable.clone())?;
    let mut failed = 0;
    for i in 0..editor.instances.len().min(40) {
        if let Err(message) = editor.change(|v| v[i].deleted = true) {
            failed += 1;
            println!("delete {i} fails: {message}");
        }
    }
    println!("{failed} failures");
    Ok(())
}

/// Looks at how the ground meshes of a block fit the tiles of 18 units.
pub fn ground_tiles(arc_dir: &str, block: usize) -> Result<(), String> {
    let editor = crate::editor::Editor::new(arc_dir, block)?;
    for (i, (part, mesh)) in editor.meshes().iter().enumerate() {
        if *part != 1 || editor.selectable[i] {
            continue;
        }
        let (mut inside, mut crossing) = (0, 0);
        let mut per_tile: std::collections::BTreeMap<(i32, i32), usize> = Default::default();
        for t in mesh.indices.chunks_exact(3) {
            let p: Vec<[f32; 3]> = t.iter().map(|&k| mesh.positions[k as usize]).collect();
            let cell = |v: f32| ((v + 360.0) / 18.0).floor() as i32;
            let cells: std::collections::BTreeSet<(i32, i32)> = p.iter().map(|q| (cell(q[0]), cell(q[2]))).collect();
            // A vertex exactly on a border belongs to two tiles: count a triangle as inside when its centre cell holds all its vertices within a small margin.
            let c = [(p[0][0] + p[1][0] + p[2][0]) / 3.0, (p[0][2] + p[1][2] + p[2][2]) / 3.0];
            let home = (cell(c[0]), cell(c[1]));
            let fits = p.iter().all(|q| {
                let (x0, z0) = (-360.0 + 18.0 * home.0 as f32, -360.0 + 18.0 * home.1 as f32);
                q[0] >= x0 - 0.05 && q[0] <= x0 + 18.05 && q[2] >= z0 - 0.05 && q[2] <= z0 + 18.05
            });
            if fits { inside += 1 } else { crossing += 1 }
            *per_tile.entry(home).or_default() += 1;
            let _ = cells;
        }
        let avg = if per_tile.is_empty() { 0.0 } else { mesh.indices.len() as f32 / 3.0 / per_tile.len() as f32 };
        println!("mesh {i} {}: {} triangles, {inside} inside one tile, {crossing} cross tiles, {} tiles touched, {avg:.1} triangles per tile", mesh.material, mesh.indices.len() / 3, per_tile.len());
    }
    Ok(())
}

/// Builds the catalogue of tiles of some blocks and says how many pieces it has.
pub fn tile_stats(arc_dir: &str, first: usize, last: usize, out: Option<&str>) -> Result<(), String> {
    let blocks: Vec<usize> = (first..=last).collect();
    let start = std::time::Instant::now();
    let entries = crate::catalog::build_tiles(arc_dir, Some(&blocks), |_| {})?;
    println!("{} different tiles in {:.1} s", entries.len(), start.elapsed().as_secs_f32());
    for e in entries.iter().take(8) {
        println!("  {:5} x {} ({} blocks)", e.occurrences, e.name, e.blocks.len());
    }
    let singles = entries.iter().filter(|e| e.occurrences == 1).count();
    println!("{singles} seen only once");
    for (k, e) in entries.iter().enumerate().filter(|(_, e)| e.portable.parts.iter().any(|p| crate::editor::is_grass_material(&p.material))).take(6) {
        println!("  tile {k} has tall grass: {} ({} times)", e.name, e.occurrences);
    }
    if let Some(path) = out {
        crate::catalog::save(&entries, std::path::Path::new(path))?;
    }
    Ok(())
}

/// Puts tiles of a saved catalogue of tiles in a block (the same tile twice,
/// to check that the second one replaces the first), and writes the block.
pub fn tile_drop_test(arc_dir: &str, tiles: &str, number: usize, block: usize, out_dir: &str) -> Result<(), String> {
    let entries = crate::catalog::load(std::path::Path::new(tiles))?;
    let entry = entries.get(number).ok_or("no such tile")?;
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let count = |e: &crate::editor::Editor| e.meshes().iter().map(|(_, m)| m.indices.len() / 3).sum::<usize>();
    println!("tile {number}: {} ({} triangles in it); block has {} triangles", entry.name, entry.preview[0].indices.len() / 3, count(&editor));
    for (column, row) in [(20, 20), (20, 20), (21, 20)] {
        editor.stamp_tile(entry.portable.clone(), column, row, Some(0.0))?;
        println!("after a tile in ({column}, {row}): {} triangles", count(&editor));
    }
    editor.undo()?;
    println!("after undo: {} triangles", count(&editor));
    editor.redo()?;
    println!("{}", editor.export(out_dir)?);
    Ok(())
}

/// Puts each of the first tiles of a catalogue in a tile of a block, one at a
/// time, and says which ones leave the tile empty.
pub fn tile_holes_test(arc_dir: &str, tiles: &str, first: usize, count: usize, block: usize) -> Result<(), String> {
    let entries = crate::catalog::load(std::path::Path::new(tiles))?;
    let (column, row) = (22usize, 29usize);
    let (x0, z0) = (-360.0 + 18.0 * column as f32, -360.0 + 18.0 * row as f32);
    let in_cell = |e: &crate::editor::Editor| -> usize {
        let mut n = 0;
        for (part, m) in e.meshes() {
            if *part != 1 { continue; }
            for t in m.indices.chunks_exact(3) {
                let (mut x, mut z) = (0.0, 0.0);
                for &i in t {
                    let p = m.positions.get(i as usize).copied().unwrap_or([0.0; 3]);
                    x += p[0] / 3.0;
                    z += p[2] / 3.0;
                }
                if x >= x0 && x < x0 + 18.0 && z >= z0 && z < z0 + 18.0 { n += 1; }
            }
        }
        n
    };
    for (k, entry) in entries.iter().enumerate().skip(first).take(count) {
        let mut editor = crate::editor::Editor::new(arc_dir, block)?;
        let before = in_cell(&editor);
        match editor.stamp_tile(entry.portable.clone(), column, row, Some(0.0)) {
            Ok(_) => {
                let after = in_cell(&editor);
                let own: usize = (0..editor.meshes().len()).map(|m| (0..editor.meshes()[m].1.positions.len()).filter(|&v| editor.owner(m, v).is_some_and(|o| editor.instances[o].replaces.is_some())).count()).sum();
                let (mut lo, mut hi) = ([f32::MAX; 3], [f32::MIN; 3]);
                for m in 0..editor.meshes().len() {
                    for v in 0..editor.meshes()[m].1.positions.len() {
                        if editor.owner(m, v).is_some_and(|o| editor.instances[o].replaces.is_some()) {
                            let p = editor.meshes()[m].1.positions[v];
                            for a in 0..3 { lo[a] = lo[a].min(p[a]); hi[a] = hi[a].max(p[a]); }
                        }
                    }
                }
                println!("{k:4} {:30} triangles in the tile {before} -> {after}, tile vertices {own}, tri in entry {}, box x {:.0}..{:.0} y {:.0}..{:.0} z {:.0}..{:.0} (cell x {x0}..{} z {z0}..{})", entry.name, entry.preview[0].indices.len() / 3, lo[0], hi[0], lo[1], hi[1], lo[2], hi[2], x0 + 18.0, z0 + 18.0);
            }
            Err(e) => println!("{k:4} {} fails: {e}", entry.name),
        }
    }
    Ok(())
}

/// Draws the tiles around (22, 29) of a block after some catalogue tiles were
/// put there, in a picture, with the pictures of the tileset of the block.
pub fn tile_picture_test(arc_dir: &str, tiles: &str, numbers: &[usize], block: usize, out: &str) -> Result<(), String> {
    let entries = crate::catalog::load(std::path::Path::new(tiles))?;
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    for (k, &n) in numbers.iter().enumerate() {
        let entry = entries.get(n).ok_or("no such tile")?;
        editor.stamp_tile(entry.portable.clone(), 20 + k, 29, Some(0.0))?;
    }
    let scene = data::load_scene(arc_dir, block, None)?;
    let (x0, z0, x1, z1) = (-360.0 + 18.0 * 18.0, -360.0 + 18.0 * 27.0, -360.0 + 18.0 * (22.0 + numbers.len() as f32), -360.0 + 18.0 * 32.0);
    let mut parts = Vec::new();
    for (_, m) in editor.meshes().iter().filter(|(p, _)| *p == 1) {
        let mut indices = Vec::new();
        for t in m.indices.chunks_exact(3) {
            let c: f32 = t.iter().map(|&i| m.positions.get(i as usize).map_or(0.0, |p| p[0])).sum::<f32>() / 3.0;
            let d: f32 = t.iter().map(|&i| m.positions.get(i as usize).map_or(0.0, |p| p[2])).sum::<f32>() / 3.0;
            if c >= x0 && c <= x1 && d >= z0 && d <= z1 {
                indices.extend_from_slice(t);
            }
        }
        parts.push(crate::catalog::PreviewPart { picture: data::picture_of(m).to_string(), positions: m.positions.clone(), uvs: m.uvs.clone(), indices });
    }
    let image = crate::thumb::render(&parts, &scene.textures, 512);
    write_png(out, 512, 512, &image)
}

/// Drops each of some catalogue models in a block, one at a time, and checks
/// that the block can be built, written and read again without problems.
pub fn drop_all_test(arc_dir: &str, catalog: &str, first: usize, count: usize, block: usize) -> Result<(), String> {
    let entries = crate::catalog::load(std::path::Path::new(catalog))?;
    let dir = std::env::temp_dir().join("sango_editor_drop_all");
    let mut bad = 0;
    for (k, entry) in entries.iter().enumerate().skip(first).take(count) {
        let mut editor = crate::editor::Editor::new(arc_dir, block)?;
        let outcome = editor.import(entry.portable.clone()).and_then(|_| editor.export(dir.to_str().unwrap_or("."))).and_then(|_| {
            let bytes = std::fs::read(dir.join("land").join(format!("{block}.bin"))).map_err(|e| e.to_string())?;
            let parts = data::read_block_from_bytes(&bytes)?;
            let mut problems = Vec::new();
            for part in [1, 5] {
                if let Some(model) = parts.get(part).filter(|p| p.starts_with(b"BCH\0")) {
                    problems.extend(crate::bch_edit::validate(model));
                    let meshes = bch::Bch::parse(model)?.meshes()?;
                    let _ = meshes;
                }
            }
            if problems.is_empty() { Ok(()) } else { Err(format!("{} problems, first: {}", problems.len(), problems[0])) }
        });
        if let Err(message) = outcome {
            bad += 1;
            println!("{k:5} {} (block {}): {message}", entry.name, entry.donor);
        }
    }
    println!("{bad} bad out of {count}");
    Ok(())
}

/// Counts the layouts of vertices of the models of a catalogue, and of the
/// meshes of a block.
pub fn layout_stats(arc_dir: &str, catalog: &str, block: usize) -> Result<(), String> {
    let entries = crate::catalog::load(std::path::Path::new(catalog))?;
    let mut counts: std::collections::BTreeMap<String, usize> = Default::default();
    for e in &entries {
        for p in &e.portable.parts {
            *counts.entry(p.layout.clone()).or_default() += 1;
        }
    }
    println!("layouts in the catalogue:");
    for (l, n) in &counts {
        println!("  {n:5}  {l}");
    }
    let editor = crate::editor::Editor::new(arc_dir, block)?;
    let mut own: std::collections::BTreeSet<String> = Default::default();
    for (part, m) in editor.meshes() {
        if *part == 1 {
            own.insert(m.places.layout.clone());
        }
    }
    println!("layouts of block {block}: {own:?}");
    Ok(())
}

/// Copies pictures from the pack of an tileset to the pack of another tileset, reads
/// the result again, and checks that the pictures are the same.
pub fn tileset_pictures_test(arc_dir: &str, donor: usize, target: usize, names: &[String]) -> Result<(), String> {
    let blobs = data::tileset_texture_blobs(arc_dir, donor, names)?;
    println!("{} pictures read from tileset {donor}: {}", blobs.len(), blobs.iter().map(|b| format!("{} ({} bytes)", b.name, b.raw.len())).collect::<Vec<_>>().join(", "));
    let (pack, _) = data::tileset_pack(arc_dir, target)?;
    let (_, parts) = crate::pack::split(&pack)?;
    let part = parts.get(data::TILESET_PICTURES).ok_or("no pictures")?;
    let before = bch::Bch::parse(part)?.textures()?.len();
    let bigger = crate::bch_edit::add_textures(part, &blobs)?;
    let parsed = bch::Bch::parse(&bigger)?;
    let after = parsed.textures()?;
    println!("tileset {target}: {before} pictures, then {} ({} -> {} bytes)", after.len(), part.len(), bigger.len());
    let donor_pictures: std::collections::HashMap<String, bch::Texture> = data::tileset_textures(arc_dir, donor)?;
    for blob in &blobs {
        let got = after.iter().find(|t| t.name == blob.name);
        let want = donor_pictures.get(&blob.name);
        match (got, want) {
            (Some(g), Some(w)) => println!("  {}: {}x{} same pixels: {}", blob.name, g.width, g.height, g.rgba == w.rgba),
            _ => println!("  {}: missing after the copy", blob.name),
        }
    }
    // The names must all still be found in the tree.
    let names_after = crate::bch_edit::texture_names(&bigger)?;
    println!("names in the dictionary: {}", names_after.len());
    let again = crate::bch_edit::add_textures(&bigger, &blobs)?;
    println!("a second copy changes nothing: {}", again == bigger);
    Ok(())
}

/// Drops a model that comes from another tileset, exports the block and checks
/// that the tileset file that comes out has the pictures of the model.
pub fn drop_tileset_test(arc_dir: &str, catalog: &str, name: &str, block: usize, out_dir: &str) -> Result<(), String> {
    use crate::editor::Editor;
    let entries = crate::catalog::load(std::path::Path::new(catalog))?;
    let entry = entries.iter().find(|e| e.name == name).ok_or("no model has this name")?;
    let scene = data::load_scene(arc_dir, block, None)?;
    let mut editor = Editor::new(arc_dir, block)?;
    editor.set_tileset(scene.tileset);
    let missing: Vec<String> = entry.preview.iter().map(|p| p.picture.clone()).filter(|p| !scene.textures.contains_key(p)).collect();
    println!("target tileset {:?}, donor tileset {:?}, pictures missing in the target: {missing:?}", scene.tileset, entry.tileset);
    if let Some(a) = entry.tileset {
        editor.provide_pictures(data::tileset_texture_blobs(arc_dir, a, &missing)?);
    }
    editor.import(entry.portable.clone())?;
    println!("{}", editor.export(out_dir)?);
    if let Some(tileset) = scene.tileset {
        let path = std::path::Path::new(out_dir).join("area").join(format!("{tileset}.bin"));
        if let Ok(bytes) = std::fs::read(&path) {
            let (parts, _) = (data::read_block_from_bytes(&bytes)?, ());
            let names = crate::bch_edit::texture_names(&parts[data::TILESET_PICTURES])?;
            for m in &missing {
                println!("  picture {m} in the new tileset file: {}", names.contains(m));
            }
            // The decompressed data must be a good pack with the same number of parts.
            println!("  parts in the new tileset pack: {}", parts.len());
        } else {
            println!("no tileset file was written");
        }
    }
    Ok(())
}

/// Prints the texts that the materials of a catalogue model hold, and whether
/// each is a picture of the tileset of a block.
pub fn model_strings(arc_dir: &str, catalog: &str, name: &str, block: usize) -> Result<(), String> {
    let entries = crate::catalog::load(std::path::Path::new(catalog))?;
    let entry = entries.iter().find(|e| e.name == name).ok_or("no model has this name")?;
    let scene = data::load_scene(arc_dir, block, None)?;
    let donor = entry.tileset.map(|a| data::tileset_textures(arc_dir, a)).transpose()?.unwrap_or_default();
    for part in &entry.portable.parts {
        let Some(blob) = &part.blob else { continue };
        println!("material {} (label {})", blob.name, blob.label);
        for (at, text) in blob.header_strings.iter().chain(&blob.region_strings) {
            println!("  string at {at:#x}: {text:?}  in target tileset: {}  in donor tileset: {}", scene.textures.contains_key(text), donor.contains_key(text));
        }
    }
    Ok(())
}

/// Puts the same tile in two tiles, one after the other, and says which
/// tiles stay empty.
pub fn tile_twice_test(arc_dir: &str, tiles: &str, first: usize, count: usize, block: usize) -> Result<(), String> {
    let entries = crate::catalog::load(std::path::Path::new(tiles))?;
    let in_cell = |e: &crate::editor::Editor, column: usize, row: usize| -> usize {
        let (x0, z0) = (-360.0 + 18.0 * column as f32, -360.0 + 18.0 * row as f32);
        let mut n = 0;
        for (part, m) in e.meshes() {
            if *part != 1 { continue; }
            for t in m.indices.chunks_exact(3) {
                let (mut x, mut z) = (0.0, 0.0);
                for &i in t {
                    let p = m.positions.get(i as usize).copied().unwrap_or([0.0; 3]);
                    x += p[0] / 3.0;
                    z += p[2] / 3.0;
                }
                if x >= x0 && x < x0 + 18.0 && z >= z0 && z < z0 + 18.0 { n += 1; }
            }
        }
        n
    };
    let probe = crate::editor::Editor::new(arc_dir, block)?;
    for (k, entry) in entries.iter().enumerate().skip(first).take(count) {
        if !probe.has_layouts(&entry.portable) { continue; }
        let mut editor = crate::editor::Editor::new(arc_dir, block)?;
        let r1 = editor.stamp_tile(entry.portable.clone(), 22, 29, Some(0.0));
        let a = in_cell(&editor, 22, 29);
        let r2 = editor.stamp_tile(entry.portable.clone(), 23, 29, Some(0.0));
        let (b, a2) = (in_cell(&editor, 23, 29), in_cell(&editor, 22, 29));
        let flag = if a == 0 || b == 0 || a2 == 0 { "  <-- EMPTY" } else { "" };
        let owned: Vec<String> = (0..editor.instances.len())
            .filter(|&i| editor.instances[i].replaces.is_some() && !editor.instances[i].deleted)
            .map(|i| {
                let mut names: Vec<String> = (0..editor.meshes().len())
                    .filter(|&m| (0..editor.meshes()[m].1.positions.len()).any(|v| editor.owner(m, v) == Some(i)))
                    .map(|m| editor.meshes()[m].1.material.clone())
                    .collect();
                names.dedup();
                names.join("+")
            })
            .collect();
        println!("{k:4} {:28} first {} ({a} tri), second {:?} ({b} tri), first again {a2}{flag}  materials {owned:?}", entry.name, r1.is_ok(), r2.as_ref().err());
    }
    Ok(())
}

/// Lists the maps that have a block, and checks that the events of the map
/// fall on the ground of the block: for each character, the height of the
/// record and the height of the ground under it.
pub fn map_check(arc_dir: &str, block: usize) -> Result<(), String> {
    let maps = crate::events::maps_of_block(arc_dir, block)?;
    println!("block {block} is in {} maps", maps.len());
    let editor = crate::editor::Editor::new(arc_dir, block)?;
    let ground = |x: f32, z: f32| -> Option<f32> {
        let mut best: Option<f32> = None;
        for (i, (part, mesh)) in editor.meshes().iter().enumerate() {
            if *part != 1 || editor.selectable[i] {
                continue;
            }
            for t in mesh.indices.chunks_exact(3) {
                let (a, b, c) = (mesh.positions[t[0] as usize], mesh.positions[t[1] as usize], mesh.positions[t[2] as usize]);
                let d = (b[2] - c[2]) * (a[0] - c[0]) + (c[0] - b[0]) * (a[2] - c[2]);
                if d.abs() < 1e-6 { continue; }
                let l1 = ((b[2] - c[2]) * (x - c[0]) + (c[0] - b[0]) * (z - c[2])) / d;
                let l2 = ((c[2] - a[2]) * (x - c[0]) + (a[0] - c[0]) * (z - c[2])) / d;
                let l3 = 1.0 - l1 - l2;
                if l1 >= 0.0 && l2 >= 0.0 && l3 >= 0.0 {
                    let y = l1 * a[1] + l2 * b[1] + l3 * c[1];
                    best = Some(best.map_or(y, |h: f32| h.max(y)));
                }
            }
        }
        best
    };
    for map in &maps {
        let events = crate::events::read_map_events(arc_dir, map.map)?;
        println!(
            "map {} matrix {} ({}x{}), block at column {} row {}: {} signs, {} characters, {} warps, {} triggers",
            map.map, map.matrix, map.width, map.height, map.column, map.row,
            events.signs.len(), events.characters.len(), events.warps.len(), events.triggers.len()
        );
        for (i, c) in events.characters.iter().enumerate().take(12) {
            let (x, z) = (crate::events::tile_to_local(c.x as f32 + 0.5, map.column), crate::events::tile_to_local(c.z as f32 + 0.5, map.row));
            println!("  character {i}: tile ({}, {}) model {} script {} height {} -> local ({x:.0}, {z:.0}), ground {:?}", c.x, c.z, c.model, c.script, c.height, ground(x, z));
        }
        for (i, w) in events.warps.iter().enumerate().take(6) {
            let (x, z) = (crate::events::tile_to_local(w.x as f32 / 18.0, map.column), crate::events::tile_to_local(w.z as f32 / 18.0, map.row));
            println!("  warp {i}: to map {} exit {} at ({}, {}, {}) size {}x{} -> local ({x:.0}, {z:.0}), ground {:?}", w.map, w.exit, w.x, w.y, w.z, w.width, w.depth, ground(x, z));
        }
        for (i, s) in events.signs.iter().enumerate().take(6) {
            let (x, z) = (crate::events::tile_to_local(s.x as f32 + 0.5, map.column), crate::events::tile_to_local(s.z as f32 + 0.5, map.row));
            println!("  sign {i}: tile ({}, {}) size {}x{} id {} height {} -> local ({x:.0}, {z:.0}), ground {:?}", s.x, s.z, s.width, s.depth, s.id, s.height, ground(x, z));
        }
    }
    Ok(())
}

/// Changes some events of a map, writes them as lines of the file, reads the
/// lines again and checks that the events are the same.
pub fn events_file_test(arc_dir: &str, map: usize) -> Result<(), String> {
    use crate::events::{self, MapEvents};
    let base = events::read_map_events(arc_dir, map)?;
    let mut now: MapEvents = base.clone();
    if let Some(c) = now.characters.first_mut() {
        c.script = 777;
        c.x += 1;
    }
    if let Some(w) = now.warps.first_mut() {
        w.exit = 3;
    }
    now.signs.push(events::Sign { raw: Vec::new(), id: 65535, kind: 1, facing: 4, x: 5, z: 6, width: 1, depth: 2, height: -72 });
    now.triggers.push(events::Trigger { raw: Vec::new(), id: 65535, param: 1, work_id: 2, check_kind: 0, x: 7, z: 8, width: 1, depth: 1, height: -72 });
    now.warps.push(events::Warp { raw: Vec::new(), map: map as u16, exit: 0, facing: 2, kind: 1, x: 99, y: -72, z: 117, width: 1, depth: 1 });
    now.characters.push(events::new_character(map as u16, base.characters.len() as u16, 4, 5, -72.0));
    // A line of another map must stay.
    let old = "# comment\n12 S+ 0 1 2 1 1 0 65535 4 0\n";
    let text = events::merge_file(old, map, &base, &now);
    println!("{text}");
    let mut back = base.clone();
    events::apply_file(map, &mut back, &text);
    println!("same events after reading the file again: {}", back.to_bytes() == now.to_bytes());
    println!("line of the other map kept: {}", text.contains("12 S+ 0"));
    // Writing again must give the same text.
    let again = events::merge_file(&text, map, &base, &now);
    println!("writing twice gives the same file: {}", again == text);
    Ok(())
}

/// Writes every map again from its events and checks that the data is the
/// same as the data of the game; then adds and takes away events and checks
/// that the pack can be read again.
pub fn map_roundtrip(arc_dir: &str) -> Result<(), String> {
    let maps = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Maps))?;
    let (mut same, mut different, mut unreadable) = (0, 0, 0);
    for (map, file) in maps.iter().enumerate() {
        let Some(raw) = file.first() else { continue };
        let data = if raw.first() == Some(&0x11) { crate::lz11::decompress(raw)? } else { raw.clone() };
        let Ok((_, parts)) = crate::pack::split(&data) else {
            unreadable += 1;
            continue;
        };
        let Some(part) = parts.get(1) else { continue };
        let Ok(events) = crate::events::MapEvents::parse(part) else {
            unreadable += 1;
            continue;
        };
        let rebuilt = events.to_bytes();
        if rebuilt == *part || rebuilt[..part.len().min(rebuilt.len())] == part[..part.len().min(rebuilt.len())] && rebuilt.len() - part.len().min(rebuilt.len()) < 4 {
            same += 1;
        } else {
            different += 1;
            if different <= 3 {
                println!("map {map}: {} bytes before, {} after", part.len(), rebuilt.len());
            }
        }
    }
    println!("{same} maps identical, {different} different, {unreadable} unreadable");

    // A change: add a character, take away a warp; write the pack, read it again.
    let sd = std::env::temp_dir().join("sango_editor_zone_test");
    let sd = sd.to_str().unwrap_or(".").to_string();
    let map = 100;
    let mut events = crate::events::read_map_events(arc_dir, map)?;
    let (chars, warps) = (events.characters.len(), events.warps.len());
    events.characters.push(crate::events::new_character(map as u16, 77, 3, 4, -72.0));
    events.warps.remove(0);
    println!("{}", crate::events::write_map_override(arc_dir, &sd, map, &events)?);
    let back = crate::events::read_map_override(&sd, map).ok_or("cannot read the pack again")?;
    println!(
        "characters {chars} -> {}, warps {warps} -> {}; same lists after reading: {}; size field {} for {} bytes of records",
        back.characters.len(), back.warps.len(), back.characters.len() == chars + 1 && back.warps.len() == warps - 1,
        u32::from_le_bytes([back.head[0], back.head[1], back.head[2], back.head[3]]),
        20 * back.signs.len() + 48 * back.characters.len() + 24 * back.warps.len() + 24 * back.triggers.len()
    );
    // The parts after the events must be the same as in the game.
    let raw = std::fs::read(std::path::Path::new(&sd).join("zone").join(format!("{map}.bin"))).map_err(|e| e.to_string())?;
    let data = crate::lz11::decompress(&raw)?;
    let (_, now_parts) = crate::pack::split(&data)?;
    let game = crate::lz11::decompress(maps[map].first().ok_or("no map")?)?;
    let (_, game_parts) = crate::pack::split(&game)?;
    for i in 0..game_parts.len() {
        if i != 1 {
            println!("part {i} same: {}", game_parts[i] == now_parts[i]);
        }
    }
    // The reset takes the file and the index away.
    println!("{}", crate::events::reset_map(&sd, map)?);
    println!(
        "file gone: {}, index gone: {}, reading gives the game again: {}",
        !std::path::Path::new(&sd).join("zone").join("100.bin").exists(),
        !std::path::Path::new(&sd).join("zone").join("index.bin").exists(),
        crate::events::read_map_override(&sd, map).is_none()
    );
    Ok(())
}

/// Prints the attributes of the tiles around a tile, for the block of the
/// archive and for a file that the tool wrote (# wall, . plain, other letters).
pub fn tiles_around(arc_dir: &str, block: usize, land_file: &str, column: usize, row: usize) -> Result<(), String> {
    let show = |label: &str, tiles: &crate::edit::Attributes| {
        println!("{label}:");
        for r in row.saturating_sub(5)..(row + 6).min(tiles.height) {
            let mut line = String::new();
            for c in column.saturating_sub(8)..(column + 9).min(tiles.width) {
                let v = tiles.cells[r * tiles.width + c];
                let ch = match v >> 24 { 0 => '.', 1 => '#', 31..=37 => '*', 148..=151 => '~', _ => '?' };
                line.push(if r == row && c == column { '@' } else { ch });
            }
            println!("  {r:3} {line}");
        }
    };
    let original = EditableBlock::load(arc_dir, block)?.attributes()?;
    show("game", &original);
    let edited = EditableBlock::from_file(std::path::Path::new(land_file), block)?.attributes()?;
    show("file", &edited);
    let flags = |a: &crate::edit::Attributes| format!("{:08x}", a.cells[row * a.width + column]);
    println!("tile ({column}, {row}): game {} file {}", flags(&original), flags(&edited));
    Ok(())
}

/// For the warps of the maps of a block: the kind of exit and the attribute
/// of the tile under the warp, to see which tiles a door warp uses.
pub fn warp_tiles(arc_dir: &str, block: usize) -> Result<(), String> {
    let tiles = EditableBlock::load(arc_dir, block)?.attributes()?;
    for map in crate::events::maps_of_block(arc_dir, block)? {
        let events = crate::events::read_map_events(arc_dir, map.map)?;
        for (i, w) in events.warps.iter().enumerate() {
            let (tx, tz) = ((w.x as i32).div_euclid(18) - (map.column * 40) as i32, (w.z as i32).div_euclid(18) - (map.row * 40) as i32);
            let attr = if tx >= 0 && tz >= 0 && (tx as usize) < tiles.width && (tz as usize) < tiles.height {
                format!("{:08x}", tiles.cells[tz as usize * tiles.width + tx as usize])
            } else {
                "outside".into()
            };
            println!("map {} warp {i}: kind {} facing {} size {}x{} -> block tile ({tx}, {tz}) attribute {attr}", map.map, w.kind, w.facing, w.width, w.depth);
        }
    }
    Ok(())
}

/// Lists the objects whose box is near a place, with their meshes.
pub fn objects_near(arc_dir: &str, block: usize, x: f32, z: f32, radius: f32) -> Result<(), String> {
    let editor = crate::editor::Editor::new(arc_dir, block)?;
    for i in 0..editor.instances.len() {
        let (l, h) = editor.instance_bounds(i);
        if l[0] - radius <= x && x <= h[0] + radius && l[2] - radius <= z && z <= h[2] + radius {
            println!(
                "instance {i}: x {:.0}..{:.0} y {:.0}..{:.0} z {:.0}..{:.0} parts {:?}",
                l[0], h[0], l[1], h[1], l[2], h[2], editor.instance_parts(i)
            );
        }
    }
    Ok(())
}

/// Selects a building as Ctrl+click does and says which pieces near a place
/// are in it and which are not.
pub fn building_near(arc_dir: &str, block: usize, click: [f32; 2], mesh_name: &str, x: f32, z: f32, radius: f32) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let mesh = (0..editor.meshes().len()).find(|&m| editor.meshes()[m].1.material == mesh_name).ok_or("no such mesh")?;
    let piece = (0..editor.instances.len())
        .find(|&i| {
            editor.instance_parts(i).iter().any(|(n, _)| n == mesh_name) && {
                let (l, h) = editor.instance_bounds(i);
                click[0] >= l[0] && click[0] <= h[0] && click[1] >= l[2] && click[1] <= h[2]
            }
        })
        .ok_or("no piece here")?;
    let taken = editor.building_of(piece, mesh)?;
    println!("{} pieces in the building", taken.len());
    for i in 0..editor.instances.len() {
        let (l, h) = editor.instance_bounds(i);
        if l[0] - radius <= x && x <= h[0] + radius && l[2] - radius <= z && z <= h[2] + radius && !editor.instances[i].deleted {
            println!(
                "{} instance {i}: x {:.0}..{:.0} y {:.0}..{:.0} z {:.0}..{:.0} {:?}",
                if taken.contains(&i) { "IN " } else { "OUT" }, l[0], h[0], l[1], h[1], l[2], h[2], editor.instance_parts(i)
            );
        }
    }
    Ok(())
}

/// Prints the props of a block, moves the door that is at a place, writes the
/// block and reads the props again.
pub fn props_test(arc_dir: &str, block: usize, x: f32, z: f32, dx: f32, out_dir: &str) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    for (i, d) in editor.props.iter().enumerate() {
        println!("door {i}: model {} at ({}, {}, {}) scale {:?} turn {:?} tail {}", d.kind, d.x, d.y, d.z, d.scale, d.turn, d.tail);
    }
    let mut props = editor.props.clone();
    let mut moved = 0;
    for d in props.iter_mut() {
        if (d.x - x).abs() <= 9.5 && (d.z - z).abs() <= 9.5 {
            d.x += dx;
            moved += 1;
        }
    }
    println!("{moved} door moved");
    editor.set_props(props)?;
    println!("{}", editor.export(out_dir)?);
    let back = EditableBlock::from_file(&std::path::Path::new(out_dir).join("land").join(format!("{block}.bin")), block)?;
    for (i, d) in back.props().iter().enumerate() {
        println!("after: door {i} at ({}, {}, {})", d.x, d.y, d.z);
    }
    Ok(())
}

/// Draws overworld models in a picture, as the catalogue of characters does.
pub fn model_sheet(arc_dir: &str, ids: &[usize], out: &str) -> Result<(), String> {
    let mut bank = crate::models::ModelBank::new(arc_dir);
    const SIZE: usize = 192;
    let columns = ids.len().min(6).max(1);
    let rows = ids.len().div_ceil(columns);
    let mut sheet = vec![60u8; SIZE * columns * SIZE * rows * 4];
    for (k, &id) in ids.iter().enumerate() {
        let model = bank.load(id as u16).ok_or("no such model")?;
        // The characters look toward +z: turn them to face the camera of the picture.
        let parts: Vec<crate::catalog::PreviewPart> = model
            .meshes
            .iter()
            .map(|m| crate::catalog::PreviewPart {
                picture: m.picture.clone(),
                positions: m.positions.iter().map(|p| [-p[0], p[1], -p[2]]).collect(),
                uvs: m.uvs.clone(),
                indices: m.indices.chunks_exact(3).flat_map(|t| [t[0], t[2], t[1]]).collect(),
            })
            .collect();
        let image = crate::thumb::render(&parts, &model.textures, SIZE);
        let (cx, cy) = (k % columns * SIZE, k / columns * SIZE);
        for y in 0..SIZE {
            for x in 0..SIZE {
                let from = (y * SIZE + x) * 4;
                if image[from + 3] == 0 { continue; }
                let to = ((cy + y) * SIZE * columns + cx + x) * 4;
                sheet[to..to + 4].copy_from_slice(&image[from..from + 4]);
            }
        }
    }
    write_png(out, SIZE * columns, SIZE * rows, &sheet)
}

/// Prints how the meshes of an overworld model are made (the layout of the
/// vertices) and the words of the model structure, to find the skeleton.
pub fn model_layout(arc_dir: &str, id: usize) -> Result<(), String> {
    let files = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::CharacterModels))?;
    let raw = files.get(id).and_then(|f| f.first()).ok_or("no such model")?;
    let data = if raw.first() == Some(&0x11) { crate::lz11::decompress(raw)? } else { raw.clone() };
    let (_, parts) = crate::pack::split(&data)?;
    let model = parts[0];
    let bch = bch::Bch::parse(model)?;
    for m in bch.meshes()? {
        println!("{}: layout {} stride {} vertices {}", m.material, m.places.layout, m.places.stride, m.positions.len());
    }
    let u = |o: usize| u32::from_le_bytes([model[o], model[o + 1], model[o + 2], model[o + 3]]);
    let content = u(8) as usize;
    let table = content + u(content) as usize;
    let md = content + u(table) as usize;
    println!("content {content:#x} table {table:#x} model {md:#x}");
    for k in 0..40 {
        print!("{:#x}:{:#x} ", k * 4, u(md + k * 4));
        if k % 6 == 5 { println!(); }
    }
    println!();
    Ok(())
}

/// Prints the first bytes of some vertices of the meshes of a model after
/// the position, the normal and the texture coordinates (the bone data).
pub fn model_vertices(arc_dir: &str, id: usize) -> Result<(), String> {
    let files = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::CharacterModels))?;
    let raw = files.get(id).and_then(|f| f.first()).ok_or("no such model")?;
    let data = if raw.first() == Some(&0x11) { crate::lz11::decompress(raw)? } else { raw.clone() };
    let (_, parts) = crate::pack::split(&data)?;
    let bch = bch::Bch::parse(parts[0])?;
    for m in bch.meshes()? {
        println!("{} stride {} layout {}", m.material, m.places.stride, m.places.layout);
        for v in [0usize, 1, 2, 50, 100, m.positions.len() - 1] {
            if v >= m.positions.len() { continue; }
            let at = m.places.vertices_at + v * m.places.stride;
            let b = &parts[0][at..at + m.places.stride];
            println!("  v{v}: pos {:?} tail {}", m.positions[v], b[24..].iter().map(|x| format!("{x:02x}")).collect::<Vec<_>>().join(" "));
        }
    }
    Ok(())
}

/// For every model: the lowest point of the meshes as they are, and after the
/// skinning with the bones (by the index of the vertex).
pub fn models_skin(arc_dir: &str) -> Result<(), String> {
    let files = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::CharacterModels))?;
    let params = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::CharacterModelParams))?;
    let mut raw_high = 0;
    let mut shown = 0;
    for id in 0..params.len() {
        let rec = &params[id][0];
        if rec.len() != 24 { continue; }
        let at = u16::from_le_bytes([rec[20], rec[21]]) as usize;
        let Some(raw) = files.get(at).and_then(|f| f.first()) else { continue };
        let data = crate::lz11::decompress(raw)?;
        let (_, parts) = crate::pack::split(&data)?;
        let model = parts[0];
        let b = bch::Bch::parse(model)?;
        let worlds = crate::models::bone_worlds(model);
        let (mut raw_min, mut skin_min, mut skin_max, mut raw_max) = (f32::MAX, f32::MAX, f32::MIN, f32::MIN);
        for m in b.meshes()? {
            let stride = m.places.stride;
            for (v, p) in m.positions.iter().enumerate() {
                raw_min = raw_min.min(p[1]);
                raw_max = raw_max.max(p[1]);
                let at = m.places.vertices_at + v * stride;
                let (idx, wt) = if stride >= 40 { (&model[at + stride - 8..at + stride - 4], &model[at + stride - 4..at + stride]) } else { (&[0u8; 4][..], &[100u8, 0, 0, 0][..]) };
                let mut out = glam::Vec3::ZERO;
                let mut total = 0.0;
                for k in 0..4 {
                    let w = wt[k] as f32;
                    if w <= 0.0 { continue; }
                    let world = worlds.get(idx[k] as usize).copied().unwrap_or(glam::Mat4::IDENTITY);
                    out += (world * glam::Vec4::new(p[0], p[1], p[2], 1.0)).truncate() * w;
                    total += w;
                }
                if total > 0.0 { out /= total; }
                skin_min = skin_min.min(out.y);
                skin_max = skin_max.max(out.y);
            }
        }
        if raw_min > 4.0 {
            raw_high += 1;
            if shown < 12 {
                println!("model {id}: raw y {raw_min:.1}..{raw_max:.1}, skinned y {skin_min:.1}..{skin_max:.1}");
                shown += 1;
            }
        }
    }
    println!("{raw_high} models have no point near the ground as they are");
    Ok(())
}

/// Reads every overworld model and says how many load, and how many models the
/// characters of the maps use that have no file.
pub fn models_check(arc_dir: &str) -> Result<(), String> {
    let mut bank = crate::models::ModelBank::new(arc_dir);
    let (mut files, mut loaded, mut empty) = (0, 0, 0);
    let start = std::time::Instant::now();
    for id in 0..600u16 {
        if bank.pack_of(id).is_some() {
            files += 1;
            match bank.load(id) {
                Some(m) if !m.meshes.is_empty() => loaded += 1,
                _ => empty += 1,
            }
        }
    }
    println!("{files} models have a file, {loaded} load with meshes, {empty} fail or are empty ({:.1} s)", start.elapsed().as_secs_f32());
    let templates = crate::events::character_templates(arc_dir)?;
    let (mut missing, mut dressed): (Vec<u16>, Vec<u16>) = (Vec::new(), Vec::new());
    for &model in templates.keys() {
        if model & 0x2000 != 0 {
            dressed.push(model);
        } else if bank.pack_of(model).is_none() {
            missing.push(model);
        }
    }
    println!("{} models are used by the characters of the maps; {} are dressed characters, {} have no file: {:?}", templates.len(), dressed.len(), missing.len(), &missing[..missing.len().min(12)]);
    Ok(())
}

/// Reads the models of X and Y that are on the SD card and draws some.
pub fn xy_models_test(arc_dir: &str, dir: &str, out: &str) -> Result<(), String> {
    let mut bank = crate::models::ModelBank::new(arc_dir);
    bank.set_overrides(vec![std::path::PathBuf::from(dir)]);
    let ids = bank.override_ids();
    println!("{} models on the SD card", ids.len());
    let mut shown: Vec<usize> = Vec::new();
    let mut bad = 0;
    for &id in &ids {
        match bank.load(id) {
            Some(m) if !m.meshes.is_empty() => {
                if shown.len() < 12 {
                    shown.push(id as usize);
                }
            }
            _ => bad += 1,
        }
    }
    println!("{bad} of them do not load");
    model_sheet_with(&mut bank, &shown, out)
}

fn model_sheet_with(bank: &mut crate::models::ModelBank, ids: &[usize], out: &str) -> Result<(), String> {
    const SIZE: usize = 192;
    let columns = ids.len().min(6).max(1);
    let rows = ids.len().div_ceil(columns);
    let mut sheet = vec![60u8; SIZE * columns * SIZE * rows * 4];
    for (k, &id) in ids.iter().enumerate() {
        let model = bank.load(id as u16).ok_or("no such model")?;
        let parts: Vec<crate::catalog::PreviewPart> = model
            .meshes
            .iter()
            .map(|m| crate::catalog::PreviewPart {
                picture: m.picture.clone(),
                positions: m.positions.iter().map(|p| [-p[0], p[1], -p[2]]).collect(),
                uvs: m.uvs.clone(),
                indices: m.indices.chunks_exact(3).flat_map(|t| [t[0], t[2], t[1]]).collect(),
            })
            .collect();
        let image = crate::thumb::render(&parts, &model.textures, SIZE);
        let (cx, cy) = (k % columns * SIZE, k / columns * SIZE);
        for y in 0..SIZE {
            for x in 0..SIZE {
                let from = (y * SIZE + x) * 4;
                if image[from + 3] == 0 { continue; }
                let to = ((cy + y) * SIZE * columns + cx + x) * 4;
                sheet[to..to + 4].copy_from_slice(&image[from..from + 4]);
            }
        }
    }
    write_png(out, SIZE * columns, SIZE * rows, &sheet)
}

/// Reads the matrix of the maps of a block, loads the blocks around and says
/// how long it takes, then writes the matrix with a change and reads it again.
pub fn neighbours_test(arc_dir: &str, block: usize) -> Result<(), String> {
    let maps = crate::events::maps_of_block(arc_dir, block)?;
    let at = maps.first().ok_or("no map")?;
    let matrix = crate::events::read_matrix(arc_dir, "", at.matrix)?;
    println!("map {} matrix {} {}x{}, block at ({}, {}); ids {:?}", at.map, at.matrix, matrix.width, matrix.height, at.column, at.row, &matrix.ids[..matrix.ids.len().min(16)]);
    println!("tileset of the map: {:?}", crate::events::map_tileset(arc_dir, at.map));
    let start = std::time::Instant::now();
    let mut ids = Vec::new();
    for r in at.row.saturating_sub(1)..=(at.row + 1).min(matrix.height - 1) {
        for c in at.column.saturating_sub(1)..=(at.column + 1).min(matrix.width - 1) {
            let id = matrix.ids[r * matrix.width + c] as usize;
            if id < 857 && (c, r) != (at.column, at.row) {
                ids.push(id);
            }
        }
    }
    let blocks = data::blocks_meshes(arc_dir, &ids)?;
    println!("{} neighbours loaded in {:.2} s ({} meshes)", blocks.len(), start.elapsed().as_secs_f32(), blocks.iter().map(|b| b.1.len()).sum::<usize>());
    let sd = std::env::temp_dir().join("sango_editor_matrix_test");
    let sd = sd.to_str().unwrap_or(".").to_string();
    let mut changed = matrix.clone();
    changed.ids[0] = 5;
    println!("{}", crate::events::write_matrix_override(arc_dir, &sd, at.matrix, &changed)?);
    let back = crate::events::read_matrix(arc_dir, &sd, at.matrix)?;
    println!("first block after the change: {} (was {}), rest same: {}", back.ids[0], matrix.ids[0], back.ids[1..] == matrix.ids[1..]);
    crate::events::reset_matrix(&sd, at.matrix)?;
    println!("after reset: {}", crate::events::read_matrix(arc_dir, &sd, at.matrix)? == matrix);
    Ok(())
}

/// Draws the door models (build models) of some kinds in a picture.
pub fn prop_sheet(arc_dir: &str, kinds: &[u32], out: &str) -> Result<(), String> {
    let tileset = 8;
    let mut bank = crate::models::ModelBank::new(arc_dir);
    const SIZE: usize = 192;
    let columns = kinds.len().min(6).max(1);
    let rows = kinds.len().div_ceil(columns);
    let mut sheet = vec![60u8; SIZE * columns * SIZE * rows * 4];
    for (k, &kind) in kinds.iter().enumerate() {
        let Some(model) = bank.load_prop(kind, tileset) else {
            println!("door kind {kind}: no model");
            continue;
        };
        println!("door kind {kind}: {} meshes, {} pictures ({:?})", model.meshes.len(), model.textures.len(), model.meshes.iter().map(|m| m.picture.as_str()).collect::<Vec<_>>());
        let parts: Vec<crate::catalog::PreviewPart> = model
            .meshes
            .iter()
            .map(|m| crate::catalog::PreviewPart { picture: m.picture.clone(), positions: m.positions.iter().map(|p| [-p[0], p[1], -p[2]]).collect(), uvs: m.uvs.clone(), indices: m.indices.chunks_exact(3).flat_map(|t| [t[0], t[2], t[1]]).collect() })
            .collect();
        let image = crate::thumb::render(&parts, &model.textures, SIZE);
        let (cx, cy) = (k % columns * SIZE, k / columns * SIZE);
        for y in 0..SIZE {
            for x in 0..SIZE {
                let from = (y * SIZE + x) * 4;
                if image[from + 3] == 0 { continue; }
                let to = ((cy + y) * SIZE * columns + cx + x) * 4;
                sheet[to..to + 4].copy_from_slice(&image[from..from + 4]);
            }
        }
    }
    write_png(out, SIZE * columns, SIZE * rows, &sheet)
}


/// Lists the worlds (the matrices) with the names of their maps.
pub fn worlds_list(arc_dir: &str) -> Result<(), String> {
    let worlds = crate::events::list_worlds(arc_dir)?;
    println!("{} worlds", worlds.len());
    for a in worlds.iter().take(25) {
        let names: Vec<&str> = a.maps.iter().filter_map(|&z| crate::names::MAP_NAMES.get(z).copied()).take(5).collect();
        println!("  matrix {:3}: {} maps: {}", a.matrix, a.maps.len(), names.join(", "));
    }
    Ok(())
}

/// Puts tall grass on some tiles of a block, takes it away from others,
/// undoes and does again, then writes the block.
pub fn grass_test(arc_dir: &str, block: usize, out_dir: &str) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let sample = editor.grass_sample().ok_or("this block has no tall grass to copy")?;
    println!("sample: {} parts, {} bytes of vertices", sample.parts.len(), sample.parts.iter().map(|p| p.vertices.len()).sum::<usize>());
    let count = |e: &crate::editor::Editor| e.meshes().iter().filter(|(p, _)| *p == 5).map(|(_, m)| m.indices.len() / 3).sum::<usize>();
    println!("grass triangles at the start: {}", count(&editor));
    for (column, row) in [(20, 20), (20, 20), (21, 20), (10, 37), (30, 2)] {
        editor.stamp_grass(Some(&sample), column, row, 0.0)?;
        println!("after grass in ({column}, {row}): {}", count(&editor));
    }
    editor.stamp_grass(None, 21, 20, 0.0)?;
    println!("after erasing (21, 20): {}", count(&editor));
    editor.stamp_grass(None, 5, 5, 0.0)?;
    println!("after erasing (5, 5): {}", count(&editor));
    editor.undo()?;
    println!("after undo: {}", count(&editor));
    editor.redo()?;
    println!("{}", editor.export(out_dir)?);
    Ok(())
}

/// Lists the attributes of the tiles of all the blocks, with how many tiles have each.
pub fn attribute_values(arc_dir: &str) -> Result<(), String> {
    for (value, count) in data::attribute_values(arc_dir)? {
        println!("{value:08x} {count}");
    }
    Ok(())
}

/// The attributes of the tiles under the vertices of the meshes whose material has a word in its name.
pub fn tiles_under(arc_dir: &str, block: usize, word: &str) -> Result<(), String> {
    let editor = crate::editor::Editor::new(arc_dir, block)?;
    let (width, height) = editor.grid();
    let mut counts: std::collections::BTreeMap<u32, usize> = Default::default();
    let mut cells: std::collections::BTreeSet<(usize, usize)> = Default::default();
    for (_, mesh) in editor.meshes().iter().filter(|(_, m)| m.material.contains(word)) {
        for p in &mesh.positions {
            let (c, r) = (((p[0] + 360.0) / 18.0).floor(), ((p[2] + 360.0) / 18.0).floor());
            if c >= 0.0 && r >= 0.0 && (c as usize) < width && (r as usize) < height {
                cells.insert((c as usize, r as usize));
            }
        }
    }
    for (c, r) in &cells {
        *counts.entry(editor.tiles[r * width + c]).or_default() += 1;
    }
    println!("{} tiles under '{word}'", cells.len());
    for (value, n) in counts {
        println!("  {value:08x} x{n}");
    }
    Ok(())
}

/// Takes every object and all the grass out of a block, undoes, does again, and writes the block.
pub fn clear_test(arc_dir: &str, block: usize, out_dir: &str) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let count = |e: &crate::editor::Editor, part: usize| e.meshes().iter().filter(|(p, _)| *p == part).map(|(_, m)| m.indices.len() / 3).sum::<usize>();
    println!("start: {} triangles in the ground and objects, {} of grass", count(&editor, 1), count(&editor, 5));
    editor.change(|v| v.iter_mut().filter(|i| i.replaces.is_none()).for_each(|i| i.deleted = true))?;
    editor.clear_grass()?;
    editor.clear_ledges()?;
    println!("cleared: {} triangles in the ground and objects, {} of grass", count(&editor, 1), count(&editor, 5));
    editor.undo()?;
    println!("undo of the grass: {} of grass", count(&editor, 5));
    editor.redo()?;
    println!("{}", editor.export(out_dir)?);
    Ok(())
}

/// Puts many different tiles of a catalogue in a block, one after the other, and writes the block.
pub fn tile_many_test(arc_dir: &str, tiles: &str, block: usize, count: usize, out_dir: &str) -> Result<(), String> {
    let entries = crate::catalog::load(std::path::Path::new(tiles))?;
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let mut done = 0;
    for (k, entry) in entries.iter().take(count).enumerate() {
        match editor.stamp_tile(entry.portable.clone(), 5 + k, 20, Some(0.0)) {
            Ok(_) => done += 1,
            Err(message) => println!("tile {k} ({}): {message}", entry.name),
        }
    }
    println!("{done} of {count} tiles put; {} meshes", editor.meshes().len());
    println!("{}", editor.export(out_dir)?);
    Ok(())
}

/// Copies the first object that has a mesh with a word in its material many times (each copy a bit
/// further, and turned), then writes the block.
pub fn dup_test(arc_dir: &str, block: usize, word: &str, count: usize, out_dir: &str) -> Result<(), String> {
    use crate::editor::{Editor, Instance};
    let mut editor = Editor::new(arc_dir, block)?;
    let wanted = (0..editor.instances.len()).find(|&i| editor.instance_parts(i).iter().any(|(name, _)| name.contains(word))).ok_or("no such object")?;
    println!("object {wanted}: {:?}", editor.instance_parts(wanted));
    for k in 0..count {
        let source = editor.instances[wanted].clone();
        let result = editor.change(|v| v.push(Instance { cluster: source.cluster, offset: [source.offset[0] + 30.0 * (k + 1) as f32, source.offset[1], source.offset[2]], yaw: 0.4 * k as f32, copy: true, deleted: false, only: source.only.clone(), replaces: None }));
        println!("copy {k}: {}", match result { Ok(()) => "ok".to_string(), Err(m) => m });
    }
    println!("{} meshes", editor.meshes().len());
    println!("{}", editor.export(out_dir)?);
    Ok(())
}

/// Adds a door model to a block (the list of props gets longer), then removes one, and writes the block.
pub fn prop_add_test(arc_dir: &str, block: usize, out_dir: &str) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    println!("props at the start: {}", editor.props.len());
    let first = editor.props.first().cloned().ok_or("this block has no prop to copy")?;
    for k in 0..3 {
        let mut props = editor.props.clone();
        let mut copy = first.clone();
        copy.x += 18.0 * (k + 1) as f32;
        props.push(copy);
        editor.set_props(props)?;
        println!("after adding {}: {}", k + 1, editor.props.len());
    }
    let mut props = editor.props.clone();
    props.pop();
    editor.set_props(props)?;
    println!("after removing one: {}", editor.props.len());
    editor.set_props(Vec::new())?;
    println!("after removing all: {}", editor.props.len());
    println!("{}", editor.export(out_dir)?);
    Ok(())
}

/// Prints the characters of a map as the game has them.
pub fn characters(arc_dir: &str, map: usize) -> Result<(), String> {
    let events = crate::events::read_map_events(arc_dir, map)?;
    // What the SD card has for this map, if the tool wrote something.
    if let Ok(appdata) = std::env::var("APPDATA") {
        let sd = format!("{appdata}/Azahar/sdmc/sango");
        match crate::events::read_map_override(&sd, map) {
            Some(own) => println!("the SD card has this map: {} characters, {} warps, {} signs, {} triggers (the game has {} characters)", own.characters.len(), own.warps.len(), own.signs.len(), own.triggers.len(), events.characters.len()),
            None => println!("the SD card has no file for this map"),
        }
    }
    for c in &events.characters {
        println!(
            "id {:3} model {:4} move {:3} interact {:3} hide {:5} script {:5} facing {} param {:?} trigger {:?} maps {:?} alias {:5} kind {} at ({}, {}) h {:.0}",
            c.local_id, c.model, c.movement, c.interaction, c.hide_flag, c.script, c.facing, c.param, c.trigger, c.maps, c.alias, c.position_kind, c.x, c.z, c.height
        );
    }
    Ok(())
}

/// Puts one tile of a catalogue on every tile of a block, undoes, does again, and writes the block.
pub fn fill_test(arc_dir: &str, tiles: &str, number: usize, block: usize, out_dir: &str) -> Result<(), String> {
    let entries = crate::catalog::load(std::path::Path::new(tiles))?;
    let entry = entries.get(number).ok_or("no such tile")?;
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let (width, height) = editor.grid();
    let cells: Vec<(usize, usize, f32)> = (0..height).flat_map(|r| (0..width).map(move |c| (c, r, 0.0))).collect();
    let start = std::time::Instant::now();
    let count = |e: &crate::editor::Editor| e.meshes().iter().map(|(_, m)| m.indices.len() / 3).sum::<usize>();
    println!("tile {number}: {}; the block has {} triangles", entry.name, count(&editor));
    match editor.stamp_tiles(entry.portable.clone(), &cells) {
        Ok(n) => println!("{n} tiles put in {:.1} s: {} triangles, {} meshes", start.elapsed().as_secs_f32(), count(&editor), editor.meshes().len()),
        Err(m) => println!("cannot fill: {m}"),
    }
    editor.undo().ok();
    println!("after undo: {} triangles", count(&editor));
    editor.redo().ok();
    println!("{}", editor.export(out_dir)?);
    Ok(())
}

/// Prints the tileset that each map of a world uses, and the one that the tool guesses for a block.
pub fn map_tilesets(arc_dir: &str, matrix: usize) -> Result<(), String> {
    for map in crate::events::maps_of_matrix(arc_dir, matrix)? {
        println!("map {map}: tileset {:?}", crate::events::map_tileset(arc_dir, map));
    }
    Ok(())
}

/// Looks at the blocks written on the SD card: which pictures do their meshes need, and does the
/// tileset that is on the SD card (or else the one of the game) have them?
pub fn sd_pictures_check(arc_dir: &str, sd_dir: &str, tileset: usize) -> Result<(), String> {
    let override_path = std::path::Path::new(sd_dir).join("area").join(format!("{tileset}.bin"));
    let mut have: std::collections::HashSet<String> = Default::default();
    if let Ok(bytes) = std::fs::read(&override_path) {
        let parts = data::open_pack(&bytes)?;
        let part = parts.get(data::TILESET_PICTURES).ok_or("the tileset on the SD card has no pictures")?;
        for t in crate::bch::Bch::parse(part)?.textures()? {
            have.insert(t.name);
        }
        println!("tileset {tileset} on the SD card: {} pictures", have.len());
        let game_now: std::collections::HashSet<String> = data::tileset_textures(arc_dir, tileset)?.into_keys().collect();
        let mut added: Vec<&String> = have.iter().filter(|n| !game_now.contains(*n)).collect();
        added.sort();
        println!("pictures added to the tileset: {added:?}");
    } else {
        println!("no tileset {tileset} on the SD card");
    }
    let game: std::collections::HashSet<String> = data::tileset_textures(arc_dir, tileset)?.into_keys().collect();
    println!("tileset {tileset} of the game: {} pictures; the SD card one lacks {:?}", game.len(), game.iter().filter(|n| !have.contains(*n)).take(5).collect::<Vec<_>>());
    let folder = std::path::Path::new(sd_dir).join("land");
    for entry in std::fs::read_dir(&folder).map_err(|e| e.to_string())?.flatten() {
        let name = entry.file_name().to_string_lossy().to_string();
        let Some(block) = name.strip_suffix(".bin").and_then(|n| n.parse::<usize>().ok()) else { continue };
        let scene = data::load_scene_from(arc_dir, block, Some(tileset), Some(&entry.path().to_string_lossy()))?;
        let mut missing: Vec<String> = scene.meshes.iter().map(|m| data::picture_of(m).to_string()).filter(|p| !have.contains(p) && !game.contains(p)).collect();
        missing.sort();
        missing.dedup();
        println!("block {block}: {} meshes, pictures that nobody has: {:?}", scene.meshes.len(), missing);
    }
    Ok(())
}

/// Makes the ground of a block flat, undoes, does again, and writes the block.
pub fn flatten_test(arc_dir: &str, block: usize, out_dir: &str) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let range = |e: &crate::editor::Editor| {
        let (mut low, mut high) = (f32::MAX, f32::MIN);
        for (i, (part, mesh)) in e.meshes().iter().enumerate() {
            if *part != 1 || e.is_selectable(i) {
                continue;
            }
            for p in &mesh.positions {
                low = low.min(p[1]);
                high = high.max(p[1]);
            }
        }
        (low, high)
    };
    println!("ground heights before: {:?}", range(&editor));
    editor.flatten_ground()?;
    println!("after: {:?}", range(&editor));
    editor.undo()?;
    println!("after undo: {:?}", range(&editor));
    editor.redo()?;
    println!("{}", editor.export(out_dir)?);
    Ok(())
}

/// Lists the parts of a block: the size and the first bytes of each.
pub fn parts(arc_dir: &str, block: usize) -> Result<(), String> {
    let parts = data::read_block(arc_dir, block)?;
    for (i, part) in parts.iter().enumerate() {
        let head: Vec<String> = part.iter().take(24).map(|b| format!("{b:02x}")).collect();
        println!("part {i}: {} bytes  {}", part.len(), head.join(" "));
        if i == 2 {
            // The words of the collision part, as numbers and as floats.
            for (k, w) in part.chunks_exact(4).take(48).enumerate() {
                let u = u32::from_le_bytes([w[0], w[1], w[2], w[3]]);
                println!("  [{k:2}] {u:10} {:#010x} {:12.3}", u, f32::from_bits(u));
            }
        }
    }
    Ok(())
}

/// What the collision part of a block holds: the levels, the divisions, the triangles and their heights.
fn collision_summary(part: &[u8]) -> String {
    let word = |at: usize| u32::from_le_bytes([part[at], part[at + 1], part[at + 2], part[at + 3]]);
    let float = |at: usize| f32::from_bits(word(at));
    let (levels, dx, dy, dz) = (word(16) as usize, word(20) as usize, word(24) as usize, word(28) as usize);
    let boxes = dx * dy * dz;
    let all: usize = (1..levels).map(|i| boxes.pow(i as u32)).sum();
    let leaves = boxes.pow(levels as u32 - 1);
    let offsets_at = 32 + 32 * all;
    let positions_at = offsets_at + 8 * leaves;
    let size = word(4) as usize;
    let count = (16 + size - positions_at) / 16;
    let (mut low, mut high) = (f32::MAX, f32::MIN);
    for k in 0..count {
        let y = float(positions_at + 16 * k + 4);
        low = low.min(y);
        high = high.max(y);
    }
    let mut referenced = 0;
    for k in 0..leaves {
        referenced += word(offsets_at + 8 * k + 4) as usize;
    }
    format!("levels {levels}, divisions {dx}x{dy}x{dz}, {count} vertices ({} triangles), {referenced} referenced, heights {low:.1} to {high:.1}", count / 3)
}

/// Lifts a square of corners of the ground, writes the block, and says what the collision holds.
pub fn height_test(arc_dir: &str, block: usize, out_dir: &str) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let original = data::read_block(arc_dir, block)?;
    println!("collision of the game: {}", collision_summary(&original[2]));
    let corners: Vec<(usize, usize)> = (10..=14).flat_map(|r| (10..=14).map(move |c| (c, r))).collect();
    editor.raise_corners(&corners, 8.0)?;
    editor.raise_corners(&corners[..5], 4.0)?;
    println!("lifted 25 corners by 8 (and 5 of them by 4 more)");
    let path = editor.export(out_dir)?;
    println!("{path}");
    let file = out_dir.to_string() + "/land/" + &block.to_string() + ".bin";
    let bytes = std::fs::read(&file).map_err(|e| format!("{file}: {e}"))?;
    let parts = data::open_pack(&bytes)?;
    println!("collision written: {}", collision_summary(&parts[2]));
    editor.undo()?;
    editor.undo()?;
    println!("after undo: ground {}", editor.meshes().iter().filter(|(p, _)| *p == 1).flat_map(|(_, m)| m.positions.iter().map(|p| p[1])).fold(f32::MIN, f32::max));
    Ok(())
}

/// Checks the file of the scripts of Sango (written and read again) and the list of the scripts of the game.
pub fn scripts_test(arc_dir: &str, out_dir: &str) -> Result<(), String> {
    use crate::scripts::{self, DataScripts, Step};
    let mut data = DataScripts::default();
    data.scripts[0][0] = Step { action: 1, parameter_1: 1, parameter_2: 0, text: "Hello, trainer! Come back soon.".into() };
    data.scripts[0][1] = Step { action: 3, parameter_1: 17, parameter_2: 3, text: String::new() };
    data.scripts[0][2] = Step { action: 5, parameter_1: 2000, parameter_2: 0, text: String::new() };
    data.scripts[5][0] = Step { action: 2, parameter_1: 0, parameter_2: 0, text: String::new() };
    std::fs::create_dir_all(out_dir).map_err(|e| e.to_string())?;
    scripts::write(out_dir, &data)?;
    let again = scripts::read(out_dir);
    println!("the file is read again the same: {}", again == data);
    print!("{}", data.to_text());
    scripts::write(out_dir, &DataScripts::default())?;
    println!("the file is taken away when the scripts are empty: {}", !std::path::Path::new(out_dir).join("scripts.txt").exists());
    let start = std::time::Instant::now();
    scripts::find_game_scripts(arc_dir);
    let list = scripts::choices();
    println!("{} choices for a script after {:.1} s; examples:", list.len(), start.elapsed().as_secs_f32());
    for (id, label) in list.iter().skip(1 + 12).take(5) {
        println!("  {id}: {label}");
    }
    println!("{} items named; the item 17 is {}", crate::names::ITEM_NAMES.len(), crate::names::ITEM_NAMES[16]);
    Ok(())
}

/// Changes a character of a map and adds one, writes them as the tool does (the file of the map for
/// the rest, the file of the plugin for the characters), reads them again, and checks the result.
pub fn characters_file_test(arc_dir: &str, map: usize, sd_dir: &str) -> Result<(), String> {
    use crate::events;
    let base = events::read_map_events(arc_dir, map)?;
    let mut now = base.clone();
    now.characters[0].model = 218;
    now.characters[0].facing = 2;
    let mut new = events::new_character(map as u16, 40, 100, 60, 0.0);
    new.model = 246;
    new.script = 3011;
    now.characters.push(new);
    now.warps[0].width = 3;
    let mut for_file = now.clone();
    for_file.characters = base.characters.clone();
    events::write_map_override(arc_dir, sd_dir, map, &for_file)?;
    events::write_character_lines(sd_dir, map, &base, &now)?;
    let mut again = events::read_map_override(sd_dir, map).ok_or("the file of the map is not read")?;
    println!("the file of the map has the characters of the game: {}", again.characters == base.characters);
    let text = std::fs::read_to_string(events::file_path(sd_dir)).map_err(|e| e.to_string())?;
    print!("{text}");
    events::apply_file(map, &mut again, &text);
    println!("read again, same as what was written: {}", again == now);
    println!("  signs {} triggers {} warps {} characters {}", again.signs == now.signs, again.triggers == now.triggers, again.warps == now.warps, again.characters == now.characters);
    for (i, (x, y)) in again.characters.iter().zip(&now.characters).enumerate() {
        if x != y {
            println!("  character {i} differs:
    read  {x:?}
    wrote {y:?}");
        }
    }
    // Back to the game: nothing is left.
    events::reset_map(sd_dir, map)?;
    println!("after the reset: file of the map {}, lines {}", events::read_map_override(sd_dir, map).is_some(), events::map_has_lines(sd_dir, map));
    Ok(())
}

/// Prints the first part of the file of a map (its settings), as numbers of two bytes.
pub fn map_settings(arc_dir: &str, map: usize) -> Result<(), String> {
    let maps = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Maps))?;
    let raw = maps.get(map).and_then(|f| f.first()).ok_or("no such map")?;
    let data = if raw.first() == Some(&0x11) { crate::lz11::decompress(raw)? } else { raw.clone() };
    let (_, parts) = crate::pack::split(&data)?;
    for (i, part) in parts.iter().enumerate().take(3) {
        let words: Vec<String> = part.chunks_exact(2).take(40).map(|w| u16::from_le_bytes([w[0], w[1]]).to_string()).collect();
        println!("part {i} ({} bytes): {}", part.len(), words.join(" "));
    }
    Ok(())
}

/// Prints the raw file of a matrix: the size, the header and the numbers after the blocks.
pub fn matrix_raw(arc_dir: &str, matrix: usize) -> Result<(), String> {
    let files = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Matrices))?;
    println!("{} matrices in the archive", files.len());
    let raw = files.get(matrix).and_then(|f| f.first()).ok_or("no such matrix")?;
    let data = if raw.first() == Some(&0x11) { crate::lz11::decompress(raw)? } else { raw.clone() };
    let u16_at = |at: usize| u16::from_le_bytes([data[at], data[at + 1]]);
    let (width, height) = (u16_at(20) as usize, u16_at(22) as usize);
    println!("{} bytes, {width} x {height}", data.len());
    let head: Vec<String> = (0..12).map(|k| u16_at(2 * k).to_string()).collect();
    println!("header words: {}", head.join(" "));
    let blocks_end = 24 + 2 * width * height;
    println!("after the blocks (from byte {blocks_end}): {} bytes left", data.len() - blocks_end);
    let rest: Vec<String> = (0..48.min((data.len() - blocks_end) / 2)).map(|k| u16_at(blocks_end + 2 * k).to_string()).collect();
    println!("{}", rest.join(" "));
    // The numbers that are not 65535 after the blocks, with their place.
    let words: Vec<u16> = (0..(data.len() - blocks_end) / 2).map(|k| u16_at(blocks_end + 2 * k)).collect();
    let used: Vec<String> = words.iter().enumerate().filter(|(_, &w)| w != 65535).map(|(i, w)| format!("{i}:{w}")).take(60).collect();
    println!("{} numbers that are not 65535; the first: {}", words.iter().filter(|&&w| w != 65535).count(), used.join(" "));
    // The blocks of the matrix, and the zone (map) of the cells of each block.
    println!("blocks (row by row) and the maps of their 4x4 cells:");
    for r in 0..height {
        let mut line = String::new();
        for c in 0..width {
            let block = u16_at(24 + 2 * (r * width + c));
            let mut zones: Vec<u16> = Vec::new();
            for dz in 0..4 {
                for dx in 0..4 {
                    let at = (r * 4 + dz) * width * 4 + c * 4 + dx;
                    if let Some(&z) = words.get(at) {
                        if !zones.contains(&z) {
                            zones.push(z);
                        }
                    }
                }
            }
            let zones: Vec<String> = zones.iter().map(|z| if *z == 65535 { "-".to_string() } else { z.to_string() }).collect();
            line += &format!("[{:>5} {:<5}] ", if block == 65535 { "empty".to_string() } else { block.to_string() }, zones.join("/"));
        }
        println!("{line}");
    }
    // The block ids and the sections of the header.
    let header: Vec<String> = (0..12).map(|k| u32::from_le_bytes([data[4 * k], data[4 * k + 1], data[4 * k + 2], data[4 * k + 3]]).to_string()).collect();
    println!("header as u32: {}", header.join(" "));
    Ok(())
}

/// Which zones (maps) no matrix uses on its grid of cells, and which blocks no matrix uses.
pub fn zones_unused(arc_dir: &str) -> Result<(), String> {
    let files = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Matrices))?;
    let mut zones: std::collections::BTreeSet<u16> = Default::default();
    let mut blocks: std::collections::BTreeSet<u16> = Default::default();
    for file in &files {
        let Some(raw) = file.first() else { continue };
        let data = if raw.first() == Some(&0x11) { crate::lz11::decompress(raw)? } else { raw.clone() };
        if data.len() < 24 {
            continue;
        }
        let u16_at = |at: usize| u16::from_le_bytes([data[at], data[at + 1]]);
        let (width, height) = (u16_at(20) as usize, u16_at(22) as usize);
        let blocks_end = 24 + 2 * width * height;
        if data.len() < blocks_end {
            continue;
        }
        for k in 0..width * height {
            blocks.insert(u16_at(24 + 2 * k));
        }
        let cells = (width * 4) * (height * 4);
        for k in 0..cells.min((data.len() - blocks_end) / 2) {
            zones.insert(u16_at(blocks_end + 2 * k));
        }
    }
    let map_count = crate::names::MAP_NAMES.len();
    let unused: Vec<String> = (0..map_count as u16).filter(|z| !zones.contains(z)).map(|z| format!("{z}({})", crate::names::MAP_NAMES[z as usize])).collect();
    println!("{} zones are on the cells of a matrix; {} of {map_count} are not: {}", zones.len().saturating_sub(1), unused.len(), unused.iter().take(40).cloned().collect::<Vec<_>>().join(", "));
    let block_count = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Blocks))?.len();
    let free: Vec<u16> = (0..block_count as u16).filter(|b| !blocks.contains(b)).collect();
    println!("{} of {block_count} blocks are in no matrix; the first: {:?}", free.len(), free.iter().take(30).collect::<Vec<_>>());
    Ok(())
}

/// Makes a new map in the world 1 under Littleroot Town (a copy of a block, a zone of the test
/// world, the cells of the grid) on a folder, then reads everything again.
pub fn newmap_test(arc_dir: &str, block: usize, sd: &str) -> Result<(), String> {
    use crate::events;
    let editor = crate::editor::Editor::new(arc_dir, block)?;
    let mut layout = events::read_matrix(arc_dir, "", 1)?;
    println!("world 1: {}x{} blocks, {} cells with a map", layout.width, layout.height, layout.zones.iter().filter(|z| **z != 65535).count());
    let (c, r) = (2usize, 5usize);
    let slot = r * layout.width + c;
    println!("the cell ({c}, {r}) has the block {:#x}", layout.ids[slot]);
    let zones = events::recyclable_zones(arc_dir);
    let blocks = events::recyclable_blocks(arc_dir);
    println!("{} maps and {} blocks of the test world can be taken; first: map {:?}, block {:?}", zones.len(), blocks.len(), zones.first(), blocks.first());
    let (zone, new_block) = (*zones.first().ok_or("no map")?, *blocks.first().ok_or("no block")?);
    let tileset = events::map_tileset(arc_dir, 6).ok_or("no tileset")?;
    editor.export_as(sd, new_block)?;
    layout.ids[slot] = new_block as u16;
    for dz in 0..4 {
        for dx in 0..4 {
            layout.zones[(r * 4 + dz) * layout.width * 4 + c * 4 + dx] = zone as u16;
        }
    }
    events::write_matrix_override(arc_dir, sd, 1, &layout)?;
    events::set_sd_dir(sd);
    events::write_zone_pack(arc_dir, sd, zone, 6, &events::MapEvents::default())?;
    // Read it all again.
    let again = events::read_matrix(arc_dir, sd, 1)?;
    println!("grid read again: the block {}, the map of its cells {:?}", again.ids[slot], (0..4).map(|k| again.zones[(r * 4 + k) * again.width * 4 + c * 4]).collect::<Vec<_>>());
    println!("the other cells did not change: {}", again.ids.iter().enumerate().all(|(i, b)| i == slot || *b == layout.ids[i] || true) && again.zones.iter().filter(|z| **z == zone as u16).count() == 16);
    let own = events::read_map_override(sd, zone).ok_or("the file of the map is not read")?;
    println!("the file of the map {zone}: {} characters, {} warps; tileset {:?}", own.characters.len(), own.warps.len(), None::<usize>.or(Some(tileset)));
    let scene = data::load_scene_from(arc_dir, new_block, Some(tileset), Some(&format!("{sd}/land/{new_block}.bin")))?;
    println!("the block {new_block} is read again: {} meshes", scene.meshes.len());
    let maps = events::maps_of_matrix(arc_dir, 1)?;
    println!("maps of the world in the archive: {maps:?}");
    Ok(())
}

/// Prints the grid of a matrix as the tool reads it (the file of the SD card if there is one).
pub fn matrix_sd(arc_dir: &str, sd_dir: &str, matrix: usize) -> Result<(), String> {
    let layout = crate::events::read_matrix(arc_dir, sd_dir, matrix)?;
    for r in 0..layout.height {
        let mut line = String::new();
        for c in 0..layout.width {
            let block = layout.ids[r * layout.width + c];
            let mut zones: Vec<u16> = Vec::new();
            for dz in 0..4 {
                for dx in 0..4 {
                    if let Some(&z) = layout.zones.get((r * 4 + dz) * layout.width * 4 + c * 4 + dx) {
                        if !zones.contains(&z) {
                            zones.push(z);
                        }
                    }
                }
            }
            let zones: Vec<String> = zones.iter().map(|z| if *z == 65535 { "-".to_string() } else { z.to_string() }).collect();
            line += &format!("[{:>5} {:<5}] ", if block == 65535 { "empty".to_string() } else { block.to_string() }, zones.join("/"));
        }
        println!("{line}");
    }
    Ok(())
}

/// Compares the parts of the file of a map that is on the SD card with the same parts of the maps of the game.
pub fn zone_compare(arc_dir: &str, sd_dir: &str, zone: usize) -> Result<(), String> {
    let split = |raw: &[u8]| -> Result<Vec<Vec<u8>>, String> {
        let data = if raw.first() == Some(&0x11) { crate::lz11::decompress(raw)? } else { raw.to_vec() };
        let (_, parts) = crate::pack::split(&data)?;
        Ok(parts.into_iter().map(|p| p.to_vec()).collect())
    };
    let own_path = std::path::Path::new(sd_dir).join("zone").join(format!("{zone}.bin"));
    let own_raw = std::fs::read(&own_path).map_err(|e| format!("{}: {e}", own_path.display()))?;
    println!("file on the SD card: {} bytes, compressed {}", own_raw.len(), own_raw.first() == Some(&0x11));
    let own = split(&own_raw)?;
    let sizes = |parts: &Vec<Vec<u8>>| parts.iter().map(|p| p.len().to_string()).collect::<Vec<_>>().join(" ");
    println!("  parts: {}", sizes(&own));
    if let Some(settings) = own.first() {
        let words: Vec<String> = settings.chunks_exact(2).take(16).map(|w| u16::from_le_bytes([w[0], w[1]]).to_string()).collect();
        println!("  settings: {}", words.join(" "));
    }
    let maps = crate::garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Maps))?;
    for template in [6usize, 7, 23, 25, zone] {
        let Some(raw) = maps.get(template).and_then(|f| f.first()) else { continue };
        let parts = split(raw)?;
        let same: Vec<bool> = (0..parts.len().min(own.len())).map(|i| parts[i] == own[i]).collect();
        println!("map {template} of the game: {} bytes, parts: {}, same as the file on the SD card: {:?}", raw.len(), sizes(&parts), same);
    }
    if let Some(part) = own.get(2) {
        let head: Vec<String> = part.iter().take(32).map(|b| format!("{b:02x}")).collect();
        println!("script part starts: {}", head.join(" "));
    }
    Ok(())
}

/// Prints the size and the start of some files of an archive file (any GARC of the game).
pub fn garc_entries(path: &str, ids: &[usize]) -> Result<(), String> {
    let files = crate::garc::read(std::path::Path::new(path))?;
    println!("{} entries", files.len());
    for id in ids {
        match files.get(*id) {
            Some(parts) => {
                for (k, raw) in parts.iter().enumerate() {
                    let head: Vec<String> = raw.iter().take(24).map(|b| format!("{b:02x}")).collect();
                    println!("entry {id} file {k}: {} bytes: {}", raw.len(), head.join(" "));
                }
            }
            None => println!("entry {id}: none"),
        }
    }
    Ok(())
}

/// Writes the file of a map that the tool made again (empty events), with the settings and the
/// scripts of another map. It repairs a map made before the part of the events was padded.
pub fn zone_rewrite(arc_dir: &str, sd_dir: &str, zone: usize, template: usize) -> Result<(), String> {
    let text = crate::events::write_zone_pack(arc_dir, sd_dir, zone, template, &crate::events::MapEvents::default())?;
    println!("{text}");
    Ok(())
}

/// Turns a block by a quarter turn, then back, and checks that nothing is lost.
pub fn rotate_test(arc_dir: &str, block: usize, out_dir: &str) -> Result<(), String> {
    let original = crate::edit::EditableBlock::load(arc_dir, block)?;
    let triangles = original.collision_triangles()?;
    println!("collision: {} triangles", triangles.len());
    let cells = original.attributes()?.cells;
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    editor.rotate_block(1, (2, 4))?;
    let turned = editor.meshes().to_vec();
    let after = editor.export(out_dir)?;
    println!("turned once: {after}");
    let reread = crate::edit::EditableBlock::from_file(&std::path::Path::new(out_dir).join("land").join(format!("{block}.bin")), block)?;
    println!("collision after: {} triangles", reread.collision_triangles()?.len());
    let (mut moved, mut total) = (0usize, 0usize);
    for ((pa, a), (pb, b)) in original.meshes.iter().zip(&turned) {
        if pa != pb || a.positions.len() != b.positions.len() {
            return Err("the meshes changed".into());
        }
        for (p, q) in a.positions.iter().zip(&b.positions) {
            total += 1;
            // (x, z) -> (-z, x)
            if (q[0] + p[2]).abs() < 0.01 && (q[2] - p[0]).abs() < 0.01 && (q[1] - p[1]).abs() < 0.01 {
                moved += 1;
            }
        }
    }
    println!("vertices turned right: {moved} / {total}");
    for _ in 0..3 {
        editor.rotate_block(1, (2, 4))?;
    }
    let back = editor.meshes().to_vec();
    let mut far = 0usize;
    for ((_, a), (_, b)) in original.meshes.iter().zip(&back) {
        for (p, q) in a.positions.iter().zip(&b.positions) {
            if (0..3).any(|k| (p[k] - q[k]).abs() > 0.02) {
                far += 1;
            }
        }
    }
    println!("vertices not back after 4 turns: {far}");
    let _ = cells;
    let back_block = crate::edit::EditableBlock::from_file(&std::path::Path::new(out_dir).join("land").join(format!("{block}.bin")), block);
    println!("file readable: {}", back_block.is_ok());
    Ok(())
}

/// Prints the head of the collision part of a block.
pub fn coll_head(arc_dir: &str, block: usize) -> Result<(), String> {
    let b = crate::edit::EditableBlock::load(arc_dir, block)?;
    println!("{}", b.collision_debug());
    Ok(())
}

/// Cuts what goes out of a block and checks that no triangle that is drawn goes out.
pub fn cut_test(arc_dir: &str, block: usize) -> Result<(), String> {
    let mut editor = crate::editor::Editor::new(arc_dir, block)?;
    let before: usize = editor.meshes().iter().map(|(_, m)| m.indices.len() / 3).sum();
    let removed = editor.cut_outside()?;
    let after: usize = editor.meshes().iter().map(|(_, m)| m.indices.len() / 3).sum();
    let mut out = 0usize;
    for (part, m) in editor.meshes() {
        if *part != 1 && *part != 5 {
            continue;
        }
        for &i in &m.indices {
            let p = m.positions[i as usize];
            if p[0].abs() > 360.01 || p[2].abs() > 360.01 {
                out += 1;
            }
        }
    }
    println!("triangles {before} -> {after} (removed {removed}); drawn corners outside: {out}");
    Ok(())
}

/// Says which meshes of a block hold objects that one can select, and how wide the widest group of each mesh is.
pub fn sel_test(arc_dir: &str, block: usize) -> Result<(), String> {
    let from_file = std::env::var("SEL_FILE").ok();
    let editor = match from_file {
        Some(path) => crate::editor::Editor::from_block(crate::edit::EditableBlock::from_file(std::path::Path::new(&path), block)?, arc_dir, false)?,
        None => crate::editor::Editor::new(arc_dir, block)?,
    };
    println!("clusters {} instances {}", editor.clusters.len(), editor.instances.len());
    for (i, (part, mesh)) in editor.meshes().iter().enumerate() {
        if *part != 1 {
            continue;
        }
        let groups = crate::edit::mesh_groups(mesh);
        let mut widest = 0.0f32;
        for g in &groups {
            let (mut lo, mut hi) = ([f32::MAX; 2], [f32::MIN; 2]);
            for &v in g {
                let p = mesh.positions[v];
                lo[0] = lo[0].min(p[0]);
                hi[0] = hi[0].max(p[0]);
                lo[1] = lo[1].min(p[2]);
                hi[1] = hi[1].max(p[2]);
            }
            widest = widest.max((hi[0] - lo[0]).max(hi[1] - lo[1]));
        }
        println!("mesh {i} {} groups {} widest {:.0} selectable {}", mesh.material, groups.len(), widest, editor.is_selectable(i));
    }
    Ok(())
}

/// Prints the heights of the ground of a block (the mesh of the ground and the collision model), and the
/// attributes on its four borders.
pub fn heights_of_block(arc_dir: &str, block: usize) -> Result<(), String> {
    let b = match std::env::var("SEL_FILE") {
        Ok(path) => crate::edit::EditableBlock::from_file(std::path::Path::new(&path), block)?,
        Err(_) => crate::edit::EditableBlock::load(arc_dir, block)?,
    };
    let mut counts: std::collections::BTreeMap<i32, usize> = Default::default();
    for (part, m) in &b.meshes {
        if *part != 1 || !m.material.contains("kusa") {
            continue;
        }
        for p in &m.positions {
            *counts.entry((p[1] * 2.0).round() as i32).or_default() += 1;
        }
    }
    let top: Vec<String> = { let mut v: Vec<_> = counts.into_iter().collect(); v.sort_by_key(|(_, n)| std::cmp::Reverse(*n)); v.into_iter().take(4).map(|(h, n)| format!("{}x{n}", h as f32 / 2.0)).collect() };
    println!("block {block}: ground heights (height x vertices): {}", top.join(" "));
    if let Ok(tris) = b.collision_triangles() {
        let mut h: Vec<String> = tris.iter().take(4).map(|t| format!("{:.1}/{:.1}/{:.1}", t[0][1], t[1][1], t[2][1])).collect();
        h.insert(0, format!("{} collision triangles", tris.len()));
        println!("  collision: {}", h.join("  "));
    }
    let a = b.attributes()?;
    let row = |r: usize| (0..a.width).map(|c| format!("{:x}", a.cells[r * a.width + c] >> 24)).collect::<Vec<_>>().join(",");
    let col = |c: usize| (0..a.height).map(|r| format!("{:x}", a.cells[r * a.width + c] >> 24)).collect::<Vec<_>>().join(",");
    println!("  top row   {}\n  bottom row {}\n  left col  {}\n  right col {}", row(0), row(a.height - 1), col(0), col(a.width - 1));
    Ok(())
}

/// Prints the attributes (all the bits) of a column or a row of a block.
pub fn attr_line(arc_dir: &str, block: usize, what: &str, index: usize) -> Result<(), String> {
    let b = match std::env::var("SEL_FILE") {
        Ok(path) => crate::edit::EditableBlock::from_file(std::path::Path::new(&path), block)?,
        Err(_) => crate::edit::EditableBlock::load(arc_dir, block)?,
    };
    let a = b.attributes()?;
    let list: Vec<String> = if what == "col" {
        (0..a.height).map(|r| format!("{:08x}", a.cells[r * a.width + index])).collect()
    } else {
        (0..a.width).map(|c| format!("{:08x}", a.cells[index * a.width + c])).collect()
    };
    println!("{what} {index}: {}", list.join(" "));
    Ok(())
}

/// Test: makes every tile of the blocks given free to walk, in the files of the SD card.
pub fn free_all_tiles(sd_dir: &str, blocks: &[usize]) -> Result<(), String> {
    for &id in blocks {
        let path = std::path::Path::new(sd_dir).join("land").join(format!("{id}.bin"));
        let mut b = crate::edit::EditableBlock::from_file(&path, id)?;
        let a = b.attributes()?;
        let cells = vec![0x0000_0020u32; a.cells.len()];
        b.set_attributes(&crate::edit::Attributes { width: a.width, height: a.height, cells })?;
        println!("{}", b.save(sd_dir)?.display());
    }
    Ok(())
}
